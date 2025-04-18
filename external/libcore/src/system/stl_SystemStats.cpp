/*
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

#include "stl_core.h"
#include "system/stl_SystemStats.h"
#include "text/stl_NewLine.h"
#include "files/stl_File.h"
#ifdef STL_MAC
#include <unistd.h>
#endif
namespace stl
{

String SystemStats::getSTLVersion()
{
    // Some basic tests, to keep an eye on things and make sure these types work ok
    // on all platforms. Let me know if any of these assertions fail on your system!
    static_assert (sizeof (pointer_sized_int) == sizeof (void*), "Basic sanity test failed: please report!");
    static_assert (sizeof (int8) == 1,                           "Basic sanity test failed: please report!");
    static_assert (sizeof (uint8) == 1,                          "Basic sanity test failed: please report!");
    static_assert (sizeof (int16) == 2,                          "Basic sanity test failed: please report!");
    static_assert (sizeof (uint16) == 2,                         "Basic sanity test failed: please report!");
    static_assert (sizeof (int32) == 4,                          "Basic sanity test failed: please report!");
    static_assert (sizeof (uint32) == 4,                         "Basic sanity test failed: please report!");
    static_assert (sizeof (int64) == 8,                          "Basic sanity test failed: please report!");
    static_assert (sizeof (uint64) == 8,                         "Basic sanity test failed: please report!");

    return "STL v" STL_STRINGIFY (STL_MAJOR_VERSION)
                "." STL_STRINGIFY (STL_MINOR_VERSION)
                "." STL_STRINGIFY (STL_BUILDNUMBER);
}

#if STL_ANDROID && ! defined (STL_DISABLE_STL_VERSION_PRINTING)
 #define STL_DISABLE_STL_VERSION_PRINTING 1
#endif

#if STL_DEBUG && ! STL_DISABLE_STL_VERSION_PRINTING
 struct JuceVersionPrinter
 {
     JuceVersionPrinter()
     {
         DBG (SystemStats::getSTLVersion());
     }
 };

 static JuceVersionPrinter juceVersionPrinter;
#endif

StringArray SystemStats::getDeviceIdentifiers()
{
    for (const auto flag : { MachineIdFlags::fileSystemId, MachineIdFlags::macAddresses  })
        if (auto ids = getMachineIdentifiers (flag); ! ids.isEmpty())
            return ids;

    jassertfalse; // Failed to create any IDs!
    return {};
}

String getLegacyUniqueDeviceID();

StringArray SystemStats::getMachineIdentifiers (MachineIdFlags flags)
{
    auto macAddressProvider = [] (StringArray& arr)
    {

    };

    auto fileSystemProvider = [] (StringArray& arr)
    {
       #if STL_WINDOWS
        File f (File::getSpecialLocation (File::windowsSystemDirectory));
       #else
        File f ("~");
       #endif
        if (auto num = f.getFileIdentifier())
            arr.add (String::toHexString ((int64) num));
    };

    auto legacyIdProvider = [] ([[maybe_unused]] StringArray& arr)
    {
       #if STL_WINDOWS
        arr.add (getLegacyUniqueDeviceID());
       #endif
    };

    auto uniqueIdProvider = [] (StringArray& arr)
    {
#ifdef STL_ANDROID
        arr.add ("");
#else
        arr.add (getUniqueDeviceID());
#endif
    };

    struct Provider { MachineIdFlags flag; void (*func) (StringArray&); };
    static const Provider providers[] =
    {
        { MachineIdFlags::macAddresses,   macAddressProvider },
        { MachineIdFlags::fileSystemId,   fileSystemProvider },
        { MachineIdFlags::legacyUniqueId, legacyIdProvider },
        { MachineIdFlags::uniqueId,       uniqueIdProvider }
    };

    StringArray ids;

    for (const auto& provider : providers)
    {
        if (hasBitValueSet (flags, provider.flag))
            provider.func (ids);
    }

    return ids;
}



static const CPUInformation& getCPUInformation() noexcept
{
    static CPUInformation info;
    return info;
}

int SystemStats::getNumCpus() noexcept          { return getCPUInformation().numLogicalCPUs; }
int SystemStats::getNumPhysicalCpus() noexcept  { return getCPUInformation().numPhysicalCPUs; }
bool SystemStats::hasMMX() noexcept             { return getCPUInformation().hasMMX; }
bool SystemStats::has3DNow() noexcept           { return getCPUInformation().has3DNow; }
bool SystemStats::hasFMA3() noexcept            { return getCPUInformation().hasFMA3; }
bool SystemStats::hasFMA4() noexcept            { return getCPUInformation().hasFMA4; }
bool SystemStats::hasSSE() noexcept             { return getCPUInformation().hasSSE; }
bool SystemStats::hasSSE2() noexcept            { return getCPUInformation().hasSSE2; }
bool SystemStats::hasSSE3() noexcept            { return getCPUInformation().hasSSE3; }
bool SystemStats::hasSSSE3() noexcept           { return getCPUInformation().hasSSSE3; }
bool SystemStats::hasSSE41() noexcept           { return getCPUInformation().hasSSE41; }
bool SystemStats::hasSSE42() noexcept           { return getCPUInformation().hasSSE42; }
bool SystemStats::hasAVX() noexcept             { return getCPUInformation().hasAVX; }
bool SystemStats::hasAVX2() noexcept            { return getCPUInformation().hasAVX2; }
bool SystemStats::hasAVX512F() noexcept         { return getCPUInformation().hasAVX512F; }
bool SystemStats::hasAVX512BW() noexcept        { return getCPUInformation().hasAVX512BW; }
bool SystemStats::hasAVX512CD() noexcept        { return getCPUInformation().hasAVX512CD; }
bool SystemStats::hasAVX512DQ() noexcept        { return getCPUInformation().hasAVX512DQ; }
bool SystemStats::hasAVX512ER() noexcept        { return getCPUInformation().hasAVX512ER; }
bool SystemStats::hasAVX512IFMA() noexcept      { return getCPUInformation().hasAVX512IFMA; }
bool SystemStats::hasAVX512PF() noexcept        { return getCPUInformation().hasAVX512PF; }
bool SystemStats::hasAVX512VBMI() noexcept      { return getCPUInformation().hasAVX512VBMI; }
bool SystemStats::hasAVX512VL() noexcept        { return getCPUInformation().hasAVX512VL; }
bool SystemStats::hasAVX512VPOPCNTDQ() noexcept { return getCPUInformation().hasAVX512VPOPCNTDQ; }
bool SystemStats::hasNeon() noexcept            { return getCPUInformation().hasNeon; }


//==============================================================================
String SystemStats::getStackBacktrace()
{
    String result;

   #if STL_ANDROID || STL_MINGW || STL_WASM
    jassertfalse; // sorry, not implemented yet!

   #elif STL_WINDOWS
    HANDLE process = GetCurrentProcess();
    SymInitialize (process, nullptr, TRUE);

    void* stack[128];
    int frames = (int) CaptureStackBackTrace (0, numElementsInArray (stack), stack, nullptr);

    HeapBlock<SYMBOL_INFO> symbol;
    symbol.calloc (sizeof (SYMBOL_INFO) + 256, 1);
    symbol->MaxNameLen = 255;
    symbol->SizeOfStruct = sizeof (SYMBOL_INFO);

    for (int i = 0; i < frames; ++i)
    {
        DWORD64 displacement = 0;

        if (SymFromAddr (process, (DWORD64) stack[i], &displacement, symbol))
        {
            result << i << ": ";

            IMAGEHLP_MODULE64 moduleInfo;
            zerostruct (moduleInfo);
            moduleInfo.SizeOfStruct = sizeof (moduleInfo);

            if (::SymGetModuleInfo64 (process, symbol->ModBase, &moduleInfo))
                result << moduleInfo.ModuleName << ": ";

            result << symbol->Name << " + 0x" << String::toHexString ((int64) displacement) << newLine;
        }
    }

   #else

#ifndef __ANDROID__
    void* stack[128];
    auto frames = backtrace (stack, numElementsInArray (stack));
    char** frameStrings = backtrace_symbols (stack, frames);

    for (auto i = (decltype (frames)) 0; i < frames; ++i)
        result << frameStrings[i] << newLine;

    ::free (frameStrings);
#endif
   #endif

    return result;
}

//==============================================================================
#if ! STL_WASM

static SystemStats::CrashHandlerFunction globalCrashHandler = nullptr;

#if STL_WINDOWS
static LONG WINAPI handleCrash (LPEXCEPTION_POINTERS ep)
{
    globalCrashHandler (ep);
    return EXCEPTION_EXECUTE_HANDLER;
}
#else
static void handleCrash (int signum)
{
    globalCrashHandler ((void*) (pointer_sized_int) signum);
    ::kill (getpid(), SIGKILL);
}

int stl_siginterrupt (int sig, int flag);
#endif

void SystemStats::setApplicationCrashHandler (CrashHandlerFunction handler)
{
    jassert (handler != nullptr); // This must be a valid function.
    globalCrashHandler = handler;

   #if STL_WINDOWS
    SetUnhandledExceptionFilter (handleCrash);
   #else
    const int signals[] = { SIGFPE, SIGILL, SIGSEGV, SIGBUS, SIGABRT, SIGSYS };

    for (int i = 0; i < numElementsInArray (signals); ++i)
    {
        ::signal (signals[i], handleCrash);
        stl_siginterrupt (signals[i], 1);
    }
   #endif
}

#endif

bool SystemStats::isRunningInAppExtensionSandbox() noexcept
{
   #if STL_MAC || STL_IOS
    static bool isRunningInAppSandbox = [&]
    {
        File bundle = File::getSpecialLocation (File::invokedExecutableFile).getParentDirectory();

       #if STL_MAC
        bundle = bundle.getParentDirectory().getParentDirectory();
       #endif

        if (bundle.isDirectory())
            return bundle.getFileExtension() == ".appex";

        return false;
    }();

    return isRunningInAppSandbox;
   #else
    return false;
   #endif
}


} // namespace stl


