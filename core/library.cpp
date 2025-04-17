/*
The MIT License (MIT)

Copyright (c) 2025 g2wfw

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.*/

#include "library.h"
#include "linker.h"
#include "core/logging/check.h"
#include <xdl.h>
#include <unistd.h>

typedef struct xdl {
    char *pathname;
    uintptr_t load_bias;
    const ElfW(Phdr) *dlpi_phdr;
    ElfW(Half)
    dlpi_phnum;

    struct xdl *next;     // to next xdl obj for cache in xdl_addr()
    void *linker_handle;  // hold handle returned by xdl_linker_force_dlopen()

    //
    // (1) for searching symbols from .dynsym
    //

    bool dynsym_try_load;
    ElfW(Sym) *dynsym;   // .dynsym
    const char *dynstr;  // .dynstr

    // .hash (SYSV hash for .dynstr)
    struct {
        const uint32_t *buckets;
        uint32_t buckets_cnt;
        const uint32_t *chains;
        uint32_t chains_cnt;
    } sysv_hash;

    // .gnu.hash (GNU hash for .dynstr)
    struct {
        const uint32_t *buckets;
        uint32_t buckets_cnt;
        const uint32_t *chains;
        uint32_t symoffset;
        const ElfW(Addr) *bloom;
        uint32_t bloom_cnt;
        uint32_t bloom_shift;
    } gnu_hash;

    //
    // (2) for searching symbols from .symtab
    //

    bool symtab_try_load;
    uintptr_t base;

    ElfW(Sym) *symtab;  // .symtab
    size_t symtab_cnt;
    char *strtab;  // .strtab
    size_t strtab_sz;
} xdl_t;


typedef void (*linker_dtor_function_t)();

typedef void (*linker_ctor_function_t)(int, char **, char **);

typedef void (*linker_function_t)();

#define SOINFO_NAME_LEN 128
struct soinfo_10 {
#if defined(__work_around_b_24465209__)
    char old_name_[SOINFO_NAME_LEN];
#endif
public:
    ElfW(Phdr) *phdr;
    size_t phnum;
#if defined(__work_around_b_24465209__)
    ElfW(Addr) unused0; // DO NOT USE, maintained for compatibility.
#endif
    ElfW(Addr) base;
    size_t size;

#if defined(__work_around_b_24465209__)
    uint32_t unused1;  // DO NOT USE, maintained for compatibility.
#endif

    ElfW(Dyn) *dynamic;

#if defined(__work_around_b_24465209__)
    uint32_t unused2; // DO NOT USE, maintained for compatibility
        uint32_t unused3; // DO NOT USE, maintained for compatibility
#endif

    soinfo_10 *next;
    uint32_t flags_;

    char *strtab_;
    ElfW(Sym) *symtab_;

    size_t nbucket_;
    size_t nchain_;
    uint32_t *bucket_;
    uint32_t *chain_;

#if defined(__mips__) || !defined(__LP64__)
    // This is only used by mips and mips64, but needs to be here for
    // all 32-bit architectures to preserve binary compatibility.
    ElfW(Addr) **plt_got_;
#endif

#if defined(USE_RELA)
    ElfW(Rela)* plt_rela_;
  size_t plt_rela_count_;

  ElfW(Rela)* rela_;
  size_t rela_count_;
#else
    ElfW(Rel) *plt_rel_;
    size_t plt_rel_count_;

    ElfW(Rel) *rel_;
    size_t rel_count_;
#endif

    linker_ctor_function_t *preinit_array_;
    size_t preinit_array_count_;

    linker_ctor_function_t *init_array_;
    size_t init_array_count_;
    linker_dtor_function_t *fini_array_;
    size_t fini_array_count_;

    linker_ctor_function_t init_func_;
    linker_dtor_function_t fini_func_;

#if defined(__arm__)
    public:
        // ARM EABI section used for stack unwinding.
        uint32_t *ARM_exidx;
        size_t ARM_exidx_count;
    private:
#elif defined(__mips__)
    uint32_t mips_symtabno_;
  uint32_t mips_local_gotno_;
  uint32_t mips_gotsym_;
  bool mips_relocate_got(const VersionTracker& version_tracker,
                         const soinfo_list_t& global_group,
                         const soinfo_list_t& local_group);
#if !defined(__LP64__)
  bool mips_check_and_adjust_fp_modes();
#endif
#endif
    size_t ref_count_;
public:
    link_map link_map_head;

    bool constructors_called;

