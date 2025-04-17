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

#include "stl_TargetPlatform.h"

/** Return the Nth argument. By passing a variadic pack followed by N other
    parameters, we can select one of those N parameter based on the length of
    the parameter pack.
*/
#define STL_NTH_ARG_(_00, _01, _02, _03, _04, _05, _06, _07, _08, _09,        \
                      _10, _11, _12, _13, _14, _15, _16, _17, _18, _19,        \
                      _20, _21, _22, _23, _24, _25, _26, _27, _28, _29,        \
                      _30, _31, _32, _33, _34, _35, _36, _37, _38, _39,        \
                      _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, N, ...)\
    N

#define STL_EACH_00_(FN)
#define STL_EACH_01_(FN, X)      FN(X)
#define STL_EACH_02_(FN, X, ...) FN(X) STL_EACH_01_(FN, __VA_ARGS__)
#define STL_EACH_03_(FN, X, ...) FN(X) STL_EACH_02_(FN, __VA_ARGS__)
#define STL_EACH_04_(FN, X, ...) FN(X) STL_EACH_03_(FN, __VA_ARGS__)
#define STL_EACH_05_(FN, X, ...) FN(X) STL_EACH_04_(FN, __VA_ARGS__)
#define STL_EACH_06_(FN, X, ...) FN(X) STL_EACH_05_(FN, __VA_ARGS__)
#define STL_EACH_07_(FN, X, ...) FN(X) STL_EACH_06_(FN, __VA_ARGS__)
#define STL_EACH_08_(FN, X, ...) FN(X) STL_EACH_07_(FN, __VA_ARGS__)
#define STL_EACH_09_(FN, X, ...) FN(X) STL_EACH_08_(FN, __VA_ARGS__)
#define STL_EACH_10_(FN, X, ...) FN(X) STL_EACH_09_(FN, __VA_ARGS__)
#define STL_EACH_11_(FN, X, ...) FN(X) STL_EACH_10_(FN, __VA_ARGS__)
#define STL_EACH_12_(FN, X, ...) FN(X) STL_EACH_11_(FN, __VA_ARGS__)
#define STL_EACH_13_(FN, X, ...) FN(X) STL_EACH_12_(FN, __VA_ARGS__)
#define STL_EACH_14_(FN, X, ...) FN(X) STL_EACH_13_(FN, __VA_ARGS__)
#define STL_EACH_15_(FN, X, ...) FN(X) STL_EACH_14_(FN, __VA_ARGS__)
#define STL_EACH_16_(FN, X, ...) FN(X) STL_EACH_15_(FN, __VA_ARGS__)
#define STL_EACH_17_(FN, X, ...) FN(X) STL_EACH_16_(FN, __VA_ARGS__)
#define STL_EACH_18_(FN, X, ...) FN(X) STL_EACH_17_(FN, __VA_ARGS__)
#define STL_EACH_19_(FN, X, ...) FN(X) STL_EACH_18_(FN, __VA_ARGS__)
#define STL_EACH_20_(FN, X, ...) FN(X) STL_EACH_19_(FN, __VA_ARGS__)
#define STL_EACH_21_(FN, X, ...) FN(X) STL_EACH_20_(FN, __VA_ARGS__)
#define STL_EACH_22_(FN, X, ...) FN(X) STL_EACH_21_(FN, __VA_ARGS__)
#define STL_EACH_23_(FN, X, ...) FN(X) STL_EACH_22_(FN, __VA_ARGS__)
#define STL_EACH_24_(FN, X, ...) FN(X) STL_EACH_23_(FN, __VA_ARGS__)
#define STL_EACH_25_(FN, X, ...) FN(X) STL_EACH_24_(FN, __VA_ARGS__)
#define STL_EACH_26_(FN, X, ...) FN(X) STL_EACH_25_(FN, __VA_ARGS__)
#define STL_EACH_27_(FN, X, ...) FN(X) STL_EACH_26_(FN, __VA_ARGS__)
#define STL_EACH_28_(FN, X, ...) FN(X) STL_EACH_27_(FN, __VA_ARGS__)
#define STL_EACH_29_(FN, X, ...) FN(X) STL_EACH_28_(FN, __VA_ARGS__)
#define STL_EACH_30_(FN, X, ...) FN(X) STL_EACH_29_(FN, __VA_ARGS__)
#define STL_EACH_31_(FN, X, ...) FN(X) STL_EACH_30_(FN, __VA_ARGS__)
#define STL_EACH_32_(FN, X, ...) FN(X) STL_EACH_31_(FN, __VA_ARGS__)
#define STL_EACH_33_(FN, X, ...) FN(X) STL_EACH_32_(FN, __VA_ARGS__)
#define STL_EACH_34_(FN, X, ...) FN(X) STL_EACH_33_(FN, __VA_ARGS__)
#define STL_EACH_35_(FN, X, ...) FN(X) STL_EACH_34_(FN, __VA_ARGS__)
#define STL_EACH_36_(FN, X, ...) FN(X) STL_EACH_35_(FN, __VA_ARGS__)
#define STL_EACH_37_(FN, X, ...) FN(X) STL_EACH_36_(FN, __VA_ARGS__)
#define STL_EACH_38_(FN, X, ...) FN(X) STL_EACH_37_(FN, __VA_ARGS__)
#define STL_EACH_39_(FN, X, ...) FN(X) STL_EACH_38_(FN, __VA_ARGS__)
#define STL_EACH_40_(FN, X, ...) FN(X) STL_EACH_39_(FN, __VA_ARGS__)
#define STL_EACH_41_(FN, X, ...) FN(X) STL_EACH_40_(FN, __VA_ARGS__)
#define STL_EACH_42_(FN, X, ...) FN(X) STL_EACH_41_(FN, __VA_ARGS__)
#define STL_EACH_43_(FN, X, ...) FN(X) STL_EACH_42_(FN, __VA_ARGS__)
#define STL_EACH_44_(FN, X, ...) FN(X) STL_EACH_43_(FN, __VA_ARGS__)
#define STL_EACH_45_(FN, X, ...) FN(X) STL_EACH_44_(FN, __VA_ARGS__)
#define STL_EACH_46_(FN, X, ...) FN(X) STL_EACH_45_(FN, __VA_ARGS__)
#define STL_EACH_47_(FN, X, ...) FN(X) STL_EACH_46_(FN, __VA_ARGS__)
#define STL_EACH_48_(FN, X, ...) FN(X) STL_EACH_47_(FN, __VA_ARGS__)
#define STL_EACH_49_(FN, X, ...) FN(X) STL_EACH_48_(FN, __VA_ARGS__)

