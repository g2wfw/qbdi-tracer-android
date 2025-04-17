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


#ifndef QBDI_TRACER_INSTRUCTION_TRACER_MANAGER_H
#define QBDI_TRACER_INSTRUCTION_TRACER_MANAGER_H

#include <QBDI.h>
#include <string>
#include "common.h"
#include "instruction_info_manager.h"

typedef void(*trace_callback_t)(uint64_t offset, QBDI::GPRState *state, QBDI::FPRState *fprState, void *ud);

typedef bool(*inst_at_cond_t)(uint64_t offset, uint32_t max_arg_count, uintptr_t *state, uintptr_t *fpr_state);

class InstructionTracerManager {
public:
    static InstructionTracerManager *get_instance();

    bool add_trace_callback_pre_hook(uint64_t offset, trace_callback_t callback, void *ud);

    bool add_trace_callback_post_hook(uint64_t offset, trace_callback_t callback, void *ud);

    /**
     * run tracer with args
     * @param regs register values
     * @return true if run success
     */
    [[nodiscard]]  bool run(std::vector<QBDI::rword> regs = {});

    /**
     * run tracer with frida attach
     */
    bool run_attach(inst_at_cond_t at_cond = nullptr);

    /**
     * init tracer manager
     * @param name target loaded library name
     * @param symbol target loaded library symbol
     * @return true if init success
     */
    [[nodiscard]]  bool init(std::string name, std::string symbol);

    /**
     * init tracer manager
     * @param name target loaded library name
     * @param offset target loaded library offset
     * @return true if init success
     */
    [[nodiscard]] bool init(std::string name, uintptr_t offset);

    /**
     * init tracer manager
     * @param address target loaded library abs address
     * @return true if init success
     */
    [[nodiscard]] bool init(uintptr_t address);


    /**
     * get qbdi vm
     * @return qbdi vm
     */
    QBDI::VM *get_qbdi_vm() const;

    /**
     * get module name
     * @return module name
     */
    const std::string &get_module_name() const;

    /**
     * if set record range,only allow log out in record range
     * @param addr target address
     * @return true if need record
     */
    [[nodiscard]] bool is_need_record(uintptr_t addr) const;

    /**
     * check address is in module range
     * @param addr target address
     * @return true if in module range
     */
    [[nodiscard]] bool is_address_in_module_range(uintptr_t addr) const;


    [[nodiscard]] bool is_address_in_stack_range(uintptr_t addr);


    bool add_record_range_size(uintptr_t offset, size_t size);

    bool add_record_range(uintptr_t offset, uintptr_t offset_end);

    ~InstructionTracerManager();


    const std::unique_ptr<InstructionInfoManager> &get_info_manager() const;

    const module_range_t &getModuleRange() const;

    void trace_callback_pre(uint64_t offset, QBDI::GPRState *state, QBDI::FPRState *fprState);

    bool check_attach_cond(uint64_t addr, uint32_t max_arg_count, uintptr_t *args, uintptr_t *fpr_arg);

    void trace_callback_post(uint64_t offset, QBDI::GPRState *state, QBDI::FPRState *fprState);

private:
    InstructionTracerManager();

    void alloc_fix_stack();

private:
    std::unordered_map<uint64_t, std::pair<trace_callback_t, void *>> pre_hook_callbacks;
    std::unordered_map<uint64_t, std::pair<trace_callback_t, void *>> post_hook_callbacks;
    std::unordered_map<uint64_t, inst_at_cond_t> inst_at_cond_list;
    //qbdi vm
    QBDI::VM *vm = nullptr;
    //instruction trace range
    std::vector<trace_range_t> record_ranges;
    //target address
    uintptr_t target_trace_address = 0;
    //trace library name
    std::string module_name;
    //trace symbol name
    std::string symbol_name;
    //trace library memory range
    module_range_t module_range;
    std::unique_ptr<InstructionInfoManager> info_manager;

};


#endif //QBDI_TRACER_INSTRUCTION_TRACER_MANAGER_H
