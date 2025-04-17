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

#ifndef REVERSE_TOOLS_MEMORY_H
#define REVERSE_TOOLS_MEMORY_H

#include <cstdint>
#include <vector>
#include <unistd.h>
#include "range.h"
#include "common.h"

namespace stl{
    /*! Memory access rights.
     */
    typedef enum {
        PF_NONE = 0,  /*!< No access */
        PF_READ = 1,  /*!< Read access */
        PF_WRITE = 2, /*!< Write access */
        PF_EXEC = 4   /*!< Execution access */
    } Permission;
    ENABLE_BITMASK_OPERATORS(Permission);
    typedef struct MemoryMap {

        Range<uintptr_t> range;    /*!< A range of memory (region), delimited between
                          * a start and an (excluded) end address.
                          */
        Permission permission; /*!< Region access rights
                          * (PF_READ, PF_WRITE, PF_EXEC).
                          */
        std::string name;      /*!< Region name or path (useful when a region
                          * is mapping a module).
                          */

        /* Construct a new (empty) MemoryMap.
         */
        MemoryMap() : range(0, 0), permission(PF_NONE) {};

        /*! Construct a new MemoryMap (given some properties).
         *
         * @param[in] start        Range start value.
         * @param[in] end          Range end value (always excluded).
         * @param[in] permission   Region access rights (PF_READ, PF_WRITE, PF_EXEC).
         * @param[in] name         Region name (useful when a region is mapping
         *                         a module).
         */
        MemoryMap(uintptr_t start, uintptr_t end, Permission permission, std::string name)
                : range(start, end), permission(permission), name(name) {}

        /*! Construct a new MemoryMap (given some properties).
         *
         * @param[in] range        A range of memory (region), delimited between
         *                         a start and an (excluded) end address.
         * @param[in] permission   Region access rights (PF_READ, PF_WRITE, PF_EXEC).
         * @param[in] name         Region name (useful when a region is mapping
         *                         a module).
         */
        MemoryMap(Range<uintptr_t> range, Permission permission, std::string name)
                : range(range), permission(permission), name(name) {}

        //permission can read
        bool isRead() {
            return permission & PF_READ;
        }

        bool isWrite() {
            return permission & PF_WRITE;
        }

        bool isExec() {
            return permission & PF_EXEC;
        }

    } MemoryMap_t;

    class Memory {
    public:
        Memory(pid_t pid) : pid(pid) {}

        Memory() : pid(getpid()) {}

        ~Memory() = default;

        std::vector<MemoryMap_t> getMemoryMap(bool full_path = true);

        std::vector<MemoryMap_t> getMemoryMap(pid_t pid, bool full_path = true);

    private:
        pid_t pid;
    private:
        DISALLOW_COPY_AND_ASSIGN(Memory);
    };

} // rt

#endif //REVERSE_TOOLS_MEMORY_H
