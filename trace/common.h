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
#include <core/stl_macro.h>

typedef enum fun_data_type {
    kUnknown = 0,
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

typedef struct serialize_file {
    uint32_t magic = 0xDEADBEEF;
    uint32_t version = 0x00000001;
    uint32_t check_sum = 0;
    bool memory_enable = false;
    bool is_64bit = false;

    uint64_t inst_count;
    uint64_t inst_offset;

    uint64_t module_base = 0;
    uint64_t module_end = 0;

    char module_name[64];
} serialize_file_t;


#define REGISTER_HANDLER(HANDLER_MAP, FUNC_NAME, HANDLER_BODY)                                                 \
    do {                                                                                                       \
        uintptr_t addr =(uintptr_t) module->find_symbol(#FUNC_NAME);                                        \
        if (addr!=0) {                                                                                            \
            std::function<void(inst_trace_info_t*)> handler = [](inst_trace_info_t * trace_info) HANDLER_BODY; \
            HANDLER_MAP.insert({addr, {#FUNC_NAME, handler}});                                                 \
        }                                                                                                      \
    } while (0)




#ifdef __aarch64__
#define QBDI_FPR_GET_B(state, i) (reinterpret_cast<const uint8_t *>(state)[i]);
#define QBDI_FPR_GET_H(state, i) (reinterpret_cast<const uint16_t *>(state)[i]);
#define QBDI_FPR_GET_S(state, i) (reinterpret_cast<const uint32_t *>(state)[i]);
#define QBDI_FPR_GET_D(state, i) (reinterpret_cast<const uint64_t *>(state)[i]);
#define QBDI_FPR_GET_V(state, i) (reinterpret_cast<const __uint128_t *>(state)[i]);
#define QBDI_FPR_GET_Q(state, i) (reinterpret_cast<const __uint128_t *>(state)[i]);


#define QBDI_FPR_SET_B(state, i, v) (reinterpret_cast<uint8_t *>(state)[i]=(uint8_t)v);
#define QBDI_FPR_SET_H(state, i, v) (reinterpret_cast<uint16_t *>(state)[i]=(uint16_t)v);
#define QBDI_FPR_SET_S(state, i, v) (reinterpret_cast<uint32_t *>(state)[i]=(uint32_t)v);
#define QBDI_FPR_SET_D(state, i, v) (reinterpret_cast<uint64_t *>(state)[i]=(uint64_t)v);
#define QBDI_FPR_SET_V(state, i, v) (reinterpret_cast<__uint128_t *>(state)[i]=(__uint128_t)v);
#define QBDI_FPR_SET_Q(state, i, v) (reinterpret_cast<__uint128_t *>(state)[i]=(__uint128_t)v);

enum vector_reg_id {
    H0 = 0,
    H1 = 8,
    H2 = 0x10,
    H3 = 0x18,
    H4 = 0x20,
    H5 = 0x28,
    H6 = 0x30,
    H7 = 0x38,
    H8 = 0x40,
    H9 = 0x48,
    H10 = 0x50,
    H11 = 0x58,
    H12 = 0x60,
    H13 = 0x68,
    H14 = 0x70,
    H15 = 0x78,
    H16 = 0x80,
    H17 = 0x88,
    H18 = 0x90,
    H19 = 0x98,
    H20 = 0xa0,
    H21 = 0xa8,
    H22 = 0xb0,
    H23 = 0xb8,
    H24 = 0xc0,
    H25 = 0xc8,
    H26 = 0xd0,
    H27 = 0xd8,
    H28 = 0xe0,
    H29 = 0xe8,
    H30 = 0xf0,
    H31 = 0xf8,

    Q0 = 0,
    Q1 = 1,
    Q2 = 2,
    Q3 = 3,
    Q4 = 4,
    Q5 = 5,
    Q6 = 6,
    Q7 = 7,
    Q8 = 8,
    Q9 = 9,
    Q10 = 10,
    Q11 = 11,
    Q12 = 12,
    Q13 = 13,
    Q14 = 14,
    Q15 = 15,
    Q16 = 16,
    Q17 = 17,
    Q18 = 18,
    Q19 = 19,
    Q20 = 20,
    Q21 = 21,
    Q22 = 22,
    Q23 = 23,
    Q24 = 24,
    Q25 = 25,
    Q26 = 26,
    Q27 = 27,
    Q28 = 28,
    Q29 = 29,
    Q30 = 30,
    Q31 = 31,


    D0 = 0x0,
    D1 = 0x2,
    D2 = 0x4,
    D3 = 0x6,
    D4 = 0x8,
    D5 = 0xa,
    D6 = 0xc,
    D7 = 0xe,
    D8 = 0x10,
    D9 = 0x12,
    D10 = 0x14,
    D11 = 0x16,
    D12 = 0x18,
    D13 = 0x1a,
    D14 = 0x1c,
    D15 = 0x1e,
    D16 = 0x20,
    D17 = 0x22,
    D18 = 0x24,
    D19 = 0x26,
    D20 = 0x28,
    D21 = 0x2a,
    D22 = 0x2c,
    D23 = 0x2e,
    D24 = 0x30,
    D25 = 0x32,
    D26 = 0x34,
    D27 = 0x36,
    D28 = 0x38,
    D29 = 0x3a,
    D30 = 0x3c,
    D31 = 0x3e,

    S0 = 0x0,
    S1 = 0x4,
    S2 = 0x8,
    S3 = 0xc,
    S4 = 0x10,
    S5 = 0x14,
    S6 = 0x18,
    S7 = 0x1c,
    S8 = 0x20,
    S9 = 0x24,
    S10 = 0x28,
    S11 = 0x2c,
    S12 = 0x30,
    S13 = 0x34,
    S14 = 0x38,
    S15 = 0x3c,
    S16 = 0x40,
    S17 = 0x44,
    S18 = 0x48,
    S19 = 0x4c,
    S20 = 0x50,
    S21 = 0x54,
    S22 = 0x58,
    S23 = 0x5c,
    S24 = 0x60,
    S25 = 0x64,
    S26 = 0x68,
    S27 = 0x6c,
    S28 = 0x70,
    S29 = 0x74,
    S30 = 0x78,
    S31 = 0x7c,
#ifdef B0
#undef B0
#endif
    B0 = 0,
    B1 = 0x10,
    B2 = 0x20,
    B3 = 0x30,
    B4 = 0x40,
    B5 = 0x50,
    B6 = 0x60,
    B7 = 0x70,
    B8 = 0x80,
    B9 = 0x90,
    B10 = 0xa0,
    B11 = 0xb0,
    B12 = 0xc0,
    B13 = 0xd0,
    B14 = 0xe0,
    B15 = 0xf0,
    B16 = 0x100,
    B17 = 0x110,
    B18 = 0x120,
    B19 = 0x130,
    B20 = 0x140,
    B21 = 0x150,
    B22 = 0x160,
    B23 = 0x170,
    B24 = 0x180,
    B25 = 0x190,
    B26 = 0x1a0,
    B27 = 0x1b0,
    B28 = 0x1c0,
    B29 = 0x1d0,
    B30 = 0x1e0,
    B31 = 0x1f0,
};

#endif

#endif  //QBDI_TRACER_COMMON_H