/** Apply the macro FN to each of the other arguments. */
#define STL_EACH(FN, ...)                                                     \
    STL_NTH_ARG_(, __VA_ARGS__,                                               \
                  STL_EACH_49_,                                               \
                  STL_EACH_48_,                                               \
                  STL_EACH_47_,                                               \
                  STL_EACH_46_,                                               \
                  STL_EACH_45_,                                               \
                  STL_EACH_44_,                                               \
                  STL_EACH_43_,                                               \
                  STL_EACH_42_,                                               \
                  STL_EACH_41_,                                               \
                  STL_EACH_40_,                                               \
                  STL_EACH_39_,                                               \
                  STL_EACH_38_,                                               \
                  STL_EACH_37_,                                               \
                  STL_EACH_36_,                                               \
                  STL_EACH_35_,                                               \
                  STL_EACH_34_,                                               \
                  STL_EACH_33_,                                               \
                  STL_EACH_32_,                                               \
                  STL_EACH_31_,                                               \
                  STL_EACH_30_,                                               \
                  STL_EACH_29_,                                               \
                  STL_EACH_28_,                                               \
                  STL_EACH_27_,                                               \
                  STL_EACH_26_,                                               \
                  STL_EACH_25_,                                               \
                  STL_EACH_24_,                                               \
                  STL_EACH_23_,                                               \
                  STL_EACH_22_,                                               \
                  STL_EACH_21_,                                               \
                  STL_EACH_20_,                                               \
                  STL_EACH_19_,                                               \
                  STL_EACH_18_,                                               \
                  STL_EACH_17_,                                               \
                  STL_EACH_16_,                                               \
                  STL_EACH_15_,                                               \
                  STL_EACH_14_,                                               \
                  STL_EACH_13_,                                               \
                  STL_EACH_12_,                                               \
                  STL_EACH_11_,                                               \
                  STL_EACH_10_,                                               \
                  STL_EACH_09_,                                               \
                  STL_EACH_08_,                                               \
                  STL_EACH_07_,                                               \
                  STL_EACH_06_,                                               \
                  STL_EACH_05_,                                               \
                  STL_EACH_04_,                                               \
                  STL_EACH_03_,                                               \
                  STL_EACH_02_,                                               \
                  STL_EACH_01_,                                               \
                  STL_EACH_00_)                                               \
    (FN, __VA_ARGS__)

/** Concatenate two tokens to form a new token. */
#define STL_CONCAT_(a, b) a##b
#define STL_CONCAT(a, b) STL_CONCAT_(a, b)

/** Quote the argument, turning it into a string. */
#define STL_TO_STRING(x) #x

