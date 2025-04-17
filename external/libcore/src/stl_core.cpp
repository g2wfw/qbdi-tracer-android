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

#ifdef STL_CORE_H_INCLUDED
/* When you add this cpp file to your project, you mustn't include it in a file where you've
   already included any other headers - just put it inside a file on its own, possibly with your config
   flags preceding it, but don't include anything else. That also includes avoiding any automatic prefix
   header files that the compiler may be using.
*/
#error "Incorrect use of STL cpp file"
#endif
#include "text/stl_String.h"
#include "text/stl_StringRef.h"
#include "memory/stl_LeakedObjectDetector.h"
#include "memory/stl_ContainerDeletePolicy.h"
#include "memory/stl_HeapBlock.h"
#include "memory/stl_MemoryBlock.h"
#include "memory/stl_ReferenceCountedObject.h"
#include "memory/stl_ScopedPointer.h"
#include "memory/stl_OptionalScopedPointer.h"
#include "containers/stl_Optional.h"
#include "containers/stl_ScopedValueSetter.h"
#include "memory/stl_Singleton.h"
#include "memory/stl_WeakReference.h"
#include "threads/stl_ScopedLock.h"
#include "threads/stl_CriticalSection.h"
#include "maths/stl_Range.h"
#include "maths/stl_NormalisableRange.h"
#include "maths/stl_StatisticsAccumulator.h"
#include "containers/stl_ElementComparator.h"
#include "containers/stl_ArrayAllocationBase.h"
#include "containers/stl_ArrayBase.h"
#include "containers/stl_Array.h"
#include "containers/stl_LinkedListPointer.h"
#include "misc/stl_ScopeGuard.h"
#include "containers/stl_ListenerList.h"
#include "containers/stl_OwnedArray.h"
#include "containers/stl_ReferenceCountedArray.h"
#include "containers/stl_SortedSet.h"
#include "containers/stl_SparseSet.h"
#include "containers/stl_AbstractFifo.h"
#include "containers/stl_SingleThreadedAbstractFifo.h"
#include "text/stl_NewLine.h"
#include "text/stl_StringPool.h"
#include "text/stl_Identifier.h"
#include "text/stl_StringArray.h"
#include "system/stl_SystemStats.h"
#include "memory/stl_HeavyweightLeakedObjectDetector.h"
#include "text/stl_StringPairArray.h"
#include "text/stl_TextDiff.h"
#include "text/stl_LocalisedStrings.h"
#include "text/stl_Base64.h"
#include "misc/stl_Functional.h"
#include "containers/stl_Span.h"
#include "misc/stl_Result.h"
#include "misc/stl_Uuid.h"
#include "misc/stl_ConsoleApplication.h"
#include "containers/stl_Variant.h"
#include "containers/stl_NamedValueSet.h"
#include "containers/stl_DynamicObject.h"
#include "containers/stl_HashMap.h"
#include "containers/stl_FixedSizeFunction.h"
#include "time/stl_RelativeTime.h"
#include "time/stl_Time.h"
#include "streams/stl_InputStream.h"
#include "streams/stl_OutputStream.h"
#include "streams/stl_BufferedInputStream.h"
#include "streams/stl_MemoryInputStream.h"
#include "streams/stl_MemoryOutputStream.h"
#include "streams/stl_SubregionStream.h"
#include "streams/stl_InputSource.h"
#include "files/stl_File.h"
#include "files/stl_DirectoryIterator.h"
#include "files/stl_RangedDirectoryIterator.h"
#include "files/stl_FileInputStream.h"
#include "files/stl_FileOutputStream.h"
#include "files/stl_FileSearchPath.h"
#include "files/stl_MemoryMappedFile.h"
#include "files/stl_TemporaryFile.h"
#include "files/stl_FileFilter.h"
#include "files/stl_WildcardFileFilter.h"
#include "streams/stl_FileInputSource.h"
#include "maths/stl_BigInteger.h"
#include "maths/stl_Expression.h"
#include "maths/stl_Random.h"
#include "misc/stl_WindowsRegistry.h"
#include "threads/stl_ChildProcess.h"
#include "threads/stl_DynamicLibrary.h"
#include "threads/stl_InterProcessLock.h"
#include "threads/stl_Process.h"
#include "threads/stl_SpinLock.h"
#include "threads/stl_WaitableEvent.h"
#include "threads/stl_Thread.h"
#include "threads/stl_HighResolutionTimer.h"
#include "threads/stl_ThreadLocalValue.h"
#include "threads/stl_ThreadPool.h"
#include "threads/stl_TimeSliceThread.h"
#include "threads/stl_ReadWriteLock.h"
#include "threads/stl_ScopedReadLock.h"
#include "threads/stl_ScopedWriteLock.h"
#include "time/stl_PerformanceCounter.h"
#include "containers/stl_PropertySet.h"
#include "memory/stl_SharedResourcePointer.h"
#include "memory/stl_AllocationHooks.h"
#include "memory/stl_Reservoir.h"
#include "json/stl_JSON.h"
#include "json/stl_JSONUtils.h"
#undef check
//==============================================================================
#include "native/stl_ThreadPriorities_native.h"
#include "native/stl_PlatformTimerListener.h"
#include "json/stl_JSON.cpp"
#include "json/stl_JSONUtils.cpp"
#include "containers/stl_DynamicObject.cpp"

