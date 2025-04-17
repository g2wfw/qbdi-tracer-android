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

#include "memory.h"

namespace stl{
    std::vector<MemoryMap_t> Memory::getMemoryMap(bool full_path) {
        return getMemoryMap(pid, full_path);
    }

    std::vector<MemoryMap_t> Memory::getMemoryMap(pid_t pid, bool full_path) {
        static const int BUFFER_SIZE = 256;
        char line[BUFFER_SIZE] = {0};
        FILE *mapfile = nullptr;
        std::vector<MemoryMap_t> maps;
        snprintf(line, BUFFER_SIZE, "/proc/%llu/maps", (unsigned long long) pid);
        mapfile = fopen(line, "r");
        if (mapfile == nullptr) {
            return maps;
        }
        while (fgets(line, BUFFER_SIZE, mapfile) != nullptr) {
            char *ptr = nullptr;
            MemoryMap m;

            // Remove \n
            if ((ptr = strchr(line, '\n')) != nullptr) {
                *ptr = '\0';
            }
            ptr = line;
            // Read range
            m.range.setStart(strtoul(ptr, &ptr, 16));
            ptr++; // '-'
            m.range.setEnd(strtoul(ptr, &ptr, 16));

            // skip the spaces
            while (isspace(*ptr))
                ptr++;

            // Read the permission
            m.permission = PF_NONE;
            if (*ptr == 'r')
                m.permission |= PF_READ;
            ptr++;
            if (*ptr == 'w')
                m.permission |= PF_WRITE;
            ptr++;
            if (*ptr == 'x')
                m.permission |= PF_EXEC;
            ptr++;
            ptr++; // skip the protected

            // skip the spaces
            while (isspace(*ptr))
                ptr++;

            // Discard the file offset
            strtoul(ptr, &ptr, 16);

            // skip the spaces
            while (isspace(*ptr))
                ptr++;

            // Discard the device id
            strtoul(ptr, &ptr, 16);
            ptr++; // ':'
            strtoul(ptr, &ptr, 16);

            // skip the spaces
            while (isspace(*ptr))
                ptr++;

            // Discard the inode
            strtoul(ptr, &ptr, 10);

            // skip the spaces
            while (isspace(*ptr))
                ptr++;

            // Get the file name
            if (full_path) {
                m.name = ptr;
            } else {
                if ((ptr = strrchr(ptr, '/')) != nullptr) {
                    m.name = ptr + 1;
                } else {
                    m.name.clear();
                }
            }
            maps.push_back(m);
        }
        fclose(mapfile);
        return maps;
    }
} // rt