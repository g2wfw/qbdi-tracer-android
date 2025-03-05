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


#ifndef QBDI_TRACER_INSTRUCTION_INFO_MANAGER_H
#define QBDI_TRACER_INSTRUCTION_INFO_MANAGER_H

#include <string>
#include <memory>
#include <QBDI.h>
#include "common.h"
#include "instruction_dispatch_manager.h"
#include "logger_manager.h"

class InstructionInfoManager {
public:
    InstructionInfoManager(std::string name, module_range_t module_base, QBDI::VM* vm)
        : vm(vm),
          module_name(
              std::move(name)),
          module_range(
              module_base) {
        this->dispatch_manager = InstructionDispatchManager::getInstance();
        this->logger = std::make_unique<LoggerManager>(module_name, module_base);
    };

    ~InstructionInfoManager() {
        if (this->pre_info != nullptr) {
            if (this->pre_info->fun_call != nullptr) {
                delete this->pre_info->fun_call;
            }
            delete this->pre_info;
        }
        if (this->cur_info != nullptr) {
            if (this->cur_info->fun_call != nullptr) {
                delete this->cur_info->fun_call;
            }
            delete this->cur_info;
        }
    };

    inst_trace_info_t* alloc_inst_trace_info(uintptr_t pc);

    inst_trace_info_t* get_current_inst_trace_info() const;

    inst_trace_info_t* get_previous_inst_trace_info() const;

    inst_trace_info_t* alloc_fun_call(uintptr_t pc);

    void dispatch_fun_call_args(uintptr_t pc) const;

    void dispatch_fun_call_common_args(uintptr_t pc) const;

    void dispatch_fun_call_return(const QBDI::GPRState* state) const;

    void dispatch_fun_call_common_return(const QBDI::GPRState* state) const;

    void write_trace_info(const QBDI::InstAnalysis* instAnalysis,
                          std::vector<QBDI::MemoryAccess>& memoryAccesses) const;

    void set_enable_to_logcat(bool enable) const;

    void set_enable_to_file(bool enable) const;
    void set_memory_dump_to_file(bool enable) const;

private:
    static void add_common_reg_values(inst_trace_info_t* info);

private:
    QBDI::VM* vm = nullptr;
    std::string module_name;
    module_range_t module_range;
    inst_trace_info_t* pre_info = nullptr;
    inst_trace_info_t* cur_info = nullptr;
    InstructionDispatchManager* dispatch_manager;
    std::unique_ptr<LoggerManager> logger;
};


#endif //QBDI_TRACER_INSTRUCTION_INFO_MANAGER_H