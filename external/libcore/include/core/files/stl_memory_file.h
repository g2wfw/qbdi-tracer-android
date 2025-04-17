#pragma once/*
 * @author: xiaobai
 * @email: xiaobaiyey@outlook.com
 * @date: 2023/11/8 13:01
 * @version: 1.0
 * @description: 
*/

#ifndef MANXI_CORE_MEMORY_FILE_H
#define MANXI_CORE_MEMORY_FILE_H

#include <cstddef>
#include <cstdint>
#include "../stl_macro.h"
#if defined(_WIN32)

#include <windows.h>
#ifdef ERROR
#undef ERROR
#endif

#else

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#endif

#include <stdexcept>
#include <cstdint>

namespace stl {
    class STL_EXPORT MemoryFile {
    public:
        MemoryFile(const char *path, bool read_only = false);

        ~MemoryFile();

        bool is_open() const;

        bool isOpen() const;

        size_t size() const;

        void sync() const;

        uint8_t *data() const;

        bool flush();

    private:
        void cleanup();

#if defined(_WIN32)
        HANDLE hFile_;
        HANDLE hMapping_;
#else
        int fd_;
#endif
        size_t offset_;
        size_t size_;
        uint8_t *buf_;
        bool read_only_ = false;
        //当前数据位置
        mutable uint32_t postion_ = 0;
    };
}


#endif //MANXI_CORE_MEMORY_FILE_H
