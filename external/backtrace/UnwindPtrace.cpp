/*
 * Copyright (C) 2013 The Android Open Source Project
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

#include <backtrace/Backtrace.h>
#include <backtrace/BacktraceMap.h>

#include <sys/types.h>
#include <string.h>
#include <ucontext.h>

#include <libunwind.h>
#include <libunwind-ptrace.h>

#include "BacktraceLog.h"
#include "UnwindMap.h"
#include "UnwindPtrace.h"
#include <android/log.h>
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, __FUNCTION__, __VA_ARGS__)
UnwindPtrace::UnwindPtrace() : addr_space_(NULL), upt_info_(NULL) {
}

UnwindPtrace::~UnwindPtrace() {
  if (upt_info_) {
    _UPT_destroy(upt_info_);
    upt_info_ = NULL;
  }
  if (addr_space_) {
    // Remove the map from the address space before destroying it.
    // It will be freed in the UnwindMap destructor.
    unw_map_set(addr_space_, NULL);

    unw_destroy_addr_space(addr_space_);
    addr_space_ = NULL;
  }
}

bool UnwindPtrace::Unwind(size_t num_ignore_frames, ucontext_t* ucontext) {
  if (ucontext) {
    LOGW("Unwinding from a specified context not supported yet.");
    return false;
  }

  addr_space_ = unw_create_addr_space(&_UPT_accessors, 0);
  if (!addr_space_) {
    LOGW("unw_create_addr_space failed.");
    return false;
  }

  UnwindMap* map = static_cast<UnwindMap*>(GetMap());
  unw_map_set(addr_space_, map->GetMapCursor());

  upt_info_ = reinterpret_cast<struct UPT_info*>(_UPT_create(Tid()));
  if (!upt_info_) {
    LOGW("Failed to create upt info.");
    return false;
  }

  unw_cursor_t cursor;
  int ret = unw_init_remote(&cursor, addr_space_, upt_info_);

  if (ret < 0) {
    LOGW("unw_init_remote failed %d", ret);
    return false;
  }

  std::vector<backtrace_frame_data_t>* frames = GetFrames();
  frames->reserve(MAX_BACKTRACE_FRAMES);
  size_t num_frames = 0;
  do {
    unw_word_t pc;
    ret = unw_get_reg(&cursor, UNW_REG_IP, &pc);
    if (ret < 0) {
      LOGW("Failed to read IP %d", ret);
      break;
    }
    unw_word_t sp;
    ret = unw_get_reg(&cursor, UNW_REG_SP, &sp);
    if (ret < 0) {
      LOGW("Failed to read SP %d", ret);
      break;
    }

    if (num_ignore_frames == 0) {
      frames->resize(num_frames+1);
      backtrace_frame_data_t* frame = &frames->at(num_frames);
      frame->num = num_frames;
      frame->pc = static_cast<uintptr_t>(pc);
      frame->sp = static_cast<uintptr_t>(sp);
      frame->stack_size = 0;

      if (num_frames > 0) {
        backtrace_frame_data_t* prev = &frames->at(num_frames-1);
        prev->stack_size = frame->sp - prev->sp;
      }

      frame->func_name = GetFunctionName(frame->pc, &frame->func_offset);

      frame->map =  (backtrace_map_t*)FindMap(frame->pc);

      num_frames++;
    } else {
      num_ignore_frames--;
    }
    ret = unw_step (&cursor);
  } while (ret > 0 && num_frames < MAX_BACKTRACE_FRAMES);

  return true;
}

std::string UnwindPtrace::GetFunctionNameRaw(uintptr_t pc, uintptr_t* offset) {
  *offset = 0;
  char buf[512];
  unw_word_t value;
  if (unw_get_proc_name_by_ip(addr_space_, pc, buf, sizeof(buf), &value,
                              upt_info_) >= 0 && buf[0] != '\0') {
    *offset = static_cast<uintptr_t>(value);
    return buf;
  }
  return "";
}

//-------------------------------------------------------------------------
// C++ object creation function.
//-------------------------------------------------------------------------
Backtrace* CreatePtraceObj(pid_t pid, pid_t tid, BacktraceMap* map) {
  return new BacktracePtrace(new UnwindPtrace(), pid, tid, map);
}
