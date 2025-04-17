// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#ifndef STL_LOG_HEADER_ONLY
#    include "common.h"
#endif

#include <algorithm>
#include <iterator>

namespace stl {
namespace level {

#if __cplusplus >= 201703L
constexpr
#endif
    static string_view_t level_string_views[] STL_LOG_LEVEL_NAMES;

static const char *short_level_names[] STL_LOG_SHORT_LEVEL_NAMES;

STL_LOG_INLINE const string_view_t &to_string_view(stl::level::level_enum l) STL_LOG_NOEXCEPT
{
    return level_string_views[l];
}

STL_LOG_INLINE const char *to_short_c_str(stl::level::level_enum l) STL_LOG_NOEXCEPT
{
    return short_level_names[l];
}

STL_LOG_INLINE stl::level::level_enum from_str(const std::string &name) STL_LOG_NOEXCEPT
{
    auto it = std::find(std::begin(level_string_views), std::end(level_string_views), name);
    if (it != std::end(level_string_views))
        return static_cast<level::level_enum>(std::distance(std::begin(level_string_views), it));

    // check also for "warn" and "err" before giving up..
    if (name == "warn")
    {
        return level::warn;
    }
    if (name == "err")
    {
        return level::err;
    }
    return level::off;
}
} // namespace level

STL_LOG_INLINE spdlog_ex::spdlog_ex(std::string msg)
    : msg_(std::move(msg))
{}

STL_LOG_INLINE spdlog_ex::spdlog_ex(const std::string &msg, int last_errno)
{
#ifdef STL_LOG_USE_STD_FORMAT
    msg_ = std::system_error(std::error_code(last_errno, std::generic_category()), msg).what();
#else
    memory_buf_t outbuf;
    fmt::format_system_error(outbuf, last_errno, msg.c_str());
    msg_ = fmt::to_string(outbuf);
#endif
}

STL_LOG_INLINE const char *spdlog_ex::what() const STL_LOG_NOEXCEPT
{
    return msg_.c_str();
}

STL_LOG_INLINE void throw_spdlog_ex(const std::string &msg, int last_errno)
{
    STL_LOG_THROW(spdlog_ex(msg, last_errno));
}

STL_LOG_INLINE void throw_spdlog_ex(std::string msg)
{
    STL_LOG_THROW(spdlog_ex(std::move(msg)));
}

} // namespace stl
