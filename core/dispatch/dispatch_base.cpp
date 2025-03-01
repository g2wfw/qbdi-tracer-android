//
//
//
#include "dispatch_base.h"
#include <unordered_map>
#include <spdlog/fmt/fmt.h>

gboolean dispatch_export_func(const GumExportDetails *details,
                              gpointer user_data) {
    auto dispatch_base = (DispatchBase *) user_data;
    if (details != nullptr && dispatch_base != nullptr) {
        dispatch_base->add_export_info(details);
    }
    return true;
}

gboolean dispatch_symbol_func(const GumSymbolDetails *details,
                              gpointer user_data) {
    auto dispatch_base = (DispatchBase *) user_data;
    if (details != nullptr && dispatch_base != nullptr) {
        dispatch_base->add_symbol_info(details);
        //LOGI("0x%lx %s", details->address, details->name);
    }
    return true;
}


void DispatchBase::add_export_info(const GumExportDetails *details) {
    this->symbol_info.emplace(details->address, details->name);
}

void DispatchBase::add_symbol_info(const GumSymbolDetails *details) {
    this->symbol_info.emplace(details->address, details->name);
}

bool DispatchBase::is_module_address(uintptr_t address) const {
    if (address < module_range.base) {
        return false;
    }
    if (address > module_range.end) {
        return false;
    }
    return true;
}

std::string DispatchBase::get_address_symbol(uintptr_t address) {
    auto find = this->symbol_info.find(address);
    if (find == this->symbol_info.end()) {
        return {};
    }
    return find->second;
}


const char *DispatchBase::read_string_from_address(uintptr_t address) {
    if (address == 0) {
        return "";
    }
    return reinterpret_cast<const char *>(address);
}

static std::string hexdump(const void *data, size_t size) {
    auto *byteData = static_cast<const unsigned char *>(data);
    std::string result;

    for (size_t i = 0; i < size; i += 16) {
        result += fmt::format("{}: ", fmt::ptr((byteData + i)));

        for (size_t j = 0; j < 16; ++j) {
            if (i + j < size) {
                result += fmt::format("{:02X} ", byteData[i + j]);
            } else {
                result += "   ";
            }
        }

        result += " |";
        for (size_t j = 0; j < 16; ++j) {
            if (i + j < size) {
                char c = byteData[i + j];
                result += (c >= 32 && c <= 126) ? c : '.';
            }
        }
        result += "| \t";
    }
    return result;
}

std::string DispatchBase::read_buffer_hexdump_from_address(uintptr_t address, size_t size) {
    if (address == 0 || size == 0) {
        return "";
    }
    auto addr = reinterpret_cast<uint8_t *>(address);
    if (gum_memory_is_readable(addr, size)) {
        return hexdump(addr, size);
    }
    auto start = addr;
    auto *end = reinterpret_cast<uint8_t *>(address + size);
    while (gum_memory_is_readable((void *) addr, 1)) {
        if (addr > end) {
            break;
        }
        addr++;
    }
    return hexdump(start, addr - start - 1);
}

