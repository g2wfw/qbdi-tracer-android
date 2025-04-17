// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#ifndef STL_LOG_HEADER_ONLY
#    include "stdout_color_sinks.h"
#endif

#include "../logger.h"
#include "../common.h"

namespace stl {

template<typename Factory>
STL_LOG_INLINE std::shared_ptr<logger> stdout_color_mt(const std::string &logger_name, color_mode mode)
{
    return Factory::template create<sinks::stdout_color_sink_mt>(logger_name, mode);
}

template<typename Factory>
STL_LOG_INLINE std::shared_ptr<logger> stdout_color_st(const std::string &logger_name, color_mode mode)
{
    return Factory::template create<sinks::stdout_color_sink_st>(logger_name, mode);
}

template<typename Factory>
STL_LOG_INLINE std::shared_ptr<logger> stderr_color_mt(const std::string &logger_name, color_mode mode)
{
    return Factory::template create<sinks::stderr_color_sink_mt>(logger_name, mode);
}

template<typename Factory>
STL_LOG_INLINE std::shared_ptr<logger> stderr_color_st(const std::string &logger_name, color_mode mode)
{
    return Factory::template create<sinks::stderr_color_sink_st>(logger_name, mode);
}
} // namespace stl
