/* libunwind - a platform-independent unwind library
   Copyright (C) 2001-2005 Hewlett-Packard Co
   Copyright (C) 2007 David Mosberger-Tang
	Contributed by David Mosberger-Tang <dmosberger@gmail.com>

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

/* This files contains libunwind-internal definitions which are
   subject to frequent change and are not to be exposed to
   libunwind-users.  */

#ifndef libunwind_i_h
#define libunwind_i_h

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "compiler.h"

#ifdef HAVE___THREAD
  /* For now, turn off per-thread caching.  It uses up too much TLS
     memory per thread even when the thread never uses libunwind at
     all.  */
# undef HAVE___THREAD
#endif

/* Platform-independent libunwind-internal declarations.  */

#include <sys/types.h>	/* HP-UX needs this before include of pthread.h */

#include <assert.h>
#include <libunwind.h>
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>

#if defined(HAVE_ELF_H)
# include <elf.h>
#elif defined(HAVE_SYS_ELF_H)
# include <sys/elf.h>
#else
# error Could not locate <elf.h>
#endif

#if defined(HAVE_ENDIAN_H)
# include <endian.h>
#elif defined(HAVE_SYS_ENDIAN_H)
# include <sys/endian.h>
#else
# define __LITTLE_ENDIAN	1234
# define __BIG_ENDIAN		4321
# if defined(__hpux)
#   define __BYTE_ORDER __BIG_ENDIAN
# elif defined(__QNX__)
#   if defined(__BIGENDIAN__)
#     define __BYTE_ORDER __BIG_ENDIAN
#   elif defined(__LITTLEENDIAN__)
#     define __BYTE_ORDER __LITTLE_ENDIAN
#   else
#     error Host has unknown byte-order.
#   endif
# else
#   error Host has unknown byte-order.
# endif
#endif

#if defined(HAVE__BUILTIN_UNREACHABLE)
# define unreachable() __builtin_unreachable()
#else
# define unreachable() do { } while (1)
#endif

#ifdef DEBUG
# define UNW_DEBUG	1
#else
# define UNW_DEBUG	0
#endif

/* Make it easy to write thread-safe code which may or may not be
   linked against libpthread.  The macros below can be used
   unconditionally and if -lpthread is around, they'll call the
   corresponding routines otherwise, they do nothing.  */

#pragma weak pthread_mutex_init
#pragma weak pthread_mutex_lock
#pragma weak pthread_mutex_unlock

#define mutex_init(l)							\
	(pthread_mutex_init != NULL ? pthread_mutex_init ((l), NULL) : 0)
#define mutex_lock(l)							\
	(pthread_mutex_lock != NULL ? pthread_mutex_lock (l) : 0)
#define mutex_unlock(l)							\
	(pthread_mutex_unlock != NULL ? pthread_mutex_unlock (l) : 0)

#ifdef HAVE_ATOMIC_OPS_H
# include <atomic_ops.h>
static inline int
cmpxchg_ptr (void *addr, void *old, void *new)
{
  union
    {
      void *vp;
      AO_t *aop;
    }
  u;

  u.vp = addr;
  return AO_compare_and_swap(u.aop, (AO_t) old, (AO_t) new);
}
# define fetch_and_add1(_ptr)		AO_fetch_and_add1(_ptr)
# define fetch_and_add(_ptr, value)	AO_fetch_and_add(_ptr, value)
   /* GCC 3.2.0 on HP-UX crashes on cmpxchg_ptr() */
#  if !(defined(__hpux) && __GNUC__ == 3 && __GNUC_MINOR__ == 2)
#   define HAVE_CMPXCHG
#  endif
# define HAVE_FETCH_AND_ADD
#elif defined(HAVE_SYNC_ATOMICS) || defined(HAVE_IA64INTRIN_H)
# ifdef HAVE_IA64INTRIN_H
#  include <ia64intrin.h>
# endif
static inline int
cmpxchg_ptr (void *addr, void *old, void *new)
{
  union
    {
      void *vp;
      long *vlp;
    }
  u;

  u.vp = addr;
  return __sync_bool_compare_and_swap(u.vlp, (long) old, (long) new);
}
# define fetch_and_add1(_ptr)		__sync_fetch_and_add(_ptr, 1)
# define fetch_and_add(_ptr, value)	__sync_fetch_and_add(_ptr, value)
# define HAVE_CMPXCHG
# define HAVE_FETCH_AND_ADD
#endif
#define atomic_read(ptr)	(*(ptr))

