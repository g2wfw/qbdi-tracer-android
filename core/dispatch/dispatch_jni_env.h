#ifndef ITRACE_NATIVE_DISPATCH_JNI_ENV_H
#define ITRACE_NATIVE_DISPATCH_JNI_ENV_H

#include <jni.h>
#include "dispatch_base.h"

class DispatchJNIEnv final : public DispatchBase {
public:
    static DispatchJNIEnv* get_instance();

    ~DispatchJNIEnv() override = default;

    bool dispatch_args(inst_trace_info_t* info) override;

    bool dispatch_ret(inst_trace_info_t* info, const QBDI::GPRState* ret_status) override;

private:
    DispatchJNIEnv();

    void init_env_fun_table();

private:
    static void dispatch_env(std::string name, inst_trace_info_t* trace_info);

    std::string getJNIType(jclass clazz);
};

#endif  //ITRACE_NATIVE_DISPATCH_JNI_ENV_H
