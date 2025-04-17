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
#include <android/api-level.h>
#include <xdl.h>
#include <vector>
#include <unordered_map>
#include <core/logging/check.h>
#include "linker.h"
#include "library.h"
#include "hook_manager.h"


namespace stl{

    static bool dl_hooked = false;
    static bool linker_hooked = false;
    std::vector<std::tuple<std::string, linker_type_t, linker_callback_t, void *>> monitor_list;


#if !defined(__LP64__)
#define __work_around_b_24465209__
#define __work_around_b_19059885__
#endif

    static char *(*_get_realpath)(void *) = nullptr;

    static void *(*solist_get_head)() = nullptr;

    static void *(*solist_get_somain)() = nullptr;

    static uintptr_t (*_linker_soinfo_to_handle)(void *) = nullptr;


    Linker *Linker::getInstance() {
        static Linker linker;
        return &linker;
    }

    Linker::~Linker() {

    }

    static const char *get_android_linker_path() {
#if __LP64__
        if (android_get_device_api_level() >= 29) {
            return (const char *) "/apex/com.android.runtime/bin/linker64";
        } else {
            return (const char *) "/system/bin/linker64";
        }
#else
        if (android_get_device_api_level() >= 29) {
            return (const char *) "/apex/com.android.runtime/bin/linker";
        } else {
            return (const char *) "/system/bin/linker";
        }
#endif
    }

    typedef void (*linker_dtor_function_t)();

    typedef void (*linker_ctor_function_t)(int, char **, char **);

    typedef void (*linker_function_t)();

    static int *g_argc = 0;
    static char ***g_argv = nullptr;
    static char ***g_envp = nullptr;


    static void call_functionT(const char *function_name __unused,
                               linker_ctor_function_t function,
                               const char *realpath __unused) {
        if (function == nullptr ||
            reinterpret_cast<uintptr_t>(function) == static_cast<uintptr_t>(-1)) {
            return;
        }
        function(g_argc == nullptr ? 0 : *g_argc, g_argv == nullptr ? nullptr : *g_argv,
                 g_envp == nullptr ? nullptr : *g_envp);
    }


    Linker::Linker() {
        this->handler = xdl_open(get_android_linker_path(), XDL_DEFAULT);

        if (_get_realpath == nullptr) {
            _get_realpath = (char *(*)(void *)) xdl_sym(this->handler,
                                                        "__dl__ZNK6soinfo12get_realpathEv",
                                                        nullptr);
            if (_get_realpath == nullptr) {
                _get_realpath = (char *(*)(void *)) xdl_dsym(this->handler,
                                                             "__dl__ZNK6soinfo12get_realpathEv",
                                                             nullptr);
            }
        }
        if (_get_realpath == nullptr) {
            LOGE("_get_realpath is null");
        }
        if (solist_get_head == nullptr) {
            solist_get_head = (void *(*)()) xdl_sym(this->handler, "__dl__Z15solist_get_headv",
                                                    nullptr);
            if (solist_get_head == nullptr) {
                solist_get_head = (void *(*)()) xdl_dsym(this->handler, "__dl__Z15solist_get_headv",
                                                         nullptr);
            }
        }
        if (solist_get_head == nullptr) {
            LOGE("solist_get_head is null");
        }
        if (solist_get_somain == nullptr) {
            solist_get_somain = (void *(*)()) xdl_sym(this->handler, "__dl__Z17solist_get_somainv",
                                                      nullptr);
            if (solist_get_somain == nullptr) {
                solist_get_somain = (void *(*)()) xdl_dsym(this->handler,
                                                           "__dl__Z17solist_get_somainv",
                                                           nullptr);
            }
        }
        if (solist_get_somain == nullptr) {
            LOGE("solist_get_somain is null");
        }
        if (_linker_soinfo_to_handle == nullptr) {
            _linker_soinfo_to_handle = (uintptr_t (*)(void *)) xdl_sym(this->handler,
                                                                       "__dl__ZN6soinfo9to_handleEv",
                                                                       nullptr);
            if (_linker_soinfo_to_handle == nullptr) {
                _linker_soinfo_to_handle = (uintptr_t (*)(void *)) xdl_dsym(this->handler,
                                                                            "__dl__ZN6soinfo9to_handleEv",
                                                                            nullptr);
            }
        }
        if (_linker_soinfo_to_handle == nullptr) {
            LOGE("_linker_soinfo_to_handle is null");
        }
        if (g_argc == nullptr) {
            g_argc = (int *) xdl_sym(this->handler, "__dl_g_argc", nullptr);
            if (g_argc == nullptr) {
                g_argc = (int *) xdl_dsym(this->handler, "__dl_g_argc", nullptr);
            }
        }
        if (g_argv == nullptr) {
            g_argv = (char ***) xdl_sym(this->handler, "__dl_g_argv", nullptr);
            if (g_argv == nullptr) {
                g_argv = (char ***) xdl_dsym(this->handler, "__dl_g_argv", nullptr);
            }
        }
        if (g_envp == nullptr) {
            g_envp = (char ***) xdl_sym(this->handler, "__dl_g_envp", nullptr);
            if (g_envp == nullptr) {
                g_envp = (char ***) xdl_dsym(this->handler, "__dl_g_envp", nullptr);
            }
        }


    }