#define UNWI_OBJ(fn)	  UNW_PASTE(UNW_PREFIX,UNW_PASTE(I,fn))
#define UNWI_ARCH_OBJ(fn) UNW_PASTE(UNW_PASTE(UNW_PASTE(_UI,UNW_TARGET),_), fn)

#define unwi_full_mask    UNWI_ARCH_OBJ(full_mask)

/* Type of a mask that can be used to inhibit preemption.  At the
   userlevel, preemption is caused by signals and hence sigset_t is
   appropriate.  In constrast, the Linux kernel uses "unsigned long"
   to hold the processor "flags" instead.  */
typedef sigset_t intrmask_t;

extern intrmask_t unwi_full_mask;

/* Silence compiler warnings about variables which are used only if libunwind
   is configured in a certain way */
static inline void mark_as_used(void *v UNUSED) {
}

#if defined(CONFIG_BLOCK_SIGNALS)
# define SIGPROCMASK(how, new_mask, old_mask) \
  sigprocmask((how), (new_mask), (old_mask))
#else
# define SIGPROCMASK(how, new_mask, old_mask) mark_as_used(old_mask)
#endif

/* ANDROID support update. */
#define __lock_acquire_internal(l, m, acquire_func)	\
do {							\
  SIGPROCMASK (SIG_SETMASK, &unwi_full_mask, &(m));	\
  acquire_func (l);					\
} while (0)
#define __lock_release_internal(l, m, release_func)	\
do {							\
  release_func (l);					\
  SIGPROCMASK (SIG_SETMASK, &(m), NULL);		\
} while (0)

#define lock_rdwr_var(name)				\
  pthread_rwlock_t name
#define lock_rdwr_init(l)	pthread_rwlock_init (l, NULL)
#define lock_rdwr_wr_acquire(l, m)			\
  __lock_acquire_internal(l, m, pthread_rwlock_wrlock)
#define lock_rdwr_rd_acquire(l, m)			\
  __lock_acquire_internal(l, m, pthread_rwlock_rdlock)
#define lock_rdwr_release(l, m)				\
  __lock_release_internal(l, m, pthread_rwlock_unlock)

#define lock_var(name) \
  pthread_mutex_t name
#define define_lock(name) \
  lock_var (name) = PTHREAD_MUTEX_INITIALIZER
#define lock_init(l)		mutex_init (l)
#define lock_acquire(l,m)				\
  __lock_acquire_internal(l, m, mutex_lock)
#define lock_release(l,m)			\
  __lock_release_internal(l, m, mutex_unlock)
/* End of ANDROID update. */

#define SOS_MEMORY_SIZE 16384	/* see src/mi/mempool.c */

#ifndef MAP_ANONYMOUS
# define MAP_ANONYMOUS MAP_ANON
#endif
#define GET_MEMORY(mem, size)				    		    \
do {									    \
  /* Hopefully, mmap() goes straight through to a system call stub...  */   \
  mem = mmap (NULL, size, PROT_READ | PROT_WRITE,			    \
	      MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);			    \
  if (mem == MAP_FAILED)						    \
    mem = NULL;								    \
} while (0)

#define unwi_find_dynamic_proc_info	UNWI_OBJ(find_dynamic_proc_info)
#define unwi_extract_dynamic_proc_info	UNWI_OBJ(extract_dynamic_proc_info)
#define unwi_put_dynamic_unwind_info	UNWI_OBJ(put_dynamic_unwind_info)
#define unwi_dyn_remote_find_proc_info	UNWI_OBJ(dyn_remote_find_proc_info)
#define unwi_dyn_remote_put_unwind_info	UNWI_OBJ(dyn_remote_put_unwind_info)
#define unwi_dyn_validate_cache		UNWI_OBJ(dyn_validate_cache)

extern int unwi_find_dynamic_proc_info (unw_addr_space_t as,
					unw_word_t ip,
					unw_proc_info_t *pi,
					int need_unwind_info, void *arg);
extern int unwi_extract_dynamic_proc_info (unw_addr_space_t as,
					   unw_word_t ip,
					   unw_proc_info_t *pi,
					   unw_dyn_info_t *di,
					   int need_unwind_info,
					   void *arg);
extern void unwi_put_dynamic_unwind_info (unw_addr_space_t as,
					  unw_proc_info_t *pi, void *arg);

/* These handle the remote (cross-address-space) case of accessing
   dynamic unwind info. */