    // When you read a virtual address from the ELF file, add this
    // value to get the corresponding address in the process' address space.
    ElfW(Addr)
    load_bias;

#if !defined(__LP64__)
    bool has_text_relocations;
#endif
    bool has_DT_SYMBOLIC;
};


struct soinfo_11 {
#if defined(__work_around_b_24465209__)
    char old_name_[SOINFO_NAME_LEN];
#endif
public:
    ElfW(Phdr) *phdr;
    size_t phnum;
#if defined(__work_around_b_24465209__)
    ElfW(Addr) unused0; // DO NOT USE, maintained for compatibility.
#endif
    ElfW(Addr)
    base;
    size_t size;

#if defined(__work_around_b_24465209__)
    uint32_t unused1;  // DO NOT USE, maintained for compatibility.
#endif

    ElfW(Dyn) *dynamic;

#if defined(__work_around_b_24465209__)
    uint32_t unused2; // DO NOT USE, maintained for compatibility
        uint32_t unused3; // DO NOT USE, maintained for compatibility
#endif

    soinfo_10 *next;
    uint32_t flags_;

    char *strtab_;
    ElfW(Sym) *symtab_;

    size_t nbucket_;
    size_t nchain_;
    uint32_t *bucket_;
    uint32_t *chain_;

#if defined(__mips__) || !defined(__LP64__)
    // This is only used by mips and mips64, but needs to be here for
    // all 32-bit architectures to preserve binary compatibility.
    ElfW(Addr) **plt_got_;
#endif

#if defined(USE_RELA)
    ElfW(Rela)* plt_rela_;
  size_t plt_rela_count_;

  ElfW(Rela)* rela_;
  size_t rela_count_;
#else
    ElfW(Rel) *plt_rel_;
    size_t plt_rel_count_;

    ElfW(Rel) *rel_;
    size_t rel_count_;
#endif

    linker_ctor_function_t *preinit_array_;
    size_t preinit_array_count_;

    linker_ctor_function_t *init_array_;
    size_t init_array_count_;
    linker_dtor_function_t *fini_array_;
    size_t fini_array_count_;

    linker_ctor_function_t init_func_;
    linker_dtor_function_t fini_func_;

#if defined(__arm__)
    public:
        // ARM EABI section used for stack unwinding.
        uint32_t *ARM_exidx;
        size_t ARM_exidx_count;
    private:
#elif defined(__mips__)
    uint32_t mips_symtabno_;
  uint32_t mips_local_gotno_;
  uint32_t mips_gotsym_;
  bool mips_relocate_got(const VersionTracker& version_tracker,
                         const soinfo_list_t& global_group,
                         const soinfo_list_t& local_group);
#if !defined(__LP64__)
  bool mips_check_and_adjust_fp_modes();
#endif
#endif
    size_t ref_count_;
public:
    link_map link_map_head;

    bool constructors_called;

    // When you read a virtual address from the ELF file, add this
    // value to get the corresponding address in the process' address space.
    ElfW(Addr)
    load_bias;

#if !defined(__LP64__)
    bool has_text_relocations;
#endif
    bool has_DT_SYMBOLIC;
};
struct soinfo_13 {
#if defined(__work_around_b_24465209__)
    char old_name_[SOINFO_NAME_LEN];
#endif
public:
    ElfW(Phdr) *phdr;
    size_t phnum;
#if defined(__work_around_b_24465209__)
    ElfW(Addr) unused0; // DO NOT USE, maintained for compatibility.
#endif
    ElfW(Addr)
    base;
    size_t size;

#if defined(__work_around_b_24465209__)
    uint32_t unused1;  // DO NOT USE, maintained for compatibility.
#endif

    ElfW(Dyn) *dynamic;

#if defined(__work_around_b_24465209__)
    uint32_t unused2; // DO NOT USE, maintained for compatibility
        uint32_t unused3; // DO NOT USE, maintained for compatibility
#endif

    soinfo_13 *next;
    uint32_t flags_;

    char *strtab_;
    ElfW(Sym) *symtab_;

    size_t nbucket_;
    size_t nchain_;
    uint32_t *bucket_;
    uint32_t *chain_;

#if !defined(__LP64__)
    ElfW(Addr) **unused4; // DO NOT USE, maintained for compatibility
#endif

#if defined(USE_RELA)
    ElfW(Rela)* plt_rela_;
  size_t plt_rela_count_;

  ElfW(Rela)* rela_;
  size_t rela_count_;
#else
    ElfW(Rel) *plt_rel_;
    size_t plt_rel_count_;