bool DispatchBase::add_common_reg_values(inst_trace_info_t *info) {
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

bool DispatchBase::add_common_return_value(inst_trace_info_t *info, const QBDI::GPRState *state) {
    auto instCall = info->fun_call;
#if __arm__
    instCall->ret_type = kUnknown;
    instCall->ret_value = fmt::format("ret={:#x}", state->r0);
#else
    instCall->ret_type = kUnknown;
    instCall->ret_value = fmt::format("ret={:#x}", state->x0);
#endif
    return true;
}


uintptr_t DispatchBase::get_arg_register_value(trace_vm_status_t *vm_status, uint32_t arg_index) {

    auto &status = vm_status->gpr_state;
#ifdef __arm__
    constexpr uint32_t MAX_REG_ARGS = 4;
    constexpr uint32_t MAX_STACK_ARGS = 16;
    if (arg_index >= (MAX_REG_ARGS + MAX_STACK_ARGS)) {
        LOGE("Exceed max supported arguments: %u", arg_index);
        return 0;
    }
    if (arg_index < MAX_REG_ARGS) {
        return QBDI_GPR_GET(&status, arg_index); // R0=0, R1=1, etc.
    }
    auto sp = QBDI_GPR_GET(&status, QBDI::REG_SP);
    uintptr_t *stack_args = reinterpret_cast<uintptr_t *>(sp);
    uint32_t stack_offset = (arg_index - MAX_REG_ARGS) * sizeof(uintptr_t);

    stack_offset += (stack_offset % 8 != 0) ? (8 - (stack_offset % 8)) : 0;
    return stack_args[stack_offset / sizeof(uintptr_t)];
#elif defined(__aarch64__)
    constexpr uint32_t MAX_REG_ARGS = 8;
    constexpr uint32_t MAX_STACK_ARGS = 16;
    if (arg_index >= (MAX_REG_ARGS + MAX_STACK_ARGS)) {
        LOGE("Exceed max supported arguments: %u", arg_index);
        return 0;
    }
    if (arg_index < MAX_REG_ARGS) {
        return QBDI_GPR_GET(&status, arg_index); // X0-X7
    }
    auto sp = QBDI_GPR_GET(&status, QBDI::REG_SP);
    uintptr_t *stack_base = reinterpret_cast<uintptr_t *>(sp);
    const uint32_t stack_offset = (arg_index - MAX_REG_ARGS) * sizeof(uintptr_t);
    if (stack_offset >= (MAX_STACK_ARGS * sizeof(uintptr_t))) {
        LOGE("Stack argument %u exceeds max offset", arg_index);
        return 0;
    }
    return stack_base[stack_offset / sizeof(uintptr_t)];
#endif
    return 0;
}

uintptr_t DispatchBase::get_ret_register_value(const QBDI::GPRState *state, uint32_t arg_index) {
    auto &status = state;
#ifdef __arm__
    // ARM ABI 规定前 4 个参数通过 R0-R3 寄存器传递
    constexpr uint32_t MAX_REG_ARGS = 4;
    constexpr uint32_t MAX_STACK_ARGS = 16;
    if (arg_index >= (MAX_REG_ARGS + MAX_STACK_ARGS)) {
        LOGE("Exceed max supported arguments: %u", arg_index);
        return 0;
    }
    if (arg_index < MAX_REG_ARGS) {
        return QBDI_GPR_GET(status, arg_index); // R0=0, R1=1, etc.
    }
    auto sp = QBDI_GPR_GET(status, QBDI::REG_SP);
    uintptr_t *stack_args = reinterpret_cast<uintptr_t *>(sp);
    uint32_t stack_offset = (arg_index - MAX_REG_ARGS) * sizeof(uintptr_t);
    // 添加 AAPCS 要求的栈对齐偏移（8字节对齐）
    stack_offset += (stack_offset % 8 != 0) ? (8 - (stack_offset % 8)) : 0;
    return stack_args[stack_offset / sizeof(uintptr_t)];


#elif defined(__aarch64__)
    constexpr uint32_t MAX_REG_ARGS = 8;
    constexpr uint32_t MAX_STACK_ARGS = 16;
    if (arg_index >= (MAX_REG_ARGS + MAX_STACK_ARGS)) {
        LOGE("Exceed max supported arguments: %u", arg_index);
        return 0;
    }
    if (arg_index < MAX_REG_ARGS) {
        return QBDI_GPR_GET(status, arg_index); // X0-X7
    }
    auto sp = QBDI_GPR_GET(status, QBDI::REG_SP);
    uintptr_t *stack_base = reinterpret_cast<uintptr_t *>(sp);
    const uint32_t stack_offset = (arg_index - MAX_REG_ARGS) * sizeof(uintptr_t);
    if (stack_offset >= (MAX_STACK_ARGS * sizeof(uintptr_t))) {
        LOGE("Stack argument %u exceeds max offset", arg_index);
        return 0;
    }
    return stack_base[stack_offset / sizeof(uintptr_t)];
#endif
    return 0;
}