#if STL_CLANG || STL_GCC || STL_MINGW
    #define STL_IGNORE_GCC_IMPL_(compiler, warning)
    #define STL_IGNORE_GCC_IMPL_0(compiler, warning)
    #define STL_IGNORE_GCC_IMPL_1(compiler, warning)                          \
        _Pragma(STL_TO_STRING(compiler diagnostic ignored warning))

    /** If 'warning' is recognised by this compiler, ignore it. */
    #if defined (__has_warning)
        #define STL_IGNORE_GCC_LIKE(compiler, warning)                        \
            STL_CONCAT(STL_IGNORE_GCC_IMPL_, __has_warning(warning))(compiler, warning)
    #else
        #define STL_IGNORE_GCC_LIKE(compiler, warning)                        \
            STL_IGNORE_GCC_IMPL_1(compiler, warning)
    #endif

    /** Ignore GCC/clang-specific warnings. */
    #define STL_IGNORE_GCC(warning)   STL_IGNORE_GCC_LIKE(GCC, warning)
    #define STL_IGNORE_clang(warning) STL_IGNORE_GCC_LIKE(clang, warning)

    #define STL_IGNORE_WARNINGS_GCC_LIKE(compiler, ...)                       \
        _Pragma(STL_TO_STRING(compiler diagnostic push))                      \
        STL_EACH(STL_CONCAT(STL_IGNORE_, compiler), __VA_ARGS__)

    /** Push a new warning scope, and then ignore each warning for either clang
        or gcc. If the compiler doesn't support __has_warning, we add -Wpragmas
        as the first disabled warning because otherwise we might get complaints
        about unknown warning options.
    */
    #if defined (__has_warning)
        #define STL_PUSH_WARNINGS_GCC_LIKE(compiler, ...)                     \
            STL_IGNORE_WARNINGS_GCC_LIKE(compiler, __VA_ARGS__)
    #else
        #define STL_PUSH_WARNINGS_GCC_LIKE(compiler, ...)                     \
            STL_IGNORE_WARNINGS_GCC_LIKE(compiler, "-Wpragmas", __VA_ARGS__)
    #endif

    /** Pop the current warning scope. */
    #define STL_POP_WARNINGS_GCC_LIKE(compiler)                               \
        _Pragma(STL_TO_STRING(compiler diagnostic pop))

    /** Push/pop warnings on compilers with gcc-like warning flags.
        These macros expand to nothing on other compilers (like MSVC).
    */
    #if STL_CLANG
        #define STL_BEGIN_IGNORE_WARNINGS_GCC_LIKE(...) STL_PUSH_WARNINGS_GCC_LIKE(clang, __VA_ARGS__)
        #define STL_END_IGNORE_WARNINGS_GCC_LIKE STL_POP_WARNINGS_GCC_LIKE(clang)
    #else
        #define STL_BEGIN_IGNORE_WARNINGS_GCC_LIKE(...) STL_PUSH_WARNINGS_GCC_LIKE(GCC, __VA_ARGS__)
        #define STL_END_IGNORE_WARNINGS_GCC_LIKE STL_POP_WARNINGS_GCC_LIKE(GCC)
    #endif
#else
    #define STL_BEGIN_IGNORE_WARNINGS_GCC_LIKE(...)
    #define STL_END_IGNORE_WARNINGS_GCC_LIKE
#endif

/** Push/pop warnings on MSVC. These macros expand to nothing on other
    compilers (like clang and gcc).
*/
#if STL_MSVC
    #define STL_IGNORE_MSVC(warnings) __pragma(warning(disable:warnings))
    #define STL_BEGIN_IGNORE_WARNINGS_LEVEL_MSVC(level, warnings)              \
        __pragma(warning(push, level)) STL_IGNORE_MSVC(warnings)
    #define STL_BEGIN_IGNORE_WARNINGS_MSVC(warnings)                           \
        __pragma(warning(push)) STL_IGNORE_MSVC(warnings)
    #define STL_END_IGNORE_WARNINGS_MSVC __pragma(warning(pop))
#else
    #define STL_IGNORE_MSVC(warnings)
    #define STL_BEGIN_IGNORE_WARNINGS_LEVEL_MSVC(level, warnings)
    #define STL_BEGIN_IGNORE_WARNINGS_MSVC(warnings)
    #define STL_END_IGNORE_WARNINGS_MSVC
#endif

#if STL_MAC || STL_IOS
    #define STL_SANITIZER_ATTRIBUTE_MINIMUM_CLANG_VERSION 11
#else
    #define STL_SANITIZER_ATTRIBUTE_MINIMUM_CLANG_VERSION 9
#endif

/** Disable sanitizers for a range of functions.

    This functionality doesn't seem to exist on GCC yet, so at the moment this only works for clang.
*/
#if STL_CLANG && __clang_major__ >= STL_SANITIZER_ATTRIBUTE_MINIMUM_CLANG_VERSION
    #define STL_BEGIN_NO_SANITIZE(warnings)                                    \
        _Pragma (STL_TO_STRING (clang attribute push (__attribute__ ((no_sanitize (warnings))), apply_to=function)))
    #define STL_END_NO_SANITIZE _Pragma (STL_TO_STRING (clang attribute pop))
#else
    #define STL_BEGIN_NO_SANITIZE(warnings)
    #define STL_END_NO_SANITIZE
#endif

#undef STL_SANITIZER_ATTRIBUTE_MINIMUM_CLANG_VERSION
