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


/*******************************************************************************
 The block below describes the properties of this module, and is read by
 the Projucer to automatically generate project code that uses it.
 For details about the syntax and how to create or use a module, see the
 STL Module Format.md file.


 BEGIN_STL_MODULE_DECLARATION

  ID:                 stl_core
  vendor:             juce
  version:            7.0.8
  name:               STL core classes
  description:        The essential set of basic STL classes, as required by all the other STL modules. Includes text, container, memory, threading and i/o functionality.
  website:            http://www.juce.com/juce
  license:            ISC
  minimumCppStandard: 17

  dependencies:
  OSXFrameworks:      Cocoa Foundation IOKit Security
  iOSFrameworks:      Foundation
  linuxLibs:          rt dl pthread
  mingwLibs:          uuid wsock32 wininet version ole32 ws2_32 oleaut32 imm32 comdlg32 shlwapi rpcrt4 winmm

 END_STL_MODULE_DECLARATION

*******************************************************************************/


#pragma once
#define STL_CORE_H_INCLUDED
#define STL_CORE_INCLUDE_OBJC_HELPERS 0
#define STL_CORE_INCLUDE_COM_SMART_PTR 1
#define STL_CORE_INCLUDE_NATIVE_HEADERS 1
#define STL_CORE_INCLUDE_JNI_HELPERS 1
//==============================================================================
#ifdef _MSC_VER
#pragma warning (push)
// Disable warnings for long class names, padding, and undefined preprocessor definitions.
#pragma warning (disable: 4251 4786 4668 4820)
#ifdef __INTEL_COMPILER
#pragma warning (disable: 1125)
#endif
#endif

#include "system/stl_TargetPlatform.h"

//==============================================================================
/** Config: STL_FORCE_DEBUG

    Normally, STL_DEBUG is set to 1 or 0 based on compiler and project settings,
    but if you define this value, you can override this to force it to be true or false.
*/
#ifndef STL_FORCE_DEBUG
//#define STL_FORCE_DEBUG 0
#endif

//==============================================================================
/** Config: STL_LOG_ASSERTIONS

    If this flag is enabled, the jassert and jassertfalse macros will always use Logger::writeToLog()
    to write a message when an assertion happens.

    Enabling it will also leave this turned on in release builds. When it's disabled,
    however, the jassert and jassertfalse macros will not be compiled in a
    release build.

    @see jassert, jassertfalse, Logger
*/
#ifndef STL_LOG_ASSERTIONS
#if STL_ANDROID
#define STL_LOG_ASSERTIONS 0
#else
#define STL_LOG_ASSERTIONS 0
#endif
#endif

//==============================================================================
/** Config: STL_CHECK_MEMORY_LEAKS

    Enables a memory-leak check for certain objects when the app terminates. See the LeakedObjectDetector
    class and the STL_LEAK_DETECTOR macro for more details about enabling leak checking for specific classes.
*/
#if STL_DEBUG && !defined (STL_CHECK_MEMORY_LEAKS)
#define STL_CHECK_MEMORY_LEAKS 1
#endif

//==============================================================================
/** Config: STL_DONT_AUTOLINK_TO_WIN32_LIBRARIES

    In a Windows build, this can be used to stop the required system libs being
    automatically added to the link stage.
*/
#ifndef STL_DONT_AUTOLINK_TO_WIN32_LIBRARIES
#define STL_DONT_AUTOLINK_TO_WIN32_LIBRARIES 0
#endif

/** Config: STL_INCLUDE_ZLIB_CODE
    This can be used to disable Juce's embedded 3rd-party zlib code.
    You might need to tweak this if you're linking to an external zlib library in your app,
    but for normal apps, this option should be left alone.

    If you disable this, you might also want to set a value for STL_ZLIB_INCLUDE_PATH, to
    specify the path where your zlib headers live.
*/
#ifndef STL_INCLUDE_ZLIB_CODE
#define STL_INCLUDE_ZLIB_CODE 1
#endif

#ifndef STL_ZLIB_INCLUDE_PATH
#define STL_ZLIB_INCLUDE_PATH <zlib.h>
#endif

/** Config: STL_USE_CURL
    Enables http/https support via libcurl (Linux only). Enabling this will add an additional
    run-time dynamic dependency to libcurl.

    If you disable this then https/ssl support will not be available on Linux.
*/
#ifndef STL_USE_CURL
#define STL_USE_CURL 0
#endif

/** Config: STL_LOAD_CURL_SYMBOLS_LAZILY
    If enabled, STL will load libcurl lazily when required (for example, when WebInputStream
    is used). Enabling this flag may also help with library dependency errors as linking
    libcurl at compile-time may instruct the linker to hard depend on a specific version
    of libcurl. It's also useful if you want to limit the amount of STL dependencies and
    you are not using WebInputStream or the URL classes.
*/
#ifndef STL_LOAD_CURL_SYMBOLS_LAZILY
#define STL_LOAD_CURL_SYMBOLS_LAZILY 0
#endif

/** Config: STL_CATCH_UNHANDLED_EXCEPTIONS
    If enabled, this will add some exception-catching code to forward unhandled exceptions
    to your STLApplicationBase::unhandledException() callback.
*/
#ifndef STL_CATCH_UNHANDLED_EXCEPTIONS
#define STL_CATCH_UNHANDLED_EXCEPTIONS 0
#endif