extern int unwi_dyn_remote_find_proc_info (unw_addr_space_t as,
					   unw_word_t ip,
					   unw_proc_info_t *pi,
					   int need_unwind_info,
					   void *arg);
extern void unwi_dyn_remote_put_unwind_info (unw_addr_space_t as,
					     unw_proc_info_t *pi,
					     void *arg);
extern int unwi_dyn_validate_cache (unw_addr_space_t as, void *arg);

extern unw_dyn_info_list_t _U_dyn_info_list;
extern pthread_mutex_t _U_dyn_info_list_lock;

#if UNW_DEBUG
# define unwi_debug_level		UNWI_ARCH_OBJ(debug_level)
extern long unwi_debug_level;

# ifdef ANDROID
# define LOG_TAG "libunwind"
#include <android/log.h>
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, __FUNCTION__, __VA_ARGS__)

# define Debug(level, format, ...)					\
do {									\
  if (unwi_debug_level >= (level))					\
    { 									\
      LOGI("%*c>%s: " format, ((level) <= 16) ? (level) : 16, ' ',	\
            __FUNCTION__, ##__VA_ARGS__);				\
    } 									\
} while (0)
# define Dprintf(format, ...) LOGI(format, ##__VA_ARGS__);
#else
# include <stdio.h>
# define Debug(level,format...)						\
do {									\
  if (unwi_debug_level >= level)					\
    {									\
      int _n = level;							\
      if (_n > 16)							\
	_n = 16;							\
      fprintf (stderr, "%*c>%s: ", _n, ' ', __FUNCTION__);		\
      fprintf (stderr, format);						\
    }									\
} while (0)
# define Dprintf(format...) 	    fprintf (stderr, format)
# ifdef __GNUC__
#  undef inline
#  define inline	UNUSED
# endif
# endif
#else
# define Debug(level,format...)
# define Dprintf(format...)
#endif

static ALWAYS_INLINE int
print_error (const char *string)
{
  return write (2, string, strlen (string));
}

#define mi_init		UNWI_ARCH_OBJ(mi_init)

extern void mi_init (void);	/* machine-independent initializations */
extern unw_word_t _U_dyn_info_list_addr (void);

/* This is needed/used by ELF targets only.  */

struct elf_image
  {
    void *image;		/* pointer to mmap'd image */
    size_t size;		/* (file-) size of the image */
  };

struct elf_dyn_info
  {
    /* ANDROID support update.*/
    /* Removed: struct elf_image ei; */
    /* End of ANDROID update. */
    unw_dyn_info_t di_cache;
    unw_dyn_info_t di_debug;    /* additional table info for .debug_frame */
#if UNW_TARGET_IA64
    unw_dyn_info_t ktab;
#endif
#if UNW_TARGET_ARM
    unw_dyn_info_t di_arm;      /* additional table info for .ARM.exidx */
#endif
  };

static inline void invalidate_edi (struct elf_dyn_info *edi)
{
  /* ANDROID support update.*/
  /* Removed: if (edi->ei.image) */
  /*            munmap (edi->ei.image, edi->ei.size); */
  /* End of ANDROID update. */
  memset (edi, 0, sizeof (*edi));
  edi->di_cache.format = -1;
  edi->di_debug.format = -1;
#if UNW_TARGET_ARM
  edi->di_arm.format = -1;
#endif
}


/* Provide a place holder for architecture to override for fast access
   to memory when known not to need to validate and know the access
   will be local to the process. A suitable override will improve
   unw_tdep_trace() performance in particular. */
#define ACCESS_MEM_FAST(ret,validate,cur,addr,to) \
  do { (ret) = dwarf_get ((cur), DWARF_MEM_LOC ((cur), (addr)), &(to)); } \
  while (0)

/* Define GNU and processor specific values for the Phdr p_type field in case
   they aren't defined by <elf.h>.  */
#ifndef PT_GNU_EH_FRAME
# define PT_GNU_EH_FRAME	0x6474e550
#endif /* !PT_GNU_EH_FRAME */
#ifndef PT_ARM_EXIDX
# define PT_ARM_EXIDX		0x70000001	/* ARM unwind segment */
#endif /* !PT_ARM_EXIDX */

#include "tdep/libunwind_i.h"

#ifndef tdep_get_func_addr
# define tdep_get_func_addr(as,addr,v)		(*(v) = addr, 0)
#endif

#define UNW_ALIGN(x,a) (((x)+(a)-1UL)&~((a)-1UL))

#endif /* libunwind_i_h */
