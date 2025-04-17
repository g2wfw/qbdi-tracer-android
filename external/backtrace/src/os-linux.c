/* libunwind - a platform-independent unwind library
   Copyright (C) 2003-2005 Hewlett-Packard Co
	Contributed by David Mosberger-Tang <davidm@hpl.hp.com>

This file is part of libunwind.

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.  */

#include <limits.h>
#include <stdio.h>

#include "libunwind_i.h"
#include "map_info.h"
#include "os-linux.h"

/* ANDROID support update. */
HIDDEN struct map_info *
map_create_list (pid_t pid)
{
  struct map_iterator mi;
  unsigned long start, end, offset, flags;
  struct map_info *map_list = NULL;
  struct map_info *cur_map;

  if (maps_init (&mi, pid) < 0)
    return NULL;

  while (maps_next (&mi, &start, &end, &offset, &flags))
    {
      cur_map = map_alloc_info ();
      if (cur_map == MAP_FAILED)
        break;
      cur_map->next = map_list;
      cur_map->start = start;
      cur_map->end = end;
      cur_map->offset = offset;
      cur_map->flags = flags;
      cur_map->path = strdup (mi.path);
      mutex_init (&cur_map->ei_lock);
      cur_map->ei.size = 0;
      cur_map->ei.image = NULL;

      /* Indicate mapped memory of devices is special and should not
         be read or written. Use a special flag instead of zeroing the
         flags to indicate that the maps do not need to be rebuilt if
         any values ever wind up in these special maps.
         /dev/ashmem/... maps are special and don't have any restrictions,
         so don't mark them as device memory.  */
      if (strncmp ("/dev/", cur_map->path, 5) == 0
          && strncmp ("ashmem/", cur_map->path + 5, 7) != 0)
        cur_map->flags |= MAP_FLAGS_DEVICE_MEM;

      map_list = cur_map;
    }

  maps_close (&mi);

  return map_list;
}
/* End of ANDROID update. */