    ElfW(Rel) *rel_;
    size_t rel_count_;
#endif

    linker_ctor_function_t *preinit_array_;
    size_t preinit_array_count_;

    linker_ctor_function_t *init_array_;
    size_t init_array_count_;
    linker_dtor_function_t *fini_array_;
    size_t fini_array_count_;

    linker_ctor_function_t init_func_;
    linker_dtor_function_t fini_func_;

#if defined(__arm__)
    public:
  // ARM EABI section used for stack unwinding.
  uint32_t* ARM_exidx;
  size_t ARM_exidx_count;
 private:
#endif
    size_t ref_count_;
public:
    link_map link_map_head;

    bool constructors_called;

    // When you read a virtual address from the ELF file, add this
    // value to get the corresponding address in the process' address space.
    ElfW(Addr)
    load_bias;

#if !defined(__LP64__)
    bool has_text_relocations;
#endif
    bool has_DT_SYMBOLIC;
};
struct soinfo_15 {
#if defined(__work_around_b_24465209__)
    private:
  char old_name_[SOINFO_NAME_LEN];
#endif
public:
    const ElfW(Phdr) *phdr;
    size_t phnum;
#if defined(__work_around_b_24465209__)
    ElfW(Addr) unused0; // DO NOT USE, maintained for compatibility.
#endif
    ElfW(Addr)
    base;
    size_t size;

#if defined(__work_around_b_24465209__)
    uint32_t unused1;  // DO NOT USE, maintained for compatibility.
#endif

    ElfW(Dyn) *dynamic;

#if defined(__work_around_b_24465209__)
    uint32_t unused2; // DO NOT USE, maintained for compatibility
  uint32_t unused3; // DO NOT USE, maintained for compatibility
#endif

    soinfo_15 *next;

    uint32_t flags_;

    const char *strtab_;
    ElfW(Sym) *symtab_;

    size_t nbucket_;
    size_t nchain_;
    uint32_t *bucket_;
    uint32_t *chain_;

#if !defined(__LP64__)
    ElfW(Addr) **unused4; // DO NOT USE, maintained for compatibility
#endif

#if defined(USE_RELA)
    ElfW(Rela)* plt_rela_;
  size_t plt_rela_count_;

  ElfW(Rela)* rela_;
  size_t rela_count_;
#else
    ElfW(Rel) *plt_rel_;
    size_t plt_rel_count_;

    ElfW(Rel) *rel_;
    size_t rel_count_;
#endif

    linker_ctor_function_t *preinit_array_;
    size_t preinit_array_count_;

    linker_ctor_function_t *init_array_;
    size_t init_array_count_;
    linker_dtor_function_t *fini_array_;
    size_t fini_array_count_;

    linker_ctor_function_t init_func_;
    linker_dtor_function_t fini_func_;

#if defined(__arm__)
    public:
  // ARM EABI section used for stack unwinding.
  uint32_t* ARM_exidx;
  size_t ARM_exidx_count;
 private:
#endif
    size_t ref_count_;
public:
    link_map link_map_head;

    bool constructors_called;

    // When you read a virtual address from the ELF file, add this
    // value to get the corresponding address in the process' address space.
    ElfW(Addr)
    load_bias;

#if !defined(__LP64__)
    bool has_text_relocations;
#endif
    bool has_DT_SYMBOLIC;
};

struct soinfo_14 {
#if defined(__work_around_b_24465209__)

    char old_name_[SOINFO_NAME_LEN];
#endif
public:
    const ElfW(Phdr) *phdr;
    size_t phnum;
#if defined(__work_around_b_24465209__)
    ElfW(Addr) unused0; // DO NOT USE, maintained for compatibility.
#endif
    ElfW(Addr)
    base;
    size_t size;

#if defined(__work_around_b_24465209__)
    uint32_t unused1;  // DO NOT USE, maintained for compatibility.
#endif

    ElfW(Dyn) *dynamic;

#if defined(__work_around_b_24465209__)
    uint32_t unused2; // DO NOT USE, maintained for compatibility
  uint32_t unused3; // DO NOT USE, maintained for compatibility
#endif

    soinfo_14 *next;

    uint32_t flags_;

    const char *strtab_;
    ElfW(Sym) *symtab_;

    size_t nbucket_;
    size_t nchain_;
    uint32_t *bucket_;
    uint32_t *chain_;

#if !defined(__LP64__)
    ElfW(Addr) **unused4; // DO NOT USE, maintained for compatibility
#endif

#if defined(USE_RELA)
    ElfW(Rela)* plt_rela_;
  size_t plt_rela_count_;