/** Config: STL_ALLOW_STATIC_NULL_VARIABLES
    If disabled, this will turn off dangerous static globals like String::empty, var::null, etc
    which can cause nasty order-of-initialisation problems if they are referenced during static
    constructor code.
*/
#ifndef STL_ALLOW_STATIC_NULL_VARIABLES
#define STL_ALLOW_STATIC_NULL_VARIABLES 0
#endif

/** Config: STL_STRICT_REFCOUNTEDPOINTER
    If enabled, this will make the ReferenceCountedObjectPtr class stricter about allowing
    itself to be cast directly to a raw pointer. By default this is disabled, for compatibility
    with old code, but if possible, you should always enable it to improve code safety!
*/
#ifndef STL_STRICT_REFCOUNTEDPOINTER
#define STL_STRICT_REFCOUNTEDPOINTER 0
#endif

/** Config: STL_ENABLE_ALLOCATION_HOOKS
    If enabled, this will add global allocation functions with built-in assertions, which may
    help when debugging allocations in unit tests.
*/
#ifndef STL_ENABLE_ALLOCATION_HOOKS
#define STL_ENABLE_ALLOCATION_HOOKS 0
#endif

#ifndef STL_STRING_UTF_TYPE
#define STL_STRING_UTF_TYPE 8
#endif

//==============================================================================
//==============================================================================

#if STL_CORE_INCLUDE_NATIVE_HEADERS

#include "native/stl_BasicNativeHeaders.h"

#endif

#if STL_WINDOWS
#undef small
#endif

#include "system/stl_StandardHeader.h"

namespace stl
{
    class StringRef;

    class MemoryBlock;

    class File;

    class InputStream;

    class OutputStream;

    class DynamicObject;

    class FileInputStream;

    class FileOutputStream;


    extern bool stl_isRunningUnderDebugger() noexcept;

    extern void logAssertion(const char* file, int line) noexcept;
}

#include "misc/stl_EnumHelpers.h"
#include "memory/stl_Memory.h"
#include "maths/stl_MathsFunctions.h"
#include "memory/stl_ByteOrder.h"
#include "memory/stl_Atomic.h"
#include "text/stl_CharacterFunctions.h"

STL_BEGIN_IGNORE_WARNINGS_MSVC(4514 4996)

#include "text/stl_CharPointer_UTF8.h"
#include "text/stl_CharPointer_UTF16.h"
#include "text/stl_CharPointer_UTF32.h"
#include "text/stl_CharPointer_ASCII.h"

STL_END_IGNORE_WARNINGS_MSVC

/**/
#if STL_CORE_INCLUDE_OBJC_HELPERS && (STL_MAC || STL_IOS)

#include "native/stl_CFHelpers_mac.h"
#include "native/stl_ObjCHelpers_mac.h"

#endif

#if STL_CORE_INCLUDE_COM_SMART_PTR && STL_WINDOWS

#include "native/stl_ComSmartPtr_windows.h"

#endif

#if STL_CORE_INCLUDE_JNI_HELPERS && STL_ANDROID
#include <jni.h>

#endif


#ifndef DOXYGEN
namespace stl
{
    /*
       As the very long class names here try to explain, the purpose of this code is to cause
       a linker error if not all of your compile units are consistent in the options that they
       enable before including STL headers. The reason this is important is that if you have
       two cpp files, and one includes the juce headers with debug enabled, and another does so
       without that, then each will be generating code with different class layouts, and you'll
       get subtle and hard-to-track-down memory corruption!
    */
#if STL_DEBUG


#else


#endif
}
#endif

STL_END_IGNORE_WARNINGS_MSVC

// In DLL builds, need to disable this warnings for other modules
#if defined (STL_DLL_BUILD) || defined (STL_DLL)
STL_IGNORE_MSVC (4251)
#endif


#include <cctype>
#include <cstdarg>
#include <locale>
#include <thread>

#if !(STL_ANDROID || STL_BSD)

#ifndef __ANDROID__
#include <sys/timeb.h>
#endif
#include <cwctype>

#endif

#if STL_WINDOWS
#include <ctime>

#if STL_MINGW
#include <ws2spi.h>
#include <cstdio>
#include <locale.h>
#include <sys/time.h>
#else
STL_BEGIN_IGNORE_WARNINGS_MSVC (4091)
#include <Dbghelp.h>
  STL_END_IGNORE_WARNINGS_MSVC

#if ! STL_DONT_AUTOLINK_TO_WIN32_LIBRARIES
#pragma comment (lib, "DbgHelp.lib")
#endif
#endif

#else
#if STL_LINUX || STL_BSD || STL_ANDROID
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/errno.h>
#include <unistd.h>
#include <netinet/in.h>
#endif

#if STL_WASM
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/stat.h>
#endif

#if STL_LINUX || STL_BSD
#include <stdio.h>
#include <langinfo.h>
#include <ifaddrs.h>
#include <sys/resource.h>

#if STL_USE_CURL
#endif
#endif

#include <pwd.h>
#include <fcntl.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <sys/time.h>
#include <net/if.h>
#include <sys/ioctl.h>

//#include <curl/curl.h>
#if !(STL_ANDROID || STL_WASM)
#ifndef __ANDROID__
#include <execinfo.h>
#endif
#endif
#endif

#if STL_MAC || STL_IOS

#include <xlocale.h>
#include <mach/mach.h>

#endif

#if STL_ANDROID
#include <ifaddrs.h>
#include <android/log.h>
#endif


#ifndef STL_API
#ifdef _WIN32
#define STL_API __declspec(dllexport)
#else
#define STL_API __attribute__((visibility("default")))
#endif
#endif