    static void call_function(const char *function_name __unused,
                              linker_function_t function,
                              const char *realpath __unused) {
        if (function == nullptr ||
            reinterpret_cast<uintptr_t>(function) == static_cast<uintptr_t>(-1)) {
            return;
        }
        function();
    }

    template<typename F>
    static void call_array(const char *array_name __unused,
                           F *functions,
                           size_t count,
                           bool reverse,
                           const char *realpath) {
        if (functions == nullptr) {
            return;
        }

        int begin = reverse ? (count - 1) : 0;
        int end = reverse ? -1 : count;
        int step = reverse ? -1 : 1;

        for (int i = begin; i != end; i += step) {
            call_function("function", functions[i], realpath);
        }

    }

    HOOK_DEF_STATIC(void, call_constructors, void *so_info) {
        if (so_info == nullptr) {
            return hook_call_constructors_orig(so_info);
        }
        std::string library_path = _get_realpath(so_info);
        Library library(library_path, so_info);
        if (library.is_ctor_called()) {
            return hook_call_constructors_orig(so_info);
        }
        auto orig_count = library.get_init_array_count();
        void *orig_proc = library.get_init_proc();
        library.set_init_array_count(0);
        library.set_init_proc(nullptr);
        hook_call_constructors_orig(so_info);
        if (orig_proc != nullptr) {
            auto offset= (uintptr_t) orig_proc - library.get_load_bias();
            LOGD("DT_INIT:%p 0x%zx %s", orig_proc, offset,
                 library_path.c_str());
            for (const auto &[name, type, callback, data]: monitor_list) {
                if (type != INIT_PROC || name.empty()) {
                    continue;
                }
                if (strstr(library_path.c_str(), name.c_str()) == nullptr) {
                    continue;
                }
                if (callback != nullptr) {
                    callback(library_path.c_str(), INIT_PROC, so_info, data);
                }
            }
        }
        library.set_init_proc(orig_proc);
        if (orig_proc != nullptr) {
            linker_ctor_function_t init_func_ = (linker_ctor_function_t) (orig_proc);
            call_functionT("DT_INIT", init_func_, library_path.c_str());
            for (const auto &[name, type, callback, data]: monitor_list) {
                if (type != INIT_PROC_POST || name.empty()) {
                    continue;
                }
                if (strstr(library_path.c_str(), name.c_str()) == nullptr) {
                    continue;
                }
                if (callback != nullptr) {
                    callback(library_path.c_str(), INIT_PROC_POST, so_info, data);
                }
            }
        }
        if (orig_count > 0) {
            for (const auto &[name, type, callback, data]: monitor_list) {
                if (type != INIT_ARRAY || name.empty()) {
                    continue;
                }
                if (strstr(library_path.c_str(), name.c_str()) == nullptr) {
                    continue;
                }
                if (callback != nullptr) {
                    callback(library_path.c_str(), INIT_PROC_POST, so_info, data);
                }
            }
            linker_function_t *init_array_func = static_cast<void (**)()>(library.get_init_array());
            call_array("DT_INIT_ARRAY", init_array_func, orig_count, false, library_path.c_str());
            for (const auto &[name, type, callback, data]: monitor_list) {
                if (type != INIT_ARRAY_POST || name.empty()) {
                    continue;
                }
                if (strstr(library_path.c_str(), name.c_str()) == nullptr) {
                    continue;
                }
                if (callback != nullptr) {
                    callback(library_path.c_str(), INIT_PROC_POST, so_info, data);
                }
            }
        }
        library.set_init_array_count(orig_count);
    }

