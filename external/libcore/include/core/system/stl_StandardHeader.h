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
/** Current STL version number.

    See also SystemStats::getSTLVersion() for a string version.
*/
#define STL_MAJOR_VERSION      7
#define STL_MINOR_VERSION      0
#define STL_BUILDNUMBER        8

/** Current STL version number.

    Bits 16 to 32 = major version.
    Bits 8 to 16 = minor version.
    Bits 0 to 8 = point release.

    See also SystemStats::getSTLVersion() for a string version.
*/
#define STL_VERSION   ((STL_MAJOR_VERSION << 16) + (STL_MINOR_VERSION << 8) + STL_BUILDNUMBER)

#if !DOXYGEN
#define STL_VERSION_ID \
    [[maybe_unused]] volatile auto juceVersionId = "stl_version_" STL_STRINGIFY(STL_MAJOR_VERSION) "_" STL_STRINGIFY(STL_MINOR_VERSION) "_" STL_STRINGIFY(STL_BUILDNUMBER);
#endif

//==============================================================================
#include <algorithm>
#include <array>
#include <atomic>
#include <cmath>
#include <condition_variable>
#include <cstddef>
#include <functional>
#include <future>
#include <iomanip>
#include <iostream>
#include <limits>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <numeric>
#include <optional>
#include <queue>
#include <set>
#include <sstream>
#include <string_view>
#include <thread>
#include <typeindex>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <variant>
#include <vector>

//==============================================================================
#include "stl_CompilerSupport.h"
#include "stl_CompilerWarnings.h"
#include "stl_PlatformDefs.h"

//==============================================================================
// Now we'll include some common OS headers..
STL_BEGIN_IGNORE_WARNINGS_MSVC (4514 4245 4100)

#if STL_MSVC
#include <intrin.h>
#endif


#if STL_MAC || STL_IOS
#include <libkern/OSAtomic.h>
#include <libkern/OSByteOrder.h>
#include <xlocale.h>
#include <signal.h>
#endif

#if STL_LINUX || STL_BSD

#include <cstring>
#include <signal.h>

#if __INTEL_COMPILER
#if __ia64__
#include <ia64intrin.h>
#else
#include <ia32intrin.h>
#endif
#endif
#endif

#if STL_MSVC && STL_DEBUG
#include <crtdbg.h>
#endif

STL_END_IGNORE_WARNINGS_MSVC

#if STL_MINGW
#include <cstring>
#include <sys/types.h>
#endif

#if STL_ANDROID
#include <cstring>
#include <byteswap.h>
#endif

// undef symbols that are sometimes set by misguided 3rd-party headers..
#undef TYPE_BOOL
#undef max
#undef min
#undef major
#undef minor
#undef KeyPress

//==============================================================================
// DLL building settings on Windows


//==============================================================================


#if STL_MSVC && STL_DLL_BUILD
#define STL_PUBLIC_IN_DLL_BUILD(declaration)  public: declaration; private:
#else
#define STL_PUBLIC_IN_DLL_BUILD(declaration)  declaration;
#endif

/** This macro is added to all STL public function declarations. */
#define STL_PUBLIC_FUNCTION        STL_CALLTYPE

#ifndef DOXYGEN
#define STL_NAMESPACE juce  // This old macro is deprecated: you should just use the juce namespace directly.
#endif
