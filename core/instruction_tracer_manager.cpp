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
#include "instruction_tracer_manager.h"
#include "instruction_call_back.h"

static void *stack_base = nullptr;

InstructionTracerManager *InstructionTracerManager::get_instance() {
    static InstructionTracerManager instance;
    return &instance;
}

InstructionTracerManager::InstructionTracerManager() {
    this->vm = new QBDI::VM();
    this->interceptor = gum_interceptor_obtain();

}

GumInterceptor *InstructionTracerManager::get_interceptor() const {
    return this->interceptor;
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
    auto target_module = gum_process_find_module_by_name(name.c_str());
    if (target_module == nullptr) {
        LOGE("target module not found:%s", name.c_str());
        return false;
    }
    gum_module_ensure_initialized(target_module);
    auto target_address = gum_module_find_export_by_name(target_module, symbol.c_str());
    if (target_address == 0) {
        LOGE("target symbol not found:%s", symbol.c_str());
        g_object_unref(target_module);
        return false;
    }
    this->target_trace_address = target_address;
    auto target_range = gum_module_get_range(target_module);
    this->module_range.base = target_range->base_address;
    this->module_range.end = target_range->base_address + target_range->size;
    g_object_unref(target_module);
    this->info_manager = std::make_unique<InstructionInfoManager>(name, module_range, vm);
    return true;
}

bool InstructionTracerManager::init(std::string name, uintptr_t offset) {
    this->module_name = name;
    alloc_fix_stack();
    vm->clearAllCache();
    auto target_module = gum_process_find_module_by_name(name.c_str());
    if (target_module == nullptr) {
        LOGE("target module not found:%s", name.c_str());
        return false;
    }
    gum_module_ensure_initialized(target_module);
    auto target_range = gum_module_get_range(target_module);
    this->module_range.base = target_range->base_address;
    this->module_range.end = target_range->base_address + target_range->size;
    if (offset > target_range->size) {
        LOGE("offset out of range:%s", name.c_str());
        g_object_unref(target_module);
        return false;
    }
    this->target_trace_address = target_range->base_address + offset;
    g_object_unref(target_module);
    this->info_manager = std::make_unique<InstructionInfoManager>(name, module_range, vm);
    return true;
}

bool InstructionTracerManager::init(uintptr_t address) {
    this->target_trace_address = address;
    alloc_fix_stack();
    vm->clearAllCache();
    auto target_module = gum_process_find_module_by_address(address);
    if (target_module == nullptr) {
        LOGE("target module not found");
        return false;
    }
    gum_module_ensure_initialized(target_module);
    auto target_range = gum_module_get_range(target_module);
    this->module_range.base = target_range->base_address;
    this->module_range.end = target_range->base_address + target_range->size;
    this->module_name = gum_module_get_name(target_module);
    auto symbol_name_ = gum_symbol_name_from_address((void *) address);
    if (symbol_name_ != nullptr) {
        this->symbol_name = symbol_name_;
        g_free(symbol_name_);
    }
    g_object_unref(target_module);
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
    if (!result) {
        LOGE("run fail");
        return -1;
    }
    return ret_value;
}

bool InstructionTracerManager::run_attach() {
    gum_interceptor_begin_transaction(this->interceptor);
    auto ret = gum_interceptor_replace(this->interceptor, (void *) this->target_trace_address,
                                       (void *) &frida_on_enter, (void *) this,
                                       nullptr);

    gum_interceptor_end_transaction(this->interceptor);
    return ret == GUM_REPLACE_OK;
}

InstructionTracerManager::~InstructionTracerManager() {
    if (this->vm != nullptr) {
        this->vm->clearAllCache();
        delete this->vm;
    }
    g_object_unref(this->interceptor);
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


void __attribute__((constructor)) init_proc() {
    gum_init_embedded();
}

void __attribute__((destructor)) fini_proc() {
    gum_deinit_embedded();
}
