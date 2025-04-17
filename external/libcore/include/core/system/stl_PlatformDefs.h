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

#pragma once
#include "../stl_core.h"
namespace stl
{

//==============================================================================
/*  This file defines miscellaneous macros for debugging, assertions, etc.
*/

//==============================================================================
#ifdef STL_FORCE_DEBUG
 #undef STL_DEBUG

 #if STL_FORCE_DEBUG
  #define STL_DEBUG 1
 #endif
#endif

/** This macro defines the C calling convention used as the standard for STL calls. */
#if STL_WINDOWS
 #define STL_CALLTYPE   __declspec(dllexport)
 #define STL_CDECL      __cdecl
#else
 #define STL_CALLTYPE
 #define STL_CDECL
#endif

//==============================================================================
// Debugging and assertion macros

#ifndef STL_LOG_CURRENT_ASSERTION
 #if STL_LOG_ASSERTIONS || STL_DEBUG
  #define STL_LOG_CURRENT_ASSERTION    stl::logAssertion (__FILE__, __LINE__);
 #else
  #define STL_LOG_CURRENT_ASSERTION
 #endif
#endif

//==============================================================================
#if STL_IOS || STL_LINUX || STL_BSD
  /** This will try to break into the debugger if the app is currently being debugged.
      If called by an app that's not being debugged, the behaviour isn't defined - it may
      crash or not, depending on the platform.
      @see jassert()
  */
  #define STL_BREAK_IN_DEBUGGER        { ::kill (0, SIGTRAP); }
#elif STL_MSVC
  #ifndef __INTEL_COMPILER
    #pragma intrinsic (__debugbreak)
  #endif
  #define STL_BREAK_IN_DEBUGGER        { __debugbreak(); }
#elif STL_INTEL && (STL_GCC || STL_CLANG || STL_MAC)
  #if STL_NO_INLINE_ASM
   #define STL_BREAK_IN_DEBUGGER       { }
  #else
   #define STL_BREAK_IN_DEBUGGER       { asm ("int $3"); }
  #endif
#elif STL_ARM && STL_MAC
  #define STL_BREAK_IN_DEBUGGER        { __builtin_debugtrap(); }
#elif STL_ANDROID
  #define STL_BREAK_IN_DEBUGGER        { __builtin_trap(); }
#else
  #define STL_BREAK_IN_DEBUGGER        { __asm int 3 }
#endif

#if STL_CLANG && defined (__has_feature) && ! defined (STL_ANALYZER_NORETURN)
 #if __has_feature (attribute_analyzer_noreturn)
  inline void __attribute__ ((analyzer_noreturn)) stl_assert_noreturn() {}
  #define STL_ANALYZER_NORETURN stl::stl_assert_noreturn();
 #endif
#endif

#ifndef STL_ANALYZER_NORETURN
 #define STL_ANALYZER_NORETURN
#endif

/** Used to silence Wimplicit-fallthrough on Clang and GCC where available
    as there are a few places in the codebase where we need to do this
    deliberately and want to ignore the warning.
*/
#if STL_CLANG
 #if __has_cpp_attribute(clang::fallthrough)
  #define STL_FALLTHROUGH [[clang::fallthrough]];
 #else
  #define STL_FALLTHROUGH
 #endif
#elif STL_GCC
 #if __GNUC__ >= 7
  #define STL_FALLTHROUGH [[gnu::fallthrough]];
 #else
  #define STL_FALLTHROUGH
 #endif
#else
 #define STL_FALLTHROUGH
#endif

//==============================================================================
#if STL_MSVC && ! defined (DOXYGEN)
 #define STL_BLOCK_WITH_FORCED_SEMICOLON(x) \
   __pragma(warning(push)) \
   __pragma(warning(disable:4127)) \
   do { x } while (false) \
   __pragma(warning(pop))
#else
 /** This is the good old C++ trick for creating a macro that forces the user to put
    a semicolon after it when they use it.
 */
 #define STL_BLOCK_WITH_FORCED_SEMICOLON(x) do { x } while (false)
#endif

//==============================================================================
#if (STL_DEBUG && ! STL_DISABLE_ASSERTIONS) || DOXYGEN
  /** Writes a string to the standard error stream.
      Note that as well as a single string, you can use this to write multiple items
      as a stream, e.g.
      @code
        DBG ("foo = " << foo << "bar = " << bar);
      @endcode
      The macro is only enabled in a debug build, so be careful not to use it with expressions
      that have important side-effects!
      @see Logger::outputDebugString
  */
  #define DBG(textToWrite)              STL_BLOCK_WITH_FORCED_SEMICOLON (stl::String tempDbgBuf; tempDbgBuf << textToWrite; stl::Logger::outputDebugString (tempDbgBuf);)

