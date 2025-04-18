/*
 * Copyright (C) 2012 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _DEBUGGERD_BACKTRACE_H
#define _DEBUGGERD_BACKTRACE_H

#include <sys/types.h>

#include "utility.h"

class Backtrace;

// Dumps a backtrace using a format similar to what Dalvik uses so that the result
// can be intermixed in a bug report.
void dump_backtrace( pid_t pid, pid_t tid, bool need_attach,
                    int* total_sleep_time_usec);

/* Dumps the backtrace in the backtrace data structure to the log. */
void dump_backtrace_to_log(Backtrace* backtrace, const char* prefix);

#endif // _DEBUGGERD_BACKTRACE_H