  ElfW(Rela)* rela_;
  size_t rela_count_;
#else
    ElfW(Rel) *plt_rel_;
    size_t plt_rel_count_;

    ElfW(Rel) *rel_;
    size_t rel_count_;
#endif

    linker_ctor_function_t *preinit_array_;
    size_t preinit_array_count_;

    linker_ctor_function_t *init_array_;
    size_t init_array_count_;
    linker_dtor_function_t *fini_array_;
    size_t fini_array_count_;

    linker_ctor_function_t init_func_;
    linker_dtor_function_t fini_func_;

#if defined(__arm__)
    public:
  // ARM EABI section used for stack unwinding.
  uint32_t* ARM_exidx;
  size_t ARM_exidx_count;
 private:
#endif
    size_t ref_count_;
public:
    link_map link_map_head;

    bool constructors_called;

    // When you read a virtual address from the ELF file, add this
    // value to get the corresponding address in the process' address space.
    ElfW(Addr)
    load_bias;

#if !defined(__LP64__)
    bool has_text_relocations;
#endif
    bool has_DT_SYMBOLIC;
};


namespace stl {
    Library::Library(const std::string &library_name) : library_name(library_name) {
        this->handler = xdl_open(library_name.c_str(), XDL_DEFAULT);
    }


    Library::~Library() {
        if (handler != nullptr) {
            xdl_close(handler);
        }
    }

    void *Library::find_symbol(const std::string symbol_name) {
        if (symbol_name.empty() || handler == nullptr) {
            return nullptr;
        }
        auto addr = xdl_sym(handler, symbol_name.c_str(), nullptr);
        if (addr == nullptr) {
            addr = xdl_dsym(handler, symbol_name.c_str(), nullptr);
        }
        return addr;
    }

    uintptr_t Library::get_load_bias() {
        if (handler != nullptr) {
            auto self = (xdl_t *) this->handler;
            return self->load_bias;
        }

        auto so_info_ = get_so_info();
        if (so_info == nullptr) {
            return 0;
        }
        auto sdk = android_get_device_api_level();
        if (sdk < 29) {
            LOGE("get_init_array_count not support sdk %d", sdk);
            return 0;
        }
        switch (sdk) {
            case 29: {
                auto soinfo10_ = static_cast<soinfo_10 *>(so_info_);
                return soinfo10_->load_bias;
            }
            case 31:
            case 30: {
                auto soinfo11_ = static_cast<soinfo_11 *>(so_info_);
                return soinfo11_->load_bias;
            }
            case 33: {
                auto soinfo13_ = static_cast<soinfo_13 *>(so_info_);
                return soinfo13_->load_bias;
            }
            case 34: {
                auto soinfo14_ = static_cast<soinfo_14 *>(so_info_);
                return soinfo14_->load_bias;
            }
            case 35: {
                auto soinfo15_ = static_cast<soinfo_15 *>(so_info_);
                return soinfo15_->load_bias;
            }
            default:
                LOGE("get_load_bias not support sdk %d", sdk);
                return 0;
        };

        return 0;
    }

    void *Library::get_so_info() {
        if (this->so_info == nullptr) {
            this->so_info = Linker::getInstance()->get_so_info(this->library_name.c_str());
        }
        return this->so_info;
    }

    void *Library::get_init_proc() {
        auto so_info_ = get_so_info();
        auto sdk = android_get_device_api_level();
        if (sdk < 29) {
            LOGE("get_init_proc not support sdk %d", sdk);
            return nullptr;
        }
        switch (sdk) {
            case 29: {
                auto soinfo10_ = static_cast<soinfo_10 *>(so_info_);
                return (void *) soinfo10_->init_func_;
            }
            case 31:
            case 30: {
                auto soinfo11_ = static_cast<soinfo_11 *>(so_info_);
                return (void *) soinfo11_->init_func_;
            }
            case 33: {
                auto soinfo13_ = static_cast<soinfo_13 *>(so_info_);
                return (void *) soinfo13_->init_func_;
            }
            case 34: {
                auto soinfo14_ = static_cast<soinfo_14 *>(so_info_);
                return (void *) soinfo14_->init_func_;
            }
            case 35: {
                auto soinfo15_ = static_cast<soinfo_15 *>(so_info_);
                return (void *) soinfo15_->init_func_;
            }
            default:
                LOGE("get_init_proc not support sdk %d", sdk);
                return nullptr;

        };
    }

