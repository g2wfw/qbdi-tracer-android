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
#include "dobby.h"

void InstructionRegisterUtils::qbdi_to_doby(QBDI::GPRState *status, void *ic_) {
    DobbyRegisterContext *ic = static_cast<DobbyRegisterContext *>(ic_);
#ifdef __arm__
    ic->general.regs.r0 = status->r0;
    ic->general.regs.r1 = status->r1;
    ic->general.regs.r2 = status->r2;
    ic->general.regs.r3 = status->r3;
    ic->general.regs.r4 = status->r4;
    ic->general.regs.r5 = status->r5;
    ic->general.regs.r6 = status->r6;
    ic->general.regs.r7 = status->r7;
    ic->general.regs.r8 = status->r8;
    ic->general.regs.r9 = status->r9;
    ic->general.regs.r10 = status->r10;
    ic->general.regs.r11 = status->r11;
    ic->general.regs.r12 = status->r12;

    ic->lr = status->lr;

#else

    ic->general.x[0] = status->x0;
    ic->general.x[1] = status->x1;
    ic->general.x[2] = status->x2;
    ic->general.x[3] = status->x3;
    ic->general.x[4] = status->x4;
    ic->general.x[5] = status->x5;
    ic->general.x[6] = status->x6;
    ic->general.x[7] = status->x7;
    ic->general.x[8] = status->x8;
    ic->general.x[9] = status->x9;
    ic->general.x[10] = status->x10;
    ic->general.x[11] = status->x11;
    ic->general.x[12] = status->x12;
    ic->general.x[13] = status->x13;
    ic->general.x[14] = status->x14;
    ic->general.x[15] = status->x15;
    ic->general.x[16] = status->x16;
    ic->general.x[17] = status->x17;
    ic->general.x[18] = status->x18;
    ic->general.x[19] = status->x19;
    ic->general.x[20] = status->x20;
    ic->general.x[21] = status->x21;
    ic->general.x[22] = status->x22;
    ic->general.x[23] = status->x23;
    ic->general.x[24] = status->x24;
    ic->general.x[25] = status->x25;
    ic->general.x[26] = status->x26;
    ic->general.x[27] = status->x27;
    ic->general.x[28] = status->x28;
    ic->sp = status->sp;
    ic->lr = status->lr;

#endif
}

void InstructionRegisterUtils::doby_to_qbdi(void *ic_, QBDI::FPRState *status) {
#ifdef __arm__
    DobbyRegisterContext *ic = static_cast<DobbyRegisterContext *>(ic_);
    //todo doby hook add vector register
    //only support common register
    return;
#else
    DobbyRegisterContext *ic = static_cast<DobbyRegisterContext *>(ic_);
    status->v0 = ic->floating.regs.q0.q;
    status->v1 = ic->floating.regs.q1.q;
    status->v2 = ic->floating.regs.q2.q;
    status->v3 = ic->floating.regs.q3.q;
    status->v4 = ic->floating.regs.q4.q;
    status->v5 = ic->floating.regs.q5.q;
    status->v6 = ic->floating.regs.q6.q;
    status->v7 = ic->floating.regs.q7.q;
    status->v8 = ic->floating.regs.q8.q;
    status->v9 = ic->floating.regs.q9.q;
    status->v10 = ic->floating.regs.q10.q;
    status->v11 = ic->floating.regs.q11.q;
    status->v12 = ic->floating.regs.q12.q;
    status->v13 = ic->floating.regs.q13.q;
    status->v14 = ic->floating.regs.q14.q;
    status->v15 = ic->floating.regs.q15.q;
    status->v16 = ic->floating.regs.q16.q;
    status->v17 = ic->floating.regs.q17.q;
    status->v18 = ic->floating.regs.q18.q;
    status->v19 = ic->floating.regs.q19.q;
    status->v20 = ic->floating.regs.q20.q;
    status->v21 = ic->floating.regs.q21.q;
    status->v22 = ic->floating.regs.q22.q;
    status->v23 = ic->floating.regs.q23.q;
    status->v24 = ic->floating.regs.q24.q;
    status->v25 = ic->floating.regs.q25.q;
    status->v26 = ic->floating.regs.q26.q;
    status->v27 = ic->floating.regs.q27.q;
    status->v28 = ic->floating.regs.q28.q;
    status->v29 = ic->floating.regs.q29.q;
    status->v30 = ic->floating.regs.q30.q;
    status->v31 = ic->floating.regs.q31.q;
#endif
}

void InstructionRegisterUtils::doby_to_qbdi(void *ic_, QBDI::GPRState *status) {
    DobbyRegisterContext *ic = static_cast<DobbyRegisterContext *>(ic_);
#ifdef __arm__
    status->r0 = ic->general.r[0];
    status->r1 = ic->general.r[1];
    status->r2 = ic->general.r[2];
    status->r3 = ic->general.r[3];
    status->r4 = ic->general.r[4];
    status->r5 = ic->general.r[5];
    status->r6 = ic->general.r[6];
    status->r7 = ic->general.r[7];
    status->r8 = ic->general.r[8];
    status->r9 = ic->general.r[9];
    status->r10 = ic->general.r[10];
    status->r11 = ic->general.r[11];
    status->r12 = ic->general.r[12];
    status->lr = ic->lr;
#else
    status->x0 = ic->general.x[0];
    status->x1 = ic->general.x[1];
    status->x2 = ic->general.x[2];
    status->x3 = ic->general.x[3];
    status->x4 = ic->general.x[4];
    status->x5 = ic->general.x[5];
    status->x6 = ic->general.x[6];
    status->x7 = ic->general.x[7];
    status->x8 = ic->general.x[8];
    status->x9 = ic->general.x[9];
    status->x10 = ic->general.x[10];
    status->x11 = ic->general.x[11];
    status->x12 = ic->general.x[12];
    status->x13 = ic->general.x[13];
    status->x14 = ic->general.x[14];
    status->x15 = ic->general.x[15];
    status->x16 = ic->general.x[16];
    status->x17 = ic->general.x[17];
    status->x18 = ic->general.x[18];
    status->x19 = ic->general.x[19];
    status->x20 = ic->general.x[20];
    status->x21 = ic->general.x[21];
    status->x22 = ic->general.x[22];
    status->x23 = ic->general.x[23];
    status->x24 = ic->general.x[24];
    status->x25 = ic->general.x[25];
    status->x26 = ic->general.x[26];
    status->x27 = ic->general.x[27];
    status->x28 = ic->general.x[28];
    status->lr = ic->lr;
    //status->sp = ic->sp;
    status->x29 = ic->fp;

#endif

}

