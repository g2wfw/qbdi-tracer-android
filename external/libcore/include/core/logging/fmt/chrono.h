//
// Copyright(c) 2016 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#pragma once
//
// include bundled or external copy of fmtlib's chrono support
//

#if !defined(STL_LOG_USE_STD_FORMAT)
#    if !defined(STL_LOG_STL_FMT_EXTERNAL)
#        ifdef STL_LOG_HEADER_ONLY
#            ifndef STL_FMT_HEADER_ONLY
#                define STL_FMT_HEADER_ONLY
#            endif
#        endif
#        include "bundled/chrono.h"
#    else
#        include <fmt/chrono.h>
#    endif
#endif
