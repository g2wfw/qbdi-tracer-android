//
// Created by xiaob on 2023/7/28.
//

#ifndef ANDROID_ENV_DETECT_DEBUGGER_H
#define ANDROID_ENV_DETECT_DEBUGGER_H

#include <sys/types.h>

int dump_attached_thread(pid_t pid);

void dump_thread(pid_t pid);

#endif