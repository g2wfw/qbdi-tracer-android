/* system/debuggerd/utility.h
**
** Copyright 2008, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

#ifndef _DEBUGGERD_UTILITY_H
#define _DEBUGGERD_UTILITY_H

#include <stdbool.h>
#include <sys/types.h>

// Figure out the abi based on defined macros.
#if defined(__arm__)
#define ABI_STRING "arm"
#elif defined(__aarch64__)
#define ABI_STRING "arm64"
#elif defined(__mips__)
#define ABI_STRING "mips"
#elif defined(__i386__)
#define ABI_STRING "x86"
#elif defined(__x86_64__)
#define ABI_STRING "x86_64"
#else
#error "Unsupported ABI"
#endif


struct log_t{
    /* tombstone file descriptor */
    int tfd;
    /* Activity Manager socket file descriptor */
    int amfd;
    // The tid of the thread that crashed.
    pid_t crashed_tid;
    // The tid of the thread we are currently working with.
    pid_t current_tid;
    // logd daemon crash, can block asking for logcat data, allow suppression.
    bool should_retrieve_logcat;

    log_t()
        : tfd(-1), amfd(-1), crashed_tid(-1), current_tid(-1), should_retrieve_logcat(true) {}
};

// List of types of logs to simplify the logging decision in _LOG
enum logtype {
  ERROR,
  HEADER,
  THREAD,
  REGISTERS,
  FP_REGISTERS,
  BACKTRACE,
  MAPS,
  MEMORY,
  STACK,
  LOGS
};

// Log information onto the tombstone.
void _LOG(logtype ltype, const char *fmt, ...)
        __attribute__ ((format(printf, 2, 3)));

int wait_for_sigstop(pid_t, int*);

void dump_memory( pid_t tid, uintptr_t addr);

#endif // _DEBUGGERD_UTILITY_H
