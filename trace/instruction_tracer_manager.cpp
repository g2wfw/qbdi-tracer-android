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
#include <sys/mman.h>
#include <core/library.h>
#include <dobby.h>
#include <libgen.h>
#include "instruction_tracer_manager.h"
#include "instruction_call_back.h"
#include "core/logging/check.h"

static void *stack_base = nullptr;

InstructionTracerManager *InstructionTracerManager::get_instance() {
    static InstructionTracerManager instance;
    return &instance;
}

InstructionTracerManager::InstructionTracerManager() {
    this->vm = new QBDI::VM();

}


QBDI::VM *InstructionTracerManager::get_qbdi_vm() const {
    return this->vm;
}

const std::string &InstructionTracerManager::get_module_name() const {
    return this->module_name;
}

bool InstructionTracerManager::init(std::string name, std::string symbol) {
    this->module_name = name;
    this->symbol_name = symbol;
    alloc_fix_stack();
    vm->clearAllCache();
    auto target_module = stl::Library::find_library(name);
    if (target_module == nullptr) {
        LOGE("target module not found:%s", name.c_str());
        return false;
    }

    auto target_address = target_module->find_symbol(symbol);
    if (target_address == nullptr) {
        LOGE("target symbol not found:%s", symbol.c_str());
        return false;
    }
    this->target_trace_address = reinterpret_cast<uintptr_t>(target_address);
    auto target_range = target_module->get_library_range();
    this->module_range.base = target_range.start();
    this->module_range.end = target_range.start() + target_range.end();
    this->info_manager = std::make_unique<InstructionInfoManager>(name, module_range, vm);
    return true;
}

bool InstructionTracerManager::init(std::string name, uintptr_t offset) {
    this->module_name = name;
    alloc_fix_stack();
    vm->clearAllCache();
    auto target_module = stl::Library::find_library(name);
    if (target_module == nullptr) {
        LOGE("target module not found:%s", name.c_str());
        return false;
    }

    auto target_range = target_module->get_library_range();
    this->module_range.base = target_range.start();
    this->module_range.end = target_range.end();
    if (offset > target_range.size()) {
        LOGE("offset out of range:%s", name.c_str());
        return false;
    }
    this->target_trace_address = target_range.start() + offset;
    this->info_manager = std::make_unique<InstructionInfoManager>(name, module_range, vm);
    return true;
}

bool InstructionTracerManager::init(uintptr_t address) {
    this->target_trace_address = address;
    alloc_fix_stack();
    vm->clearAllCache();
    auto target_module = stl::Library::find_library(address);
    if (target_module == nullptr) {
        LOGE("target module not found");
        return false;
    }
    auto target_range = target_module->get_library_range();
    this->module_range.base = target_range.start();
    this->module_range.end = target_range.end();

    this->module_name = basename(target_module->get_library_name().c_str());
    auto symbol_name_ = target_module->get_symbol_by_address(address);
    if (!symbol_name_.empty()) {
        this->symbol_name = symbol_name_;
    }
    this->info_manager = std::make_unique<InstructionInfoManager>(module_name, module_range,
                                                                  vm);
    return true;
}

void InstructionTracerManager::alloc_fix_stack() {
    if (stack_base != nullptr) {
        QBDI_GPR_SET(vm->getGPRState(), QBDI::REG_SP,
                     reinterpret_cast<QBDI::rword>(stack_base) + 0x1000000);
        QBDI_GPR_SET(vm->getGPRState(), QBDI::REG_BP,
                     QBDI_GPR_GET(vm->getGPRState(), QBDI::REG_SP));
        return;
    }
    auto maps = QBDI::getCurrentProcessMaps(true);
    uintptr_t max_address = 0;
    for (auto &map: maps) {
        if (map.name.find("[stack]") != std::string::npos) {
            if (map.range.end() > max_address) {
                max_address = map.range.end();
            }
        }
    }
    stack_base = mmap((void *) max_address, 0x1000000, PROT_READ | PROT_WRITE,
                      MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK, -1, 0);
    LOGI("vm stack base:%p", stack_base);
    if (stack_base == nullptr) {
        LOGE("alloc stack fail");
        return;
    }
    QBDI_GPR_SET(vm->getGPRState(), QBDI::REG_SP,
                 reinterpret_cast<QBDI::rword>(stack_base) + 0x1000000);
    QBDI_GPR_SET(vm->getGPRState(), QBDI::REG_BP,
                 QBDI_GPR_GET(vm->getGPRState(), QBDI::REG_SP));
    LOGI("vm stack pointer:0x%zx", QBDI_GPR_GET(vm->getGPRState(), QBDI::REG_SP));
}

