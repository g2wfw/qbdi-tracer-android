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

//==============================================================================
/*  This file figures out which platform is being built, and defines some macros
    that the rest of the code can use for OS-specific compilation.

    Macros that will be set here are:

    - One of STL_WINDOWS, STL_MAC STL_LINUX, STL_IOS, STL_ANDROID, etc.
    - Either STL_32BIT or STL_64BIT, depending on the architecture.
    - Either STL_LITTLE_ENDIAN or STL_BIG_ENDIAN.
    - Either STL_INTEL or STL_ARM
    - Either STL_GCC or STL_CLANG or STL_MSVC
*/
#define STL_GLOBAL_MODULE_SETTINGS_INCLUDED 1
//==============================================================================
#ifdef STL_APP_CONFIG_HEADER
 #include STL_APP_CONFIG_HEADER
#elif ! defined (STL_GLOBAL_MODULE_SETTINGS_INCLUDED)
 /*
    Most projects will contain a global header file containing various settings that
    should be applied to all the code in your project. If you use the projucer, it'll
    set up a global header file for you automatically, but if you're doing things manually,
    you may want to set the STL_APP_CONFIG_HEADER macro with the name of a file to include,
    or just include one before all the module cpp files, in which you set
    STL_GLOBAL_MODULE_SETTINGS_INCLUDED=1 to silence this error.
    (Or if you don't need a global header, then you can just define STL_GLOBAL_MODULE_SETTINGS_INCLUDED
    globally to avoid this error).

    Note for people who hit this error when trying to compile a STL project created by
    a pre-v4.2 version of the Introjucer/Projucer, it's very easy to fix: just re-save
    your project with the latest version of the Projucer, and it'll magically fix this!
 */
 #error "No global header file was included!"
#endif

//==============================================================================
#if defined (_WIN32) || defined (_WIN64)
  #define       STL_WINDOWS 1
#elif defined (STL_ANDROID)
  #undef        STL_ANDROID
  #define       STL_ANDROID 1
#elif defined (__FreeBSD__) || defined (__OpenBSD__)
  #define       STL_BSD 1
#elif defined (LINUX) || defined (__linux__)
  #define       STL_LINUX 1
#elif defined (__APPLE_CPP__) || defined (__APPLE_CC__)
  #define CF_EXCLUDE_CSTD_HEADERS 1
  #include <TargetConditionals.h> // (needed to find out what platform we're using)
  #include <AvailabilityMacros.h>

  #if TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR
    #define     STL_IPHONE 1
    #define     STL_IOS 1
  #else
    #define     STL_MAC 1
  #endif
#elif defined (__wasm__)
  #define       STL_WASM 1
#else
  #error "Unknown platform!"
#endif

//==============================================================================
#if STL_WINDOWS
  #ifdef _MSC_VER
    #ifdef _WIN64
      #define STL_64BIT 1
    #else
      #define STL_32BIT 1
    #endif
  #endif

  #ifdef _DEBUG
    #define STL_DEBUG 1
  #endif

  #ifdef __MINGW32__
    #define STL_MINGW 1
    #ifdef __MINGW64__
      #define STL_64BIT 1
    #else
      #define STL_32BIT 1
    #endif
  #endif

  /** If defined, this indicates that the processor is little-endian. */
  #define STL_LITTLE_ENDIAN 1

  #if defined (_M_ARM) || defined (_M_ARM64) || defined (__arm__) || defined (__aarch64__)
    #define STL_ARM 1
  #else
    #define STL_INTEL 1
  #endif
#endif
#ifdef STL_MAC
#ifndef NDEBUG
#define NDEBUG 0
#endif
#endif
//==============================================================================
#if STL_MAC || STL_IOS

  #if defined (DEBUG) || defined (_DEBUG) || ! (defined (NDEBUG) || defined (_NDEBUG))
    #define STL_DEBUG 1
  #endif

  #if ! (defined (DEBUG) || defined (_DEBUG) || defined (NDEBUG) || defined (_NDEBUG))
    #warning "Neither NDEBUG or DEBUG has been defined - you should set one of these to make it clear whether this is a release build,"
  #endif

  #ifdef __LITTLE_ENDIAN__
    #define STL_LITTLE_ENDIAN 1
  #else
    #define STL_BIG_ENDIAN 1
  #endif

  #ifdef __LP64__
    #define STL_64BIT 1
  #else
    #define STL_32BIT 1
  #endif

  #if defined (__ppc__) || defined (__ppc64__)
    #error "PowerPC is no longer supported by STL!"
  #elif defined (__arm__) || defined (__arm64__)
    #define STL_ARM 1
  #else
    #define STL_INTEL 1
  #endif

  #if STL_MAC
    #if ! defined (MAC_OS_X_VERSION_10_13)
      #error "The 10.14 SDK (Xcode 10.1+) is required to build STL apps. You can create apps that run on macOS 10.9+ by changing the deployment target."
    #elif MAC_OS_X_VERSION_MIN_REQUIRED < MAC_OS_X_VERSION_10_9
      #error "Building for OSX 10.8 and earlier is no longer supported!"
    #endif
  #endif
#endif

//==============================================================================
#if STL_LINUX || STL_ANDROID || STL_BSD

  #ifdef _DEBUG
    #define STL_DEBUG 1
  #endif

  // Allow override for big-endian Linux platforms
  #if defined (__LITTLE_ENDIAN__) || ! defined (STL_BIG_ENDIAN)
    #define STL_LITTLE_ENDIAN 1
    #undef STL_BIG_ENDIAN
  #else
    #undef STL_LITTLE_ENDIAN
    #define STL_BIG_ENDIAN 1
  #endif

  #if defined (__LP64__) || defined (_LP64) || defined (__arm64__)
    #define STL_64BIT 1
  #else
    #define STL_32BIT 1
  #endif

  #if defined (__arm__) || defined (__arm64__) || defined (__aarch64__)
    #define STL_ARM 1
  #elif __MMX__ || __SSE__ || __amd64__
    #define STL_INTEL 1
  #endif
#endif

//==============================================================================
// Compiler type macros.

#if defined (__clang__)
  #define STL_CLANG 1

#elif defined (__GNUC__)
  #define STL_GCC 1

#elif defined (_MSC_VER)
  #define STL_MSVC 1

#else
  #error unknown compiler
#endif
