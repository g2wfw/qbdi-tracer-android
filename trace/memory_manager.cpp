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

#include <spdlog/fmt/fmt.h>
#include <android/api-level.h>
#include <dlfcn.h>
#include <QBDI.h>
#include <sstream>
#include "memory_manager.h"
#include "hex_dump.h"


#define LOG_TAG "QBDI"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

static int (*malloc_iterate_addr)(uintptr_t base, size_t size,
                                  void (*callback)(uintptr_t base, size_t size, void *arg),
                                  void *arg) = nullptr;

MemoryManager::MemoryManager() {
    void *handler = dlopen("libc.so", RTLD_NOW);
    if (handler == nullptr) {
        return;
    }
    malloc_iterate_addr = reinterpret_cast<decltype(malloc_iterate_addr)>(dlsym(handler,
                                                                                "malloc_iterate"));
    if (malloc_iterate_addr == nullptr) {
        LOGE("malloc_iterate not found.");
        malloc_iterate_addr = reinterpret_cast<decltype(malloc_iterate_addr)>(dlsym(handler,
                                                                                    "je_malloc_iterate"));
    }
    if (malloc_iterate_addr == nullptr) {
        LOGE("malloc_iterate not found.");
        malloc_iterate_addr = reinterpret_cast<decltype(malloc_iterate_addr)>(dlsym(handler,
                                                                                    "scudo_malloc_iterate"));
    }
    dlclose(handler);
}

void MemoryManager::set_dump_path(const std::string &path) {
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
    for (auto &&memory_info: this->memory_infos) {
        LOGI("memory block index:0x%zx size:0x%zx address:0x%zx", memory_info.memory_index,
             memory_info.end - memory_info.start, memory_info.start);
        this->write_memory_buffer(reinterpret_cast<void *>(memory_info.start),
                                  memory_info.end - memory_info.start, memory_info.memory_index);

    }
    this->memory_infos.clear();
    this->memory_dump_file.flush();
    this->memory_dump_file.close();
}


bool MemoryManager::write_memory_buffer(void *addr, size_t len, size_t index) {
    // write hexdump to file
    if (!this->memory_dump_file.is_open()) {
        return false;
    }
    //format buffer to hexdump
    this->memory_dump_file << "memory block index:"
                           << fmt::format("{:#x} size:{:#x} address:{:#x}\n", index, len,
                                          (uintptr_t) addr);
    const HexDump hexdump(addr, len);
    if (len == 16) {
        std::stringstream ss;
        ss << hexdump;
        LOGI("memory:%s", ss.str().c_str());
        this->memory_dump_file << ss.str();
    } else {
        this->memory_dump_file << hexdump;
    }
    this->memory_dump_file << "\n";
    this->memory_dump_file.flush();
    return true;
}

bool MemoryManager::remove_memory(const uintptr_t addr) {
    for (auto it = this->memory_infos.begin(); it != this->memory_infos.end();) {
        if (it->start == addr) {
            this->write_memory_buffer(reinterpret_cast<void *>(it->start), it->end - it->start,
                                      it->memory_index);
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
    auto contains = std::any_of(this->memory_infos.begin(), this->memory_infos.end(),
                                [&](const auto &memory_info) {
                                    if (addr >= memory_info.start && addr <= memory_info.end) {
                                        return true;
                                    }
                                    return false;
                                });
    return contains;
}

std::tuple<size_t, uint64_t> MemoryManager::get_memory_offset(uintptr_t addr) {
    size_t offset = -1;
    uint64_t block_index = 0;
    bool any_of = std::any_of(this->memory_infos.begin(), this->memory_infos.end(),
                              [&](const auto &memory_info) {
                                  if (addr >= memory_info.start && addr <= memory_info.end) {
                                      offset = addr - memory_info.start;
                                      block_index = memory_info.memory_index;
                                      return true;
                                  }
                                  return false;
                              });
    if (any_of) {
        return {offset, block_index};
    }
    LOGE("addr:%zx not in memory,find memory block.", addr);
    auto already_alloc_memory_info = find_already_alloc_memory_info(addr);
    if (already_alloc_memory_info.start != 0) {
        this->memory_infos.emplace_back(already_alloc_memory_info);
        return {addr - already_alloc_memory_info.start, already_alloc_memory_info.memory_index};
    }
    return {-1, 0};
}

typedef struct {
    uintptr_t addr;
    uintptr_t chunk;
    size_t size;
} malloc_chunk_t;

static void callback(uintptr_t chunk, size_t size, void *arg) {
    if (arg == nullptr) {
        return;
    }
    malloc_chunk_t *chunk_info = reinterpret_cast<malloc_chunk_t *>(arg);
    auto start = chunk;
    auto end = chunk + size;
    if (chunk_info->addr >= start && chunk_info->addr <= end) {
        chunk_info->chunk = chunk;
        chunk_info->size = size;
    }
}

memory_info_t MemoryManager::find_already_alloc_memory_info(uintptr_t addr) {
    if (malloc_iterate_addr == nullptr) {
        return {0, 0, 0};
    }
    malloc_chunk_t chunk{addr, 0, 0};
    auto memory_maps = QBDI::getCurrentProcessMaps(true);
    for (auto &memory_map: memory_maps) {
        if (memory_map.range.start() <= addr && memory_map.range.end() >= addr) {
            LOGI("find already alloc memory info:%zx", memory_map.range.start());
            malloc_iterate_addr(memory_map.range.start(), memory_map.range.size(), callback,
                                &chunk);
            if (chunk.chunk == 0) {
                return {this->memory_index.fetch_add(1), memory_map.range.start(),
                        memory_map.range.end()};
            }
            break;
        }
    }

    return {this->memory_index.fetch_add(1), chunk.chunk, chunk.chunk + chunk.size};
}