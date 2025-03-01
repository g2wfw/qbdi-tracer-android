
#ifndef ITRACE_NATIVE_ITRACE_CALL_BACK_H
#define ITRACE_NATIVE_ITRACE_CALL_BACK_H

#include <QBDI.h>

QBDI::VMAction pre_instruction_call(QBDI::VM *vm, QBDI::GPRState *gprState,
                                    QBDI::FPRState *fprState, void *data);

QBDI::VMAction post_instruction_call(QBDI::VM *vm, QBDI::GPRState *gprState,
                                     QBDI::FPRState *fprState, void *data);

QBDI::VMAction pre_svc_instruction_call(QBDI::VM *vm, QBDI::GPRState *gprState,
                                        QBDI::FPRState *fprState, void *data);

QBDI::VMAction
on_fun_call(QBDI::VMInstanceRef vm, const QBDI::VMState *state, QBDI::GPRState *gprState,
            QBDI::FPRState *fprState, void *data);

QBDI::VMAction
on_memory_read_or_write(QBDI::VMInstanceRef vm, QBDI::GPRState *gprState,
                        QBDI::FPRState *fprState, void *data);

void frida_on_enter();

#endif //ITRACE_NATIVE_ITRACE_CALL_BACK_H
