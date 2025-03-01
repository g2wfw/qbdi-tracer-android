//
//
//

#ifndef ITRACE_NATIVE_DISPATCH_LIBC_H
#define ITRACE_NATIVE_DISPATCH_LIBC_H

#include "dispatch_base.h"

class DispatchLibc : public DispatchBase {
public:
    static DispatchLibc *get_instance();

    ~DispatchLibc()=default;

    bool dispatch_args(inst_trace_info_t *info) override;

    bool dispatch_ret(inst_trace_info_t *info, const QBDI::GPRState *ret_status) override;

private:
    DispatchLibc();
};


#endif //ITRACE_NATIVE_DISPATCH_LIBC_H
