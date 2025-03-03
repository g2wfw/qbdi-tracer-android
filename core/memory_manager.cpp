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


#include "memory_manager.h"

MemoryManager* MemoryManager::get_instance() {
    static MemoryManager manager;
    return &manager;
}

void MemoryManager::set_dump_path(const std::string& path) {
    this->dump_path = path;
    this->memory_dump_file.open(path, std::ios::out | std::ios::trunc | std::ios::binary);
    if (this->memory_dump_file.is_open()) {
        LOGI("Memory dump file opened.");
    }
}
bool MemoryManager::add_memory(const uintptr_t addr, const size_t size) {
    auto end = addr + size;
    auto start = addr;
    if (is_in_memory(addr) || is_in_memory(end)) {
        return false;
    }
    this->memory_infos.emplace_back(this->memory_index.load(), start, end);
    this->memory_index.fetch_add(1);
    return true;
}

void MemoryManager::clear() {
    this->memory_infos.clear();
    this->memory_dump_file.flush();
    this->memory_dump_file.close();
}
size_t MemoryManager::write_memory_buffer(void* addr, size_t len) {
    // write hexdump to file
    if (!this->memory_dump_file.is_open()) {
        return 0;
    }


}
bool MemoryManager::remove_memory(const uintptr_t addr) {
    for (auto it = this->memory_infos.begin(); it != this->memory_infos.end();) {
        if (it->start == addr) {
            it = memory_infos.erase(it);
            return true;
        }
        ++it;
    }
    return false;
}

bool MemoryManager::is_in_memory(uintptr_t addr) {
    if (addr == 0) {
        return false;
    }
    return std::any_of(this->memory_infos.begin(), this->memory_infos.end(), [&](const auto& memory_info) {
        if (addr >= memory_info.start && addr <= memory_info.end) {
            return true;
        }
        return false;
    });
}

std::tuple<uintptr_t, size_t> MemoryManager::get_memory_offset(uintptr_t addr) {
    size_t offset = -1;
    uintptr_t start = 0;
    std::any_of(this->memory_infos.begin(), this->memory_infos.end(), [&](const auto& memory_info) {
        if (addr >= memory_info.start && addr <= memory_info.end) {
            offset = addr - memory_info.start;
            start = memory_info.start;
            return true;
        }
        return false;
    });
    return {start, offset};
}