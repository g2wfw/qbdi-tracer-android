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


#ifndef QBDI_TRACER_COMMON_H
#define QBDI_TRACER_COMMON_H

#include <QBDI.h>
#include <android/log.h>
#include <cstdint>
#include <sstream>

#define LOG_TAG "QBDI"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

typedef enum fun_data_type {
    kUnknown,
    kString,
    kVariadic,
    kPointer,
    kNumber,
    kVoid,
} fun_data_type_t;

typedef struct module_range {
    uintptr_t base;
    uintptr_t end;
} module_range_t, trace_range_t;

typedef struct trace_vm_status {
    QBDI::GPRState gpr_state;
    QBDI::FPRState fpr_state;
} trace_vm_status_t;

typedef struct inst_fun_call {
    uintptr_t fun_address = 0;
    uintptr_t memory_alloc_address = 0;
    uintptr_t memory_alloc_size = 0;
    uintptr_t memory_free_address = 0;
    fun_data_type_t ret_type = kUnknown;
    bool is_svc = false;
    std::string call_module_name;
    std::string fun_name;
    std::string ret_value;
    std::vector<std::string> args = {};
} inst_fun_call_t;

typedef struct module_export_details {
    uintptr_t addr;
    const char *name;
} module_export_details_t;

typedef struct memory_info {
    uint64_t memory_index;
    uintptr_t start;
    uintptr_t end;

    memory_info(uint64_t memory_index, uintptr_t start, uintptr_t end) : memory_index(memory_index),
                                                                         start(start), end(end) {}

    memory_info() = default;
} memory_info_t;

typedef struct inst_trace_info {
    uintptr_t pc = 0;
    trace_vm_status_t pre_status{};
    trace_vm_status_t post_status{};
    inst_fun_call_t *fun_call = nullptr;
    const QBDI::InstAnalysis *inst_analysis = nullptr;
} inst_trace_info_t;


template<typename Container>
static inline std::string join(const Container &v, const char *delim) {
    if (v.empty()) {
        return "[]";
    }
    std::ostringstream os;
    os << "[";
    std::copy(v.begin(), std::prev(v.end()),
              std::ostream_iterator<typename Container::value_type>(os, delim));
    os << *(v.rbegin());
    os << "]";
    return os.str();
}


#define REGISTER_HANDLER(HANDLER_MAP, FUNC_NAME, HANDLER_BODY)                                                 \
    do {                                                                                                       \
        auto addr = gum_module_find_export_by_name(module, #FUNC_NAME);                                        \
        if (addr) {                                                                                            \
            std::function<void(inst_trace_info_t*)> handler = [](inst_trace_info_t * trace_info) HANDLER_BODY; \
            HANDLER_MAP.insert({addr, {#FUNC_NAME, handler}});                                                 \
        }                                                                                                      \
    } while (0)


#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
    TypeName(const TypeName&) = delete;    \
    void operator=(const TypeName&) = delete

#endif  //QBDI_TRACER_COMMON_H