    size_t Library::get_init_array_count() {
        auto so_info_ = get_so_info();
        auto sdk = android_get_device_api_level();
        if (sdk < 29) {
            LOGE("get_init_array_count not support sdk %d", sdk);
            return 0;
        }
        switch (sdk) {
            case 29: {
                auto soinfo10_ = static_cast<soinfo_10 *>(so_info_);
                return soinfo10_->init_array_count_;
            }
            case 31:
            case 30: {
                auto soinfo11_ = static_cast<soinfo_11 *>(so_info_);
                return soinfo11_->init_array_count_;
            }
            case 33: {
                auto soinfo13_ = static_cast<soinfo_13 *>(so_info_);
                return soinfo13_->init_array_count_;
            }
            case 34: {
                auto soinfo14_ = static_cast<soinfo_14 *>(so_info_);
                return soinfo14_->init_array_count_;
            }
            case 35: {
                auto soinfo15_ = static_cast<soinfo_15 *>(so_info_);
                return soinfo15_->init_array_count_;
            }
            default:
                LOGE("get_init_array_count not support sdk %d", sdk);
                return 0;
        };
    }

    void *Library::get_init_array() {
        auto so_info_ = get_so_info();
        auto sdk = android_get_device_api_level();
        if (sdk < 29) {
            LOGE("get_init_proc not support sdk %d", sdk);
            return nullptr;
        }
        switch (sdk) {
            case 29: {
                auto soinfo10_ = static_cast<soinfo_10 *>(so_info_);
                return (void *) soinfo10_->init_array_;
            }
            case 31:
            case 30: {
                auto soinfo11_ = static_cast<soinfo_11 *>(so_info_);
                return (void *) soinfo11_->init_array_;
            }
            case 33: {
                auto soinfo13_ = static_cast<soinfo_13 *>(so_info_);
                return (void *) soinfo13_->init_array_;
            }
            case 34: {
                auto soinfo14_ = static_cast<soinfo_14 *>(so_info_);
                return (void *) soinfo14_->init_array_;
            }
            case 35: {
                auto soinfo15_ = static_cast<soinfo_15 *>(so_info_);
                return (void *) soinfo15_->init_array_;
            }
            default:
                LOGE("get_init_array not support sdk %d", sdk);
                return nullptr;

        };
    }

    Library::Library(std::string library_name, void *linker_handler) : library_name(
            std::move(library_name)),
                                                                       so_info(linker_handler) {

    }

    void Library::set_init_array_count(size_t count) {
        auto so_info_ = get_so_info();
        auto sdk = android_get_device_api_level();
        if (sdk < 29) {
            LOGE("get_init_array_count not support sdk %d", sdk);
            return;
        }
        switch (sdk) {
            case 29: {
                auto soinfo10_ = static_cast<soinfo_10 *>(so_info_);
                soinfo10_->init_array_count_ = count;
            }
                break;
            case 31:
            case 30: {
                auto soinfo11_ = static_cast<soinfo_11 *>(so_info_);
                soinfo11_->init_array_count_ = count;
            }
                break;
            case 33: {
                auto soinfo13_ = static_cast<soinfo_13 *>(so_info_);
                soinfo13_->init_array_count_ = count;
            }
                break;
            case 34: {
                auto soinfo14_ = static_cast<soinfo_14 *>(so_info_);
                soinfo14_->init_array_count_ = count;
            }
                break;
            case 35: {
                auto soinfo15_ = static_cast<soinfo_15 *>(so_info_);
                soinfo15_->init_array_count_ = count;
            }
                break;
            default:
                LOGE("get_init_array_count not support sdk %d", sdk);
                return;
        };
    }

    void Library::set_init_proc(void *ptr) {
        auto so_info_ = get_so_info();
        auto sdk = android_get_device_api_level();
        if (sdk < 29) {
            LOGE("get_init_array_count not support sdk %d", sdk);
            return;
        }
        switch (sdk) {
            case 29: {
                auto soinfo10_ = static_cast<soinfo_10 *>(so_info_);
                soinfo10_->init_func_ = (linker_ctor_function_t) ptr;
            }
                break;
            case 31:
            case 30: {
                auto soinfo11_ = static_cast<soinfo_11 *>(so_info_);
                soinfo11_->init_func_ = (linker_ctor_function_t) ptr;
            }
                break;
            case 33: {
                auto soinfo13_ = static_cast<soinfo_13 *>(so_info_);
                soinfo13_->init_func_ = (linker_ctor_function_t) ptr;
            }
                break;
            case 34: {
                auto soinfo14_ = static_cast<soinfo_14 *>(so_info_);
                soinfo14_->init_func_ = (linker_ctor_function_t) ptr;
            }
                break;
            case 35: {
                auto soinfo15_ = static_cast<soinfo_15 *>(so_info_);
                soinfo15_->init_func_ = (linker_ctor_function_t) ptr;
            }
                break;
            default:
                LOGE("get_init_array_count not support sdk %d", sdk);
                return;
        };
    }

