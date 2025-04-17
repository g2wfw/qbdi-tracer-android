//
// Created by xiaob on 2023/7/28.
//

#include "debugger.h"
#include <sys/types.h>
#include "backtrace.h"
#include "UniquePtr.h"
#include "backtrace/backtrace_constants.h"

#include "./../include/backtrace/Backtrace.h"
#include <android/log.h>
#include <unistd.h>

#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, __FUNCTION__, __VA_ARGS__)

int dump_attached_thread(pid_t pid) {
    int total_sleep_time_usec;
    dump_backtrace(pid, getpid(), false, &total_sleep_time_usec);
    return 0;
}

void dump_thread(pid_t pid) {
    LOGW("dump thread for %d", pid);
    UniquePtr<Backtrace> backtrace(Backtrace::CreateTarget(pid, BACKTRACE_CURRENT_THREAD));
    if (backtrace->Unwind(0)) {
        dump_backtrace_to_log(backtrace.get(), "  ");
    }
}