//==============================================================================
#if !STL_WINDOWS
#include "native/stl_SharedCode_posix.h"
#endif

//==============================================================================
#if STL_MAC || STL_IOS
#include "native/stl_Files_mac.mm"
//#include "native/stl_Network_mac.mm"
#include "native/stl_Strings_mac.mm"
#include "native/stl_SharedCode_intel.h"
#include "native/stl_SystemStats_mac.mm"
#include "native/stl_Threads_mac.mm"
#include "native/stl_PlatformTimer_generic.cpp"

//==============================================================================
#elif STL_WINDOWS

#include "native/stl_Files_windows.cpp"
#include "native/stl_Network_windows.cpp"
#include "native/stl_Registry_windows.cpp"
#include "native/stl_SystemStats_windows.cpp"
#include "native/stl_Threads_windows.cpp"
#include "native/stl_PlatformTimer_windows.cpp"

//==============================================================================
#elif STL_LINUX
#include "native/stl_CommonFile_linux.cpp"
#include "native/stl_Files_linux.cpp"

#if STL_USE_CURL
#include "native/stl_Network_curl.cpp"
#endif
#include "native/stl_SystemStats_linux.cpp"
#include "native/stl_Threads_linux.cpp"
#include "native/stl_PlatformTimer_generic.cpp"

//==============================================================================
#elif STL_BSD
#include "native/stl_CommonFile_linux.cpp"
#include "native/stl_Files_linux.cpp"
#include "native/stl_Network_linux.cpp"
#if STL_USE_CURL
#include "native/stl_Network_curl.cpp"
#endif
#include "native/stl_SharedCode_intel.h"
#include "native/stl_SystemStats_linux.cpp"
#include "native/stl_Threads_linux.cpp"
#include "native/stl_PlatformTimer_generic.cpp"

//==============================================================================
#elif STL_ANDROID
#include "native/stl_CommonFile_linux.cpp"
#include "native/stl_JNIHelpers_android.cpp"
#include "native/stl_Files_android.cpp"

#include "native/stl_PlatformTimer_generic.cpp"

//==============================================================================
#elif STL_WASM
#include "native/stl_SystemStats_wasm.cpp"
#include "native/stl_PlatformTimer_generic.cpp"
#endif

#include "files/stl_common_MimeTypes.h"
#include "files/stl_common_MimeTypes.cpp"
#include "threads/stl_HighResolutionTimer.cpp"
#include "threads/stl_WaitableEvent.cpp"

#if !STL_WASM

#include "threads/stl_ChildProcess.cpp"

#endif


//==============================================================================
namespace stl {
/*
    As the very long class names here try to explain, the purpose of this code is to cause
    a linker error if not all of your compile units are consistent in the options that they
    enable before including STL headers. The reason this is important is that if you have
    two cpp files, and one includes the juce headers with debug enabled, and the other doesn't,
    then each will be generating code with different memory layouts for the classes, and
    you'll get subtle and hard-to-track-down memory corruption bugs!
*/

}
