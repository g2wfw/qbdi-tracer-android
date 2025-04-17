// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#ifndef STL_LOG_HEADER_ONLY
#    include "log_msg.h"
#endif

#include "os.h"

namespace stl {
namespace details {

STL_LOG_INLINE log_msg::log_msg(stl::log_clock::time_point log_time, stl::source_loc loc, string_view_t a_logger_name,
    stl::level::level_enum lvl, stl::string_view_t msg)
    : logger_name(a_logger_name)
    , level(lvl)
    , time(log_time)
#ifndef STL_LOG_NO_THREAD_ID
    , thread_id(os::thread_id())
#endif
    , source(loc)
    , payload(msg)
{}

STL_LOG_INLINE log_msg::log_msg(
    stl::source_loc loc, string_view_t a_logger_name, stl::level::level_enum lvl, stl::string_view_t msg)
    : log_msg(os::now(), loc, a_logger_name, lvl, msg)
{}

STL_LOG_INLINE log_msg::log_msg(string_view_t a_logger_name, stl::level::level_enum lvl, stl::string_view_t msg)
    : log_msg(os::now(), source_loc{}, a_logger_name, lvl, msg)
{}

} // namespace details
} // namespace stl
