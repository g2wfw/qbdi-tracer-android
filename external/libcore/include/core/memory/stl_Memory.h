#pragma once/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/
#include <cstring>
namespace stl
{

//==============================================================================
/** Fills a block of memory with zeros. */
inline void zeromem (void* memory, size_t numBytes) noexcept        { memset (memory, 0, numBytes); }

/** Overwrites a structure or object with zeros. */
template <typename Type>
inline void zerostruct (Type& structure) noexcept                   { memset ((void*) &structure, 0, sizeof (structure)); }

/** Delete an object pointer, and sets the pointer to null.

    Remember that it's not good c++ practice to use delete directly - always try to use a std::unique_ptr
    or other automatic lifetime-management system rather than resorting to deleting raw pointers!
*/
template <typename Type>
inline void deleteAndZero (Type& pointer)                           { delete pointer; pointer = nullptr; }

/** A handy function to round up a pointer to the nearest multiple of a given number of bytes.
    alignmentBytes must be a power of two. */
template <typename Type, typename IntegerType>
inline Type* snapPointerToAlignment (Type* basePointer, IntegerType alignmentBytes) noexcept
{
    return (Type*) ((((size_t) basePointer) + (alignmentBytes - 1)) & ~(alignmentBytes - 1));
}

/** A handy function which returns the difference between any two pointers, in bytes.
    The address of the second pointer is subtracted from the first, and the difference in bytes is returned.
*/
template <typename Type1, typename Type2>
inline int getAddressDifference (Type1* pointer1, Type2* pointer2) noexcept  { return (int) (((const char*) pointer1) - (const char*) pointer2); }

/** If a pointer is non-null, this returns a new copy of the object that it points to, or safely returns
    nullptr if the pointer is null.
*/
template <class Type>
inline Type* createCopyIfNotNull (const Type* objectToCopy) { return objectToCopy != nullptr ? new Type (*objectToCopy) : nullptr; }

//==============================================================================
/** A handy function to read un-aligned memory without a performance penalty or bus-error. */
template <typename Type>
inline Type readUnaligned (const void* srcPtr) noexcept
{
    Type value;
    memcpy (&value, srcPtr, sizeof (Type));
    return value;
}

/** A handy function to write un-aligned memory without a performance penalty or bus-error. */
template <typename Type>
inline void writeUnaligned (void* dstPtr, Type value) noexcept
{
    memcpy (dstPtr, &value, sizeof (Type));
}

//==============================================================================
/** Casts a pointer to another type via `void*`, which suppresses the cast-align
    warning which sometimes arises when casting pointers to types with different
    alignment.
    You should only use this when you know for a fact that the input pointer points
    to a region that has suitable alignment for `Type`, e.g. regions returned from
    malloc/calloc that should be suitable for any non-over-aligned type.
*/
template <typename Type>
inline Type unalignedPointerCast (void* ptr) noexcept
{
    static_assert (std::is_pointer_v<Type>);
    return reinterpret_cast<Type> (ptr);
}

/** Casts a pointer to another type via `void*`, which suppresses the cast-align
    warning which sometimes arises when casting pointers to types with different
    alignment.
    You should only use this when you know for a fact that the input pointer points
    to a region that has suitable alignment for `Type`, e.g. regions returned from
    malloc/calloc that should be suitable for any non-over-aligned type.
*/
template <typename Type>
inline Type unalignedPointerCast (const void* ptr) noexcept
{
    static_assert (std::is_pointer_v<Type>);
    return reinterpret_cast<Type> (ptr);
}

/** A handy function which adds a number of bytes to any type of pointer and returns the result.
    This can be useful to avoid casting pointers to a char* and back when you want to move them by
    a specific number of bytes,
*/
template <typename Type, typename IntegerType>
inline Type* addBytesToPointer (Type* basePointer, IntegerType bytes) noexcept
{
    return unalignedPointerCast<Type*> (reinterpret_cast<char*> (basePointer) + bytes);
}

/** A handy function which adds a number of bytes to any type of pointer and returns the result.
    This can be useful to avoid casting pointers to a char* and back when you want to move them by
    a specific number of bytes,
*/
template <typename Type, typename IntegerType>
inline const Type* addBytesToPointer (const Type* basePointer, IntegerType bytes) noexcept
{
    return unalignedPointerCast<const Type*> (reinterpret_cast<const char*> (basePointer) + bytes);
}

//==============================================================================
#if STL_MAC || STL_IOS || DOXYGEN

 /** A handy C++ wrapper that creates and deletes an NSAutoreleasePool object using RAII.
     You should use the STL_AUTORELEASEPOOL macro to create a local auto-release pool on the stack.

     @tags{Core}
 */
 class   ScopedAutoReleasePool
 {
 public:
     ScopedAutoReleasePool();
     ~ScopedAutoReleasePool();

 private:
     void* pool;

     STL_DECLARE_NON_COPYABLE (ScopedAutoReleasePool)
 };

 /** A macro that can be used to easily declare a local ScopedAutoReleasePool
     object for RAII-based obj-C autoreleasing.
     Because this may use the \@autoreleasepool syntax, you must follow the macro with
     a set of braces to mark the scope of the pool.
 */
#if (STL_COMPILER_SUPPORTS_ARC && defined (__OBJC__)) || DOXYGEN
 #define STL_AUTORELEASEPOOL  @autoreleasepool
#else
 #define STL_AUTORELEASEPOOL  const stl::ScopedAutoReleasePool STL_JOIN_MACRO (autoReleasePool_, __LINE__);
#endif

#else
 #define STL_AUTORELEASEPOOL
#endif

//==============================================================================
/* In a Windows DLL build, we'll expose some malloc/free functions that live inside the DLL, and use these for
   allocating all the objects - that way all juce objects in the DLL and in the host will live in the same heap,
   avoiding problems when an object is created in one module and passed across to another where it is deleted.
   By piggy-backing on the STL_LEAK_DETECTOR macro, these allocators can be injected into most juce classes.
*/
#if STL_MSVC && (defined (STL_DLL) || defined (STL_DLL_BUILD)) && ! (STL_DISABLE_DLL_ALLOCATORS || DOXYGEN)
 extern  void* juceDLL_malloc (size_t);
 extern  void  juceDLL_free (void*);

 #define STL_LEAK_DETECTOR(OwnerClass)  public:\
    static void* operator new (size_t sz)           { return stl::juceDLL_malloc (sz); } \
    static void* operator new (size_t, void* p)     { return p; } \
    static void operator delete (void* p)           { stl::juceDLL_free (p); } \
    static void operator delete (void*, void*)      {}
#endif

//==============================================================================
/** (Deprecated) This was a Windows-specific way of checking for object leaks - now please
    use the STL_LEAK_DETECTOR instead.
*/
#ifndef stl_UseDebuggingNewOperator
 #define stl_UseDebuggingNewOperator
#endif

/** Converts an owning raw pointer into a unique_ptr, deriving the
    type of the unique_ptr automatically.

    This should only be used with pointers to single objects.
    Do NOT pass a pointer to an array to this function, as the
    destructor of the unique_ptr will incorrectly call `delete`
    instead of `delete[]` on the pointer.
*/
template <typename T>
std::unique_ptr<T> rawToUniquePtr (T* ptr)
{
    return std::unique_ptr<T> (ptr);
}

} // namespace stl
