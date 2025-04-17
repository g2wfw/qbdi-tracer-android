//
//
//

#ifndef ITRACE_NATIVE_DISPATCH_BASE_H
#define ITRACE_NATIVE_DISPATCH_BASE_H

#include <QBDI.h>

#include <cstdint>
#include <string>
#include <unordered_map>
#include "../common.h"

void dispatch_export_func(const char *symbol, uintptr_t addr, void *user_data);


class DispatchBase {
public:
    virtual ~DispatchBase() = default;

    [[nodiscard]] bool is_module_address(uintptr_t address) const;

    void add_export_info(const char *symbol, uintptr_t addr);

    std::string get_address_symbol(uintptr_t address);

    virtual bool dispatch_args(inst_trace_info_t *info) = 0;

    virtual bool dispatch_ret(inst_trace_info_t *info, const QBDI::GPRState *ret_status) = 0;
    static uintptr_t get_arg_register_value(trace_vm_status_t *instCall, uint32_t arg_index);
protected:
    std::unordered_map<uintptr_t, std::string> symbol_info;
    module_range_t module_range = {};

protected:
    /**
      * Get the value of an argument register.
      *
      * This function retrieves the value of a specific argument register during the execution of a traced VM call.
      * It is useful for inspecting or modifying function arguments at runtime.
      *
      * @param instCall Pointer to the trace VM status, representing the state and context of the traced VM call.
      * @param arg_index The index of the argument register whose value needs to be retrieved.
      * @return The value of the specified argument register, returned as a uintptr_t which can hold any pointer type.
      */


    static uintptr_t get_ret_register_value(const QBDI::GPRState *state, uint32_t arg_index);

    static const char *read_string_from_address(uintptr_t address);

    static std::string read_buffer_hexdump_from_address(uintptr_t address, size_t size = 16);

    static bool add_common_reg_values(inst_trace_info_t *info);

    static bool add_common_return_value(inst_trace_info_t *info, const QBDI::GPRState *state);
};

#endif  //ITRACE_NATIVE_DISPATCH_BASE_H
