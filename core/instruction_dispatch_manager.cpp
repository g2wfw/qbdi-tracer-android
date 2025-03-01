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


#include "instruction_dispatch_manager.h"
#include "dispatch/dispatch_jni_env.h"
#include "dispatch/dispatch_libz.h"
#include "dispatch/dispatch_libc.h"
#include "dispatch/dispatch_syscall.h"

bool InstructionDispatchManager::dispatch_args(inst_trace_info_t *call) {
    if (call->fun_call != nullptr && call->fun_call->is_svc) {
        DispatchSyscall::get_instance()->dispatch_args(call);
        return true;
    }
    for (const auto &item: this->dispatch_list) {
        if (item->dispatch_args(call)) {
            return true;
        }
    }
    return false;
}

bool InstructionDispatchManager::dispatch_ret(inst_trace_info_t *call,
                                              const QBDI::GPRState *ret_status) {
    if (call->fun_call != nullptr && call->fun_call->is_svc) {
        DispatchSyscall::get_instance()->dispatch_ret(call, ret_status);
        return true;
    }
    for (const auto &item: this->dispatch_list) {
        if (item->dispatch_ret(call, ret_status)) {
            return true;
        }
    }
    return false;
}

InstructionDispatchManager::InstructionDispatchManager() {
    this->dispatch_list.emplace_back(DispatchLibc::get_instance());
    this->dispatch_list.emplace_back(DispatchLibz::get_instance());
    this->dispatch_list.emplace_back(DispatchJNIEnv::get_instance());
}
