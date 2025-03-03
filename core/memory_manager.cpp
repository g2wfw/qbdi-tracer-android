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

bool MemoryManager::add_memory(uintptr_t addr, size_t size) {
    auto end = addr + size;
    auto start = addr;
    if (is_in_memory(addr) || is_in_memory(end)) {
        return false;
    }
    this->memory_map.emplace(start, end);
    return true;
}

void MemoryManager::clear() {
    this->memory_map.clear();
}

bool MemoryManager::remove_memory(uintptr_t addr) {
    for (auto it = memory_map.begin(); it != memory_map.end();) {
        if (it->first == addr) {
            it = memory_map.erase(it);
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
    return std::any_of(this->memory_map.begin(), this->memory_map.end(), [&](const auto& range) {
        if (addr >= range.first && addr <= range.second) {
            return true;
        }
        return false;
    });
}

std::tuple<uintptr_t, size_t> MemoryManager::get_memory_offset(uintptr_t addr) {
    size_t offset = -1;
    uintptr_t start = 0;
    std::any_of(this->memory_map.begin(), this->memory_map.end(), [&](const auto& range) {
        if (addr >= range.first && addr <= range.second) {
            offset = addr - range.first;
            start = range.first;
            return true;
        }
        return false;
    });
    return {start, offset};
}