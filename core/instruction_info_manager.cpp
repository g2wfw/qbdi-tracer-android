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


#include "instruction_info_manager.h"

void InstructionInfoManager::set_enable_to_logcat(bool enable) const
{
    this->logger->set_enable_to_logcat(enable);
}

void InstructionInfoManager::set_enable_to_file(bool enable) const
{
    this->logger->set_enable_to_file(enable);
}

inst_trace_info_t *InstructionInfoManager::alloc_inst_trace_info(uintptr_t pc) {
    if (cur_info == nullptr) {
        this->cur_info = new inst_trace_info_t();
        this->cur_info->pc = pc;
        return cur_info;
    }
    if (pre_info != nullptr) {
        if (pre_info->fun_call != nullptr) {
            delete pre_info->fun_call;
        }
        delete pre_info;
        this->pre_info = nullptr;
    }
    this->pre_info = cur_info;
    this->cur_info = new inst_trace_info_t();
    this->cur_info->pc = pc;
    return cur_info;
}

inst_trace_info_t *InstructionInfoManager::get_current_inst_trace_info() {
    return this->cur_info;
}

inst_trace_info_t *InstructionInfoManager::alloc_fun_call(uintptr_t pc) {
    if (cur_info == nullptr) {
        alloc_inst_trace_info(pc);
    }
    if (cur_info->fun_call == nullptr) {
        cur_info->fun_call = new inst_fun_call_t();
    }
    if (cur_info->pc != pc) {
        LOGE("pc is not equal with cur_info");
        return nullptr;
    }
    return cur_info;
}


inst_trace_info_t *InstructionInfoManager::dispatch_fun_call_args(uintptr_t pc) {
    auto jump_target_address = pc;
    cur_info->fun_call->fun_address = jump_target_address;
    this->dispatch_manager->dispatch_args(cur_info);
    return cur_info;
}

inst_trace_info_t *InstructionInfoManager::dispatch_fun_call_return(const QBDI::GPRState *state) {
    this->dispatch_manager->dispatch_ret(pre_info, state);
    return pre_info;
}

void InstructionInfoManager::write_trace_info(const QBDI::InstAnalysis *instAnalysis,
                                              std::vector<QBDI::MemoryAccess> &memoryAccesses) {
    if (this->pre_info != nullptr && this->pre_info->pc == instAnalysis->address) {
        this->logger->write_trace_info(pre_info, instAnalysis, memoryAccesses);
    }
    if (this->cur_info != nullptr && this->cur_info->pc == instAnalysis->address) {
        this->logger->write_trace_info(cur_info, instAnalysis, memoryAccesses);
    }

}

inst_trace_info_t *InstructionInfoManager::get_previous_inst_trace_info() {
    return this->pre_info;
}

inst_trace_info_t *InstructionInfoManager::dispatch_fun_call_common_args(uintptr_t pc) {
    auto jump_target_address = pc;
    cur_info->fun_call->fun_address = jump_target_address;
    cur_info->fun_call->call_module_name = this->module_name;
    add_common_reg_values(cur_info);
    return cur_info;
}

bool InstructionInfoManager::add_common_reg_values(inst_trace_info_t *info) {
    auto instCall = info->fun_call;
    auto state = info->pre_status.gpr_state;
#ifdef __arm__
    instCall->args.push_back(fmt::format("R0={:#x}", state.r0));
    instCall->args.push_back(fmt::format("R1={:#x}", state.r1));
    instCall->args.push_back(fmt::format("R2={:#x}", state.r2));
    instCall->args.push_back(fmt::format("R3={:#x}", state.r3));
#else
    instCall->args.push_back(fmt::format("X0={:#x}", state.x0));
    instCall->args.push_back(fmt::format("X1={:#x}", state.x1));
    instCall->args.push_back(fmt::format("X2={:#x}", state.x2));
    instCall->args.push_back(fmt::format("X3={:#x}", state.x3));
    instCall->args.push_back(fmt::format("X4={:#x}", state.x4));
    instCall->args.push_back(fmt::format("X5={:#x}", state.x5));
    instCall->args.push_back(fmt::format("X6={:#x}", state.x6));
    instCall->args.push_back(fmt::format("X7={:#x}", state.x7));
#endif
    return true;
}



inst_trace_info_t *InstructionInfoManager::dispatch_fun_call_common_return(const QBDI::GPRState *state) {
    add_common_return_value(state);
    return pre_info;
}

bool InstructionInfoManager::add_common_return_value(const QBDI::GPRState *state) {
    auto instCall = pre_info->fun_call;
#if __arm__
    instCall->ret_type = kUnknown;
    instCall->ret_value = fmt::format("ret={:#x}", state->r0);
#else
    instCall->ret_type = kUnknown;
    instCall->ret_value = fmt::format("ret={:#x}", state->x0);
#endif
    return true;
}








