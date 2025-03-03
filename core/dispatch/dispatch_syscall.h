//
//
//

#ifndef MEITUAN_DISPATCH_SYSCALL_H
#define MEITUAN_DISPATCH_SYSCALL_H

#include "dispatch_base.h"

class DispatchSyscall : public DispatchBase {
public:
    static DispatchSyscall *get_instance();

    ~DispatchSyscall() = default;

    bool dispatch_args(inst_trace_info_t *info) override;

    bool dispatch_ret(inst_trace_info_t *info, const QBDI::GPRState *ret_status) override;

private:
    DispatchSyscall();
    bool record_memory_info(inst_trace_info_t* info, const QBDI::GPRState* ret_status);
};


#endif //MEITUAN_DISPATCH_SYSCALL_H