bool InstructionTracerManager::run(std::vector<QBDI::rword> regs) {
    QBDI::rword ret_value;
    vm->addCodeCB(QBDI::InstPosition::PREINST, pre_instruction_call, this);
    vm->addCodeCB(QBDI::InstPosition::POSTINST, post_instruction_call, this);
    vm->addVMEventCB(QBDI::VMEvent::EXEC_TRANSFER_CALL, on_fun_call, this);
    vm->addMemAccessCB(QBDI::MEMORY_READ_WRITE, on_memory_read_or_write, this);
    vm->addInstrumentedModuleFromAddr(
            reinterpret_cast<QBDI::rword>(this->target_trace_address));

    vm->addMnemonicCB("svc", QBDI::PREINST, pre_svc_instruction_call, this);
    //vm->instrumentAllExecutableMaps();
    auto result = vm->call(&ret_value, (QBDI::rword) target_trace_address, regs);
    this->info_manager->flush();
    if (!result) {
        LOGE("run fail");
        return -1;
    }
    return ret_value;
}

bool InstructionTracerManager::run_attach(inst_at_cond_t at_cond) {
    if (at_cond != nullptr) {
        this->inst_at_cond_list.emplace(target_trace_address, at_cond);
    }
    return DobbyInstrument((void *) this->target_trace_address, (dobby_instrument_callback_t) &inst_at_enter, this) ==
           0;
}

InstructionTracerManager::~InstructionTracerManager() {
    if (this->vm != nullptr) {
        this->vm->clearAllCache();
        delete this->vm;
    }

}

bool InstructionTracerManager::is_need_record(uintptr_t addr) const {
    if (this->record_ranges.empty()) {
        return true;
    }
    for (const auto &record_range: this->record_ranges) {
        if (addr >= record_range.base && addr < record_range.end) {
            return true;
        }
    }
    return false;
}

const std::unique_ptr<InstructionInfoManager> &InstructionTracerManager::get_info_manager() const {
    return info_manager;
}

bool InstructionTracerManager::is_address_in_module_range(uintptr_t addr) const {
    if (addr < this->module_range.base) {
        return false;
    }
    if (addr > this->module_range.end) {
        return false;
    }
    return true;
}

bool InstructionTracerManager::is_address_in_stack_range(uintptr_t addr) {
    uintptr_t stack_low = reinterpret_cast<uintptr_t>(stack_base);
    uintptr_t stack_high = stack_low + 0x1000000;
    if (addr >= stack_low && addr < stack_high) {
        return true;
    }
    return false;
}

bool InstructionTracerManager::add_record_range_size(uintptr_t offset, size_t size) {
    trace_range_t record_range;
    record_range.base = this->module_range.base + offset;
    record_range.end = record_range.base + size;
    if (!is_address_in_module_range(record_range.base)) {
        return false;
    }
    if (!is_address_in_module_range(record_range.end)) {
        return false;
    }
    this->record_ranges.push_back(record_range);
    return true;
}

bool InstructionTracerManager::add_record_range(uintptr_t offset, uintptr_t offset_end) {
    trace_range_t record_range;
    record_range.base = this->module_range.base + offset;
    record_range.end = this->module_range.base + offset_end;
    if (!is_address_in_module_range(record_range.base)) {
        return false;
    }
    if (!is_address_in_module_range(record_range.end)) {
        return false;
    }
    this->record_ranges.push_back(record_range);
    return true;
}

const module_range_t &InstructionTracerManager::getModuleRange() const {
    return module_range;
}

bool InstructionTracerManager::add_trace_callback_pre_hook(uint64_t offset, trace_callback_t callback, void *ud) {
    this->pre_hook_callbacks.emplace(offset, std::make_pair(callback, ud));
    return true;
}

bool InstructionTracerManager::add_trace_callback_post_hook(uint64_t offset, trace_callback_t callback, void *ud) {
    this->post_hook_callbacks.emplace(offset, std::make_pair(callback, ud));
    return true;
}


void InstructionTracerManager::trace_callback_pre(uint64_t offset, QBDI::GPRState *state, QBDI::FPRState *fprState) {
    if (pre_hook_callbacks.empty()) {
        return;
    }
    if (pre_hook_callbacks.find(offset) != pre_hook_callbacks.end()) {
        pre_hook_callbacks[offset].first(offset, state, fprState, pre_hook_callbacks[offset].second);
    }
}

void InstructionTracerManager::trace_callback_post(uint64_t offset, QBDI::GPRState *state, QBDI::FPRState *fprState) {
    if (post_hook_callbacks.empty()) {
        return;
    }
    if (post_hook_callbacks.find(offset) != post_hook_callbacks.end()) {
        post_hook_callbacks[offset].first(offset, state, fprState, post_hook_callbacks[offset].second);
    }
}

bool
InstructionTracerManager::check_attach_cond(uint64_t addr, uint32_t max_arg_count, uintptr_t *state, uintptr_t *fpr_state) {
    if (inst_at_cond_list.find(addr) != inst_at_cond_list.end()) {
        return inst_at_cond_list[addr](addr, max_arg_count, state, fpr_state);
    }
    return true;
}


