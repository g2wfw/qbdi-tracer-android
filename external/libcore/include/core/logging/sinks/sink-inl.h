// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#ifndef STL_LOG_HEADER_ONLY
#    include "sink.h"
#endif

#include "../common.h"

STL_LOG_INLINE bool stl::sinks::sink::should_log(stl::level::level_enum msg_level) const
{
    return msg_level >= level_.load(std::memory_order_relaxed);
}

STL_LOG_INLINE void stl::sinks::sink::set_level(level::level_enum log_level)
{
    level_.store(log_level, std::memory_order_relaxed);
}

STL_LOG_INLINE stl::level::level_enum stl::sinks::sink::level() const
{
    return static_cast<stl::level::level_enum>(level_.load(std::memory_order_relaxed));
}
