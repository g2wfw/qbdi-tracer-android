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


#include "instruction_register_utils.h"

void InstructionRegisterUtils::qbdi_to_frida(QBDI::GPRState *status, GumInvocationContext *ic) {
#ifdef __arm__
    ic->cpu_context->r[0] = status->r0;
    ic->cpu_context->r[1] = status->r1;
    ic->cpu_context->r[2] = status->r2;
    ic->cpu_context->r[3] = status->r3;
    ic->cpu_context->r[4] = status->r4;
    ic->cpu_context->r[5] = status->r5;
    ic->cpu_context->r[6] = status->r6;
    ic->cpu_context->r[7] = status->r7;
    ic->cpu_context->r8 = status->r8;
    ic->cpu_context->r9 = status->r9;
    ic->cpu_context->r10 = status->r10;
    ic->cpu_context->r11 = status->r11;
    ic->cpu_context->r12 = status->r12;
    ic->cpu_context->lr = status->lr;
    ic->cpu_context->cpsr = status->cpsr;
#else

    ic->cpu_context->x[0] = status->x0;
    ic->cpu_context->x[1] = status->x1;
    ic->cpu_context->x[2] = status->x2;
    ic->cpu_context->x[3] = status->x3;
    ic->cpu_context->x[4] = status->x4;
    ic->cpu_context->x[5] = status->x5;
    ic->cpu_context->x[6] = status->x6;
    ic->cpu_context->x[7] = status->x7;
    ic->cpu_context->x[8] = status->x8;
    ic->cpu_context->x[9] = status->x9;
    ic->cpu_context->x[10] = status->x10;
    ic->cpu_context->x[11] = status->x11;
    ic->cpu_context->x[12] = status->x12;
    ic->cpu_context->x[13] = status->x13;
    ic->cpu_context->x[14] = status->x14;
    ic->cpu_context->x[15] = status->x15;
    ic->cpu_context->x[16] = status->x16;
    ic->cpu_context->x[17] = status->x17;
    ic->cpu_context->x[18] = status->x18;
    ic->cpu_context->x[19] = status->x19;
    ic->cpu_context->x[20] = status->x20;
    ic->cpu_context->x[21] = status->x21;
    ic->cpu_context->x[22] = status->x22;
    ic->cpu_context->x[23] = status->x23;
    ic->cpu_context->x[24] = status->x24;
    ic->cpu_context->x[25] = status->x25;
    ic->cpu_context->x[26] = status->x26;
    ic->cpu_context->x[27] = status->x27;
    ic->cpu_context->x[28] = status->x28;
    ic->cpu_context->sp = status->x29;
    ic->cpu_context->lr = status->lr;  // Link Register
    ic->cpu_context->pc = status->pc;  // Program Counter
    ic->cpu_context->nzcv = status->nzcv;  // Program Counter

#endif
}

void InstructionRegisterUtils::frida_to_qbdi(GumInvocationContext *ic, QBDI::GPRState *status) {
#ifdef __arm__
    status->r0 = ic->cpu_context->r[0];
    status->r1 = ic->cpu_context->r[1];
    status->r2 = ic->cpu_context->r[2];
    status->r3 = ic->cpu_context->r[3];
    status->r4 = ic->cpu_context->r[4];
    status->r5 = ic->cpu_context->r[5];
    status->r6 = ic->cpu_context->r[6];
    status->r7 = ic->cpu_context->r[7];
    status->r8 = ic->cpu_context->r8;
    status->r9 = ic->cpu_context->r9;
    status->r10 = ic->cpu_context->r10;
    status->r11 = ic->cpu_context->r11;
    status->r12 = ic->cpu_context->r12;
    status->lr = ic->cpu_context->lr;
    status->cpsr = ic->cpu_context->cpsr;
#else
    status->x0 = ic->cpu_context->x[0];
    status->x1 = ic->cpu_context->x[1];
    status->x2 = ic->cpu_context->x[2];
    status->x3 = ic->cpu_context->x[3];
    status->x4 = ic->cpu_context->x[4];
    status->x5 = ic->cpu_context->x[5];
    status->x6 = ic->cpu_context->x[6];
    status->x7 = ic->cpu_context->x[7];
    status->x8 = ic->cpu_context->x[8];
    status->x9 = ic->cpu_context->x[9];
    status->x10 = ic->cpu_context->x[10];
    status->x11 = ic->cpu_context->x[11];
    status->x12 = ic->cpu_context->x[12];
    status->x13 = ic->cpu_context->x[13];
    status->x14 = ic->cpu_context->x[14];
    status->x15 = ic->cpu_context->x[15];
    status->x16 = ic->cpu_context->x[16];
    status->x17 = ic->cpu_context->x[17];
    status->x18 = ic->cpu_context->x[18];
    status->x19 = ic->cpu_context->x[19];
    status->x20 = ic->cpu_context->x[20];
    status->x21 = ic->cpu_context->x[21];
    status->x22 = ic->cpu_context->x[22];
    status->x23 = ic->cpu_context->x[23];
    status->x24 = ic->cpu_context->x[24];
    status->x25 = ic->cpu_context->x[25];
    status->x26 = ic->cpu_context->x[26];
    status->x27 = ic->cpu_context->x[27];
    status->x28 = ic->cpu_context->x[28];
    status->lr = ic->cpu_context->lr;
    status->nzcv = ic->cpu_context->nzcv;
#endif

}
