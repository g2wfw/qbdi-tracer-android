//
//
//

#ifndef ITRACE_NATIVE_DISPATCH_LIBC_H
#define ITRACE_NATIVE_DISPATCH_LIBC_H

#include "dispatch_base.h"

class DispatchLibc final : public DispatchBase {
public:
    static DispatchLibc *get_instance();

    ~DispatchLibc() override = default;

    bool dispatch_args(inst_trace_info_t *info) override;

    bool dispatch_ret(inst_trace_info_t *info, const QBDI::GPRState *ret_status) override;

private:
    bool get_format_result(inst_trace_info_t *info, const QBDI::GPRState *ret_status);

    void record_memory_info(inst_trace_info_t *info, const QBDI::GPRState *ret_status);

    DispatchLibc();
};

#endif  //ITRACE_NATIVE_DISPATCH_LIBC_H