    bool Library::is_ctor_called() {
        auto so_info_ = get_so_info();
        auto sdk = android_get_device_api_level();
        if (sdk < 29) {
            LOGE("get_init_proc not support sdk %d", sdk);
            return false;
        }
        switch (sdk) {
            case 29: {
                auto soinfo10_ = static_cast<soinfo_10 *>(so_info_);
                return soinfo10_->constructors_called;
            }
            case 31:
            case 30: {
                auto soinfo11_ = static_cast<soinfo_11 *>(so_info_);
                return soinfo11_->constructors_called;
            }
            case 33: {
                auto soinfo13_ = static_cast<soinfo_13 *>(so_info_);
                return soinfo13_->constructors_called;
            }
            case 34: {
                auto soinfo14_ = static_cast<soinfo_14 *>(so_info_);
                return soinfo14_->constructors_called;
            }
            case 35: {
                auto soinfo15_ = static_cast<soinfo_15 *>(so_info_);
                return soinfo15_->constructors_called;
            }
            default:
                LOGE("get_init_array not support sdk %d", sdk);
                return false;

        };
    }

    std::unique_ptr <Library> Library::find_library(const std::string &library_name) {
        auto ptr = std::make_unique<Library>(library_name);
        if (ptr->handler == nullptr) {
            return nullptr;
        }
        return ptr;
    }

    size_t Library::get_library_size() {
        auto so_info_ = get_so_info();
        auto sdk = android_get_device_api_level();
        if (sdk < 29) {
            LOGE("get_library_size not support sdk %d", sdk);
            return false;
        }
        switch (sdk) {
            case 29: {
                xdl_t *xdlx = (xdl_t*)this->handler;
                auto soinfo10_ = static_cast<soinfo_10 *>(so_info_);
                return soinfo10_->size;
            }
            case 31:
            case 30: {
                auto soinfo11_ = static_cast<soinfo_11 *>(so_info_);
                return soinfo11_->size;
            }
            case 33: {
                auto soinfo13_ = static_cast<soinfo_13 *>(so_info_);
                return soinfo13_->size;
            }
            case 34: {
                auto soinfo14_ = static_cast<soinfo_14 *>(so_info_);
                return soinfo14_->size;
            }
            case 35: {
                auto soinfo15_ = static_cast<soinfo_15 *>(so_info_);
                return soinfo15_->size;
            }
            default:
                LOGE("get_library_size not support sdk %d", sdk);
                return 0;

        };

    }

    std::unique_ptr <Library> Library::find_library(uintptr_t address) {
        xdl_info_t xdlInfo;
        void *cache = NULL;
        memset(&xdlInfo, 0, sizeof(xdlInfo));
        xdl_addr((void *) address, &xdlInfo, &cache);
        if (xdlInfo.dli_fname != nullptr) {
            auto ptr = find_library(xdlInfo.dli_fname);
            xdl_addr_clean(&cache);
            return ptr;
        }
        xdl_addr_clean(&cache);
        return nullptr;
    }

    Range<uintptr_t> Library::get_library_range() {
        return {get_load_bias(), get_load_bias() + get_library_size()};
    }

    std::string Library::get_library_name() {
        return this->library_name;
    }

    std::string Library::get_symbol_by_address(uintptr_t address) {
        xdl_info_t xdlInfo;
        void *cache = NULL;
        memset(&xdlInfo, 0, sizeof(xdlInfo));
        xdl_addr((void *) address, &xdlInfo, &cache);
        if (xdlInfo.dli_sname != nullptr) {
            auto ptr = find_library(xdlInfo.dli_fname);
            xdl_addr_clean(&cache);
            return xdlInfo.dli_sname;
        }
        xdl_addr_clean(&cache);
        return "";
    }

    void Library::enumerate_exports(export_callback_t callback, void *user_data) {
        if (this->handler == nullptr || callback == nullptr) {
            return;
        }
        return xdl_enumerate_exports(this->handler, callback, user_data);
    }
} // rt