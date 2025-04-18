/*
 * MIT License
 * 
 * Copyright (c) 2024 g2wfw
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */


#ifndef QBDI_TRACER_MEMORY_MANAGER_H
#define QBDI_TRACER_MEMORY_MANAGER_H

#include <atomic>
#include <fstream>
#include <vector>
#include "common.h"


class MemoryManager {
public:
    MemoryManager();

    ~MemoryManager() = default;

    void set_dump_path(const std::string& path);

    bool add_memory(uintptr_t addr, size_t size);

    void clear();

    bool remove_memory(uintptr_t addr);

    bool is_in_memory(uintptr_t addr);

    std::tuple<size_t, uint64_t> get_memory_offset(uintptr_t addr);

private:
    memory_info_t find_already_alloc_memory_info(uintptr_t addr);


    bool write_memory_buffer(void* addr, size_t len, size_t index);

private:
    std::ofstream memory_dump_file;
    std::atomic_uint64_t memory_index = 0;
    std::string dump_path;
    std::vector<memory_info_t> memory_infos;
    DISALLOW_COPY_AND_ASSIGN(MemoryManager);
};


#endif  //QBDI_TRACER_MEMORY_MANAGER_H