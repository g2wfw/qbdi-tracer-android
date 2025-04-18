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


#ifndef QBDI_TRACER_INSTRUCTION_REGISTER_UTILS_H
#define QBDI_TRACER_INSTRUCTION_REGISTER_UTILS_H


#include <QBDI.h>


class InstructionRegisterUtils {
public:
    /**
     * copy qbdi regs to frida regs
     * @param status qbdi regs
     * @param ic frida regs
     */
    static void qbdi_to_doby(QBDI::GPRState* status, void * ic);

    /**
     * copy frida regs to qbdi regs
     * has bugs app will crash
     * todo fix bugs
     * @param ic frida regs
     * @param status qbdi regs
     */
    static void doby_to_qbdi(void *ic, QBDI::GPRState* status);

    static void doby_to_qbdi(void *ic, QBDI::FPRState* status);


};


#endif //QBDI_TRACER_INSTRUCTION_REGISTER_UTILS_H