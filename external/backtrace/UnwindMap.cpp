/*
 * Copyright (C) 2014 The Android Open Source Project
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

#include <pthread.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#include <backtrace/BacktraceMap.h>

#include <libunwind.h>

#include "BacktraceLog.h"
#include "UnwindMap.h"
#include <android/log.h>
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, __FUNCTION__, __VA_ARGS__)
//-------------------------------------------------------------------------
// libunwind has a single shared address space for the current process
// aka local. If multiple maps are created for the current pid, then
// only update the local address space once, and keep a reference count
// of maps using the same map cursor.
//-------------------------------------------------------------------------
UnwindMap::UnwindMap(pid_t pid) : BacktraceMap(pid) {
}

UnwindMap::~UnwindMap() {
  unw_map_cursor_destroy(&map_cursor_);
  unw_map_cursor_clear(&map_cursor_);
}

bool UnwindMap::GenerateMap() {
  // Use the map_cursor information to construct the BacktraceMap data
  // rather than reparsing /proc/self/maps.
  unw_map_cursor_reset(&map_cursor_);

  unw_map_t unw_map;
  while (unw_map_cursor_get_next(&map_cursor_, &unw_map)) {
    backtrace_map_t map;

    map.start = unw_map.start;
    map.end = unw_map.end;
    map.flags = unw_map.flags;
    map.name = unw_map.path;

    // The maps are in descending order, but we want them in ascending order.
    maps_.push_front(map);
  }

  return true;
}

bool UnwindMap::Build() {
  return (unw_map_cursor_create(&map_cursor_, pid_) == 0) && GenerateMap();
}

UnwindMapLocal::UnwindMapLocal() : UnwindMap(getpid()), map_created_(false) {
}

UnwindMapLocal::~UnwindMapLocal() {
  if (map_created_) {
    unw_map_local_destroy();
    unw_map_cursor_clear(&map_cursor_);
  }
}

bool UnwindMapLocal::GenerateMap() {
  // It's possible for the map to be regenerated while this loop is occurring.
  // If that happens, get the map again, but only try at most three times
  // before giving up.
  for (int i = 0; i < 3; i++) {
    maps_.clear();

    unw_map_local_cursor_get(&map_cursor_);

    unw_map_t unw_map;
    int ret;
    while ((ret = unw_map_local_cursor_get_next(&map_cursor_, &unw_map)) > 0) {
      backtrace_map_t map;

      map.start = unw_map.start;
      map.end = unw_map.end;
      map.flags = unw_map.flags;
      map.name = unw_map.path;

      free(unw_map.path);

      // The maps are in descending order, but we want them in ascending order.
      maps_.push_front(map);
    }
    // Check to see if the map changed while getting the data.
    if (ret != -UNW_EINVAL) {
      return true;
    }
  }

  LOGW("Unable to generate the map.");
  return false;
}

bool UnwindMapLocal::Build() {
  return (map_created_ = (unw_map_local_create() == 0)) && GenerateMap();;
}

const backtrace_map_t* UnwindMapLocal::Find(uintptr_t addr) {
  const backtrace_map_t* map = BacktraceMap::Find(addr);
  if (!map) {
    // Check to see if the underlying map changed and regenerate the map
    // if it did.
    if (unw_map_local_cursor_valid(&map_cursor_) < 0) {
      if (GenerateMap()) {
        map = BacktraceMap::Find(addr);
      }
    }
  }
  return map;
}

//-------------------------------------------------------------------------
// BacktraceMap create function.
//-------------------------------------------------------------------------
BacktraceMap* BacktraceMap::Create(pid_t pid, bool uncached) {
  BacktraceMap* map;

  if (uncached) {
    // Force use of the base class to parse the maps when this call is made.
    map = new BacktraceMap(pid);
  } else if (pid == getpid()) {
    map = new UnwindMapLocal();
  } else {
    map = new UnwindMap(pid);
  }
  if (!map->Build()) {
    delete map;
    return NULL;
  }
  return map;
}