    bool Linker::add_library_monitor(const char *library_name, linker_type_t type,
                                     linker_callback_t callback,
                                     void *user_data) {
        if (type == DLOPEN || type == DLOPEN_POST) {
            init_dlopen_hook();
        } else if (type == INIT_ARRAY || type == INIT_ARRAY_POST || type == INIT_PROC ||
                   type == INIT_PROC_POST) {
            init_linker_hook();
        }
        monitor_list.emplace_back(library_name, type, callback, user_data);
        return dl_hooked && linker_hooked;
    }

    HOOK_DEF_STATIC(void *, do_dlopen, const char *path, int flags, const void *extinfo, void *caller_addr) {
        LOGD("do_dlopen:%s", path);
        if (path == nullptr) {
            return hook_do_dlopen_orig(path, flags, extinfo, caller_addr);
        }
        for (const auto &[name, type, callback, data]: monitor_list) {
            if (type != DLOPEN || name.empty()) {
                continue;
            }
            if (strstr(path, name.c_str()) == nullptr) {
                continue;
            }
            if (callback != nullptr) {
                callback(path, DLOPEN, nullptr, data);
            }
        }
        auto handler = hook_do_dlopen_orig(path, flags, extinfo, caller_addr);
        for (const auto &[name, type, callback, data]: monitor_list) {
            if (type != DLOPEN_POST || name.empty()) {
                continue;
            }
            if (strstr(path, name.c_str()) == nullptr) {
                continue;
            }
            void *so_info = Linker::getInstance()->get_so_info(path);
            if (callback != nullptr) {
                callback(path, DLOPEN_POST, so_info, data);
            }
        }
        return handler;

    }

    void Linker::init_dlopen_hook() {
        if (dl_hooked) {
            return;
        }
        if (android_get_device_api_level() < __ANDROID_API_Q__) {
            LOGE("Android version must be greater than or equal to 10.0");
            return;
        }
        void *addr = xdl_dsym(handler, "__dl__Z9do_dlopenPKciPK17android_dlextinfoPKv", nullptr);
        if (addr == nullptr) {
            LOGE("__dl__Z9do_dlopenPKciPK17android_dlextinfoPKv is null");
            return;
        }
        HookManager::getInstance()->inline_hook(addr, (void *) &hook_do_dlopen_stub,
                                                (void **) &hook_do_dlopen_orig);
        dl_hooked = true;

    }

    void Linker::init_linker_hook() {
        if (linker_hooked) {
            return;
        }
        if (android_get_device_api_level() < __ANDROID_API_Q__) {
            LOGE("Android version must be greater than or equal to 10.0");
            return;
        }
        void *addr = xdl_dsym(handler, "__dl__ZN6soinfo17call_constructorsEv", nullptr);
        if (addr == nullptr) {
            LOGE("__dl__ZN6soinfo17call_constructorsEv is null");
            return;
        }
        HookManager::getInstance()->inline_hook(addr, (void *) &hook_call_constructors_stub,
                                                (void **) &hook_call_constructors_orig);
        linker_hooked = true;
    }

    void *Linker::get_so_info(const char *library_name) {
        static addr_t *solist_head = NULL;
        if (solist_get_head == nullptr || solist_get_somain == nullptr ||
            _linker_soinfo_to_handle == nullptr ||
            _get_realpath == nullptr) {
            return nullptr;
        }
        if (!solist_head) {
            solist_head = (addr_t *) solist_get_head();
        }
        static addr_t somain = 0;
        if (!somain)
            somain = (addr_t) solist_get_somain();
#define PARAM_OFFSET(type_, member_) __##type_##__##member_##__offset_
#define STRUCT_OFFSET PARAM_OFFSET
        int STRUCT_OFFSET (solist, next) = 0;
        for (size_t i = 0; i < 1024 / sizeof(void *); i++) {
            if (*(addr_t *) ((addr_t) solist_head + i * sizeof(void *)) == somain) {
                STRUCT_OFFSET(solist, next) = i * sizeof(void *);
                break;
            }
        }
        std::vector<void *> linker_solist;
        linker_solist.push_back(solist_head);

        addr_t sonext = 0;
        sonext = *(addr_t *) ((addr_t) solist_head + STRUCT_OFFSET(solist, next));
        while (sonext) {
            linker_solist.push_back((void *) sonext);
            sonext = *(addr_t *) ((addr_t) sonext + STRUCT_OFFSET(solist, next));
        }
        for (auto &item: linker_solist) {
            auto path = _get_realpath(item);
            if (path == nullptr) {
                continue;
            }
            if (strstr(path, library_name)) {
                return item;
            }
        }

        return nullptr;
    }
}