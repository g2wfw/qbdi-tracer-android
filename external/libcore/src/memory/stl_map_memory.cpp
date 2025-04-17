/*
 * @author: xiaobai
 * @email: xiaobaiyey@outlook.com
 * @date: 2023/12/13 12:20
 * @version: 1.0
 * @description: 
*/

#include "memory/stl_map_memory.h"
#include "logging/stllog.h"
#include <memory>

#ifdef STL_MINGW
#include <windows.h>
#endif

#define ALIGN_TO_4K(size) ((size + 4095) & ~4095)


namespace stl {


    MapMemory::MapMemory() {
    }

    void *MapMemory::mmap(size_t length) {
        return getInstance()->mapAnonymous(length);
    }

    MapMemory *MapMemory::getInstance() {
        static MapMemory memory;
        return &memory;
    }

    void *MapMemory::mapAnonymous(size_t length) {
        auto len = ALIGN_TO_4K(length);
#ifdef STL_MINGW
        auto ptr = VirtualAlloc(nullptr, len, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
        if (ptr == nullptr) {
            error("VirtualAlloc fail for {}", length);
            return nullptr;
        }
        this->memory_maps.set(ptr, length);
        return ptr;
#elif defined(STL_MAC)
        auto ptr = ::mmap(nullptr, len, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if (ptr == MAP_FAILED) {
            error("mmap fail for {}", length);
            return nullptr;
        }
        this->memory_maps.set(ptr, length);
        return ptr;
#else

        auto ptr = ::mmap64(nullptr, len, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if (ptr == MAP_FAILED) {
            error("mmap fail for {}", length);
            return nullptr;
        }
        this->memory_maps.set(ptr, length);
        return ptr;
#endif

    }

    void MapMemory::munmap(void *address) {
        return getInstance()->munmapAnonymous(address);
    }

    void MapMemory::munmapAnonymous(void *ptr) {
        if (ptr == nullptr) {
            return;
        }
        if (!this->memory_maps.contains(ptr)) {
            return;
        }
        auto length = this->memory_maps[ptr];
#ifdef STL_MINGW
        VirtualFree(ptr, 0, MEM_RELEASE);
        ptr = nullptr;
#else
        if (::munmap(ptr, length) == -1) {
            error("munmap failed:{}", ptr);

        }
#endif

        this->memory_maps.remove(ptr);
    }

} // stl