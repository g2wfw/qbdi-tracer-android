//
// Copyright(c) 2016-2018 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#pragma once

//
// Include a bundled header-only copy of fmtlib or an external one.
// By default spdlog include its own copy.
//

#if defined(STL_LOG_USE_STD_FORMAT) // STL_LOG_USE_STD_FORMAT is defined - use std::format
#    include <format>
#elif !defined(STL_LOG_STL_FMT_EXTERNAL)
#    if !defined(STL_LOG_COMPILED_LIB) && !defined(STL_FMT_HEADER_ONLY)
#        define STL_FMT_HEADER_ONLY
#    endif
#    ifndef STL_FMT_USE_WINDOWS_H
#        define STL_FMT_USE_WINDOWS_H 0
#    endif
// enable the 'n' flag in for backward compatibility with fmt 6.x
#    define STL_FMT_DEPRECATED_N_SPECIFIER
// enable ostream formatting for backward compatibility with fmt 8.x
#    define STL_FMT_DEPRECATED_OSTREAM

#    include "bundled/core.h"
#    include "bundled/format.h"

#else // STL_LOG_STL_FMT_EXTERNAL is defined - use external fmtlib
#    include <fmt/core.h>
#    include <fmt/format.h>
#endif
