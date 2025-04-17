#pragma once/**
* Created by xiaobaiyey on 2021/8/26.
* Copyright (c) 2021 xiaobaiyey
* History: 
* Description: 用于定义以及使用各种宏
*/
#pragma once

#include <stddef.h>  // for size_t
#include <stdint.h>
// A macro to disallow the copy constructor and operator= functions
// This must be placed in the private: declarations for a class.

#define DISALLOW_COPY_AND_ASSIGN(TypeName)       \
        TypeName(const TypeName&) = delete;      \
        void operator=(const TypeName&) = delete


// bionic and glibc both have TEMP_FAILURE_RETRY, but eg Mac OS' libc doesn't.
#ifndef TEMP_FAILURE_RETRY
#define TEMP_FAILURE_RETRY(exp)            \
  ({                                       \
    decltype(exp) _rc;                     \
    do {                                   \
      _rc = (exp);                         \
    } while (_rc == -1 && errno == EINTR); \
    _rc;                                   \
  })
#endif



#ifndef LIKELY
# define LIKELY(x) __builtin_expect(!!(x), 1)
#endif
#ifndef UNLIKELY
# define UNLIKELY(x) __builtin_expect(!!(x), 0)
#endif
//for c++ version
#define CPP20 202002L
#define CPP17 201703L
#define CPP14 201402L
#define CPP11 201103L
#define CPP98 199711L

#ifndef INLINE
//# define DEX_INLINE extern __inline__
# define INLINE static __inline__
#else
# define INLINE _inline
#endif


#if defined(_MSC_VER) && !defined(__clang__)
#define ALWAYS_INLINE __forceinline
#define NOINLINE __declspec(noinline)
#define NORETURN __declspec(noreturn)
#define UNUSED
#else
#ifndef ALWAYS_INLINE
#define ALWAYS_INLINE __attribute__((always_inline))
#endif
#define NOINLINE __attribute__((noinline))
#define NORETURN __attribute__((noreturn))
#define UNUSED __attribute__((unused))
#endif

#ifdef _WIN32
#define STL_EXPORT __declspec(dllexport)
#else
#define STL_EXPORT __attribute__((visibility("default")))
#endif


#ifndef MIN
/** Returns the lesser of its two arguments. */
#define MIN(a, b) (((a)<(b))?(a):(b))
#endif
#ifndef MAX
/** Returns the greater of its two arguments. */
#define MAX(a, b) (((a)>(b))?(a):(b))
#endif

#define To_Str(value) #value

#define NO_RETURN [[ noreturn ]]

#ifndef arraysize
template<typename T, size_t N>
char (&ArraySizeHelper(T(&array)[N]))[N];  // NOLINT(readability/casting)
#endif
#define UNREACHABLE  __builtin_unreachable
#define arraysize(array) (sizeof(ArraySizeHelper(array)))



#define DISALLOW_ALLOCATION() \
  public: \
    NO_RETURN ALWAYS_INLINE void operator delete(void*, size_t) { UNREACHABLE(); } \
    ALWAYS_INLINE void* operator new(size_t, void* ptr) noexcept { return ptr; } \
    ALWAYS_INLINE void operator delete(void*, void*) noexcept { } \
  private: \
    void* operator new(size_t) = delete  // NOLINT

#define OFFSETOF_MEMBER(t, f) \
  (reinterpret_cast<uintptr_t>(&reinterpret_cast<t*>(16)->f) - static_cast<uintptr_t>(16u))  // NOLINT

#define DISALLOW_IMPLICIT_CONSTRUCTORS(TypeName) \
  TypeName() = delete;                           \
  DISALLOW_COPY_AND_ASSIGN(TypeName)

#define ALIGNED(x) __attribute__ ((__aligned__(x)))
#define PACKED(x) __attribute__ ((__aligned__(x), __packed__))

#define _MX_BEGIN namespace mx {
#define _MX_END }
#define _MX0 mx::
#define _USING_MX using namespace mx;

static constexpr size_t KB = 1024;
static constexpr size_t MB = KB * KB;
static constexpr size_t GB = KB * KB * KB;

static constexpr size_t kBitsPerByte = 8;
static constexpr size_t kBitsPerByteLog2 = 3;
static constexpr int kBitsPerIntPtrT = sizeof(intptr_t) * kBitsPerByte;


#ifndef FALLTHROUGH_INTENDED
#define FALLTHROUGH_INTENDED \
  do {                       \
  } while (0)
#endif
#define safe_delete(x) { delete x; x = nullptr; }

#ifndef PRINTF_LIKE
#define PRINTF_LIKE(fmtarg, firstvararg) __attribute__((__format__ (__printf__, fmtarg, firstvararg)))
#endif


#ifndef STL_API
#ifdef _WIN32
#define STL_API __declspec(dllexport)
#else
#define STL_API __attribute__((visibility("default")))
#endif
#endif