  //==============================================================================
  /** This will always cause an assertion failure.
      It is only compiled in a debug build, (unless STL_LOG_ASSERTIONS is enabled for your build).
      @see jassert
  */
  #define jassertfalse                  STL_BLOCK_WITH_FORCED_SEMICOLON (STL_LOG_CURRENT_ASSERTION; if (stl::stl_isRunningUnderDebugger()) STL_BREAK_IN_DEBUGGER; STL_ANALYZER_NORETURN)

  //==============================================================================
  /** Platform-independent assertion macro.

      This macro gets turned into a no-op when you're building with debugging turned off, so be
      careful that the expression you pass to it doesn't perform any actions that are vital for the
      correct behaviour of your program!
      @see jassertfalse
  */
  #define jassert(expression)           STL_BLOCK_WITH_FORCED_SEMICOLON (if (! (expression)) jassertfalse;)

  /** Platform-independent assertion macro which suppresses ignored-variable
      warnings in all build modes. You should probably use a plain jassert()
      and [[maybe_unused]] by default.
  */
  #define jassertquiet(expression)      STL_BLOCK_WITH_FORCED_SEMICOLON (if (! (expression)) jassertfalse;)

#else
  //==============================================================================
  // If debugging is disabled, these dummy debug and assertion macros are used..

  #define DBG(textToWrite)
  #define jassertfalse                  STL_BLOCK_WITH_FORCED_SEMICOLON (STL_LOG_CURRENT_ASSERTION;)

  #if STL_LOG_ASSERTIONS
   #define jassert(expression)          STL_BLOCK_WITH_FORCED_SEMICOLON (if (! (expression)) jassertfalse;)
   #define jassertquiet(expression)     STL_BLOCK_WITH_FORCED_SEMICOLON (if (! (expression)) jassertfalse;)
  #else
   #define jassert(expression)          STL_BLOCK_WITH_FORCED_SEMICOLON ( ; )
   #define jassertquiet(expression)     STL_BLOCK_WITH_FORCED_SEMICOLON (if (false) (void) (expression);)
  #endif

#endif

//==============================================================================
#ifndef DOXYGEN
 #define STL_JOIN_MACRO_HELPER(a, b) a ## b
 #define STL_STRINGIFY_MACRO_HELPER(a) #a
#endif

/** A good old-fashioned C macro concatenation helper.
    This combines two items (which may themselves be macros) into a single string,
    avoiding the pitfalls of the ## macro operator.
*/
#define STL_JOIN_MACRO(item1, item2)  STL_JOIN_MACRO_HELPER (item1, item2)

/** A handy C macro for stringifying any symbol, rather than just a macro parameter. */
#define STL_STRINGIFY(item)  STL_STRINGIFY_MACRO_HELPER (item)

//==============================================================================
/** This is a shorthand macro for deleting a class's copy constructor and
    copy assignment operator.

    For example, instead of
    @code
    class MyClass
    {
        etc..

    private:
        MyClass (const MyClass&);
        MyClass& operator= (const MyClass&);
    };@endcode

    ..you can just write:

    @code
    class MyClass
    {
        etc..

    private:
        STL_DECLARE_NON_COPYABLE (MyClass)
    };@endcode
*/
#define STL_DECLARE_NON_COPYABLE(className) \
    className (const className&) = delete;\
    className& operator= (const className&) = delete;

/** This is a shorthand macro for deleting a class's move constructor and
    move assignment operator.
*/
#define STL_DECLARE_NON_MOVEABLE(className) \
    className (className&&) = delete;\
    className& operator= (className&&) = delete;

/** This is a shorthand way of writing both a STL_DECLARE_NON_COPYABLE and
    STL_LEAK_DETECTOR macro for a class.
*/

//==============================================================================
#if DOXYGEN || ! defined (STL_LEAK_DETECTOR)
#if (DOXYGEN || STL_CHECK_MEMORY_LEAKS)
    /** This macro lets you embed a leak-detecting object inside a class.

      To use it, simply declare a STL_LEAK_DETECTOR (YourClassName) inside a private section
      of the class declaration. E.g.

      @code
      class MyClass
      {
      public:
          MyClass();
          void blahBlah();

      private:
          STL_LEAK_DETECTOR (MyClass)
      };
      @endcode

      @see STL_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR, LeakedObjectDetector
  */
  #define STL_LEAK_DETECTOR(OwnerClass) \
        friend class stl::LeakedObjectDetector<OwnerClass>; \
        static const char* getLeakedObjectClassName() noexcept { return #OwnerClass; } \
        stl::LeakedObjectDetector<OwnerClass> STL_JOIN_MACRO (leakDetector, __LINE__);
#else
#define STL_LEAK_DETECTOR(OwnerClass)
#endif
#endif

#define STL_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(className) \
    STL_DECLARE_NON_COPYABLE(className) \
    STL_LEAK_DETECTOR(className)

/** This macro can be added to class definitions to disable the use of new/delete to
    allocate the object on the heap, forcing it to only be used as a stack or member variable.
*/
#define STL_PREVENT_HEAP_ALLOCATION \
   private: \
    static void* operator new (size_t) = delete; \
    static void operator delete (void*) = delete;

//==============================================================================
#if STL_MSVC && ! defined (DOXYGEN)
 #define STL_WARNING_HELPER(file, line, mess) message(file "(" STL_STRINGIFY (line) ") : Warning: " #mess)
 #define STL_COMPILER_WARNING(message)  __pragma(STL_WARNING_HELPER (__FILE__, __LINE__, message))
#else
 #ifndef DOXYGEN
  #define STL_WARNING_HELPER(mess) message(#mess)
 #endif

 /** This macro allows you to emit a custom compiler warning message.
     Very handy for marking bits of code as "to-do" items, or for shaming
     code written by your co-workers in a way that's hard to ignore.

     GCC and Clang provide the \#warning directive, but MSVC doesn't, so this macro
     is a cross-compiler way to get the same functionality as \#warning.
 */
 #define STL_COMPILER_WARNING(message)  _Pragma(STL_STRINGIFY (STL_WARNING_HELPER (message)))
#endif


//==============================================================================
#if STL_DEBUG || DOXYGEN
  /** A platform-independent way of forcing an inline function.
      Use the syntax: @code
      forcedinline void myfunction (int x)
      @endcode
  */
  #define forcedinline  inline
#else
  #if STL_MSVC
   #define forcedinline       __forceinline
  #else
   #define forcedinline       inline __attribute__ ((always_inline))
  #endif
#endif

#if STL_MSVC || DOXYGEN
  /** This can be placed before a stack or member variable declaration to tell the compiler
      to align it to the specified number of bytes. */
  #define STL_ALIGN(bytes)   __declspec (align (bytes))
#else
  #define STL_ALIGN(bytes)   __attribute__ ((aligned (bytes)))
#endif

//==============================================================================
#if STL_ANDROID && ! defined (DOXYGEN)
 #define STL_MODAL_LOOPS_PERMITTED 0
#elif ! defined (STL_MODAL_LOOPS_PERMITTED)
 /** Some operating environments don't provide a modal loop mechanism, so this flag can be
     used to disable any functions that try to run a modal loop. */
 #define STL_MODAL_LOOPS_PERMITTED 0
#endif

//==============================================================================
#if STL_GCC || STL_CLANG
 #define STL_PACKED __attribute__ ((packed))
#elif ! defined (DOXYGEN)
 #define STL_PACKED
#endif

//==============================================================================
#if STL_GCC || DOXYGEN
 /** This can be appended to a function declaration to tell gcc to disable associative
     math optimisations which break some floating point algorithms. */
 #define STL_NO_ASSOCIATIVE_MATH_OPTIMISATIONS   __attribute__ ((__optimize__ ("no-associative-math")))
#else
 #define STL_NO_ASSOCIATIVE_MATH_OPTIMISATIONS
#endif

} // namespace stl
