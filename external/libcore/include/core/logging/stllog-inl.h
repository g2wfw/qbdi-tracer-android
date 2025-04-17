// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#ifndef STL_LOG_HEADER_ONLY
#    include "stllog.h"
#endif

#include "common.h"
#include "pattern_formatter.h"

namespace stl {

STL_LOG_INLINE void initialize_logger(std::shared_ptr<logger> logger)
{
    details::registry::instance().initialize_logger(std::move(logger));
}

STL_LOG_INLINE std::shared_ptr<logger> get(const std::string &name)
{
    return details::registry::instance().get(name);
}

STL_LOG_INLINE void set_formatter(std::unique_ptr<stl::formatter> formatter)
{
    details::registry::instance().set_formatter(std::move(formatter));
}

STL_LOG_INLINE void set_pattern(std::string pattern, pattern_time_type time_type)
{
    set_formatter(std::unique_ptr<stl::formatter>(new pattern_formatter(std::move(pattern), time_type)));
}

STL_LOG_INLINE void enable_backtrace(size_t n_messages)
{
    details::registry::instance().enable_backtrace(n_messages);
}

STL_LOG_INLINE void disable_backtrace()
{
    details::registry::instance().disable_backtrace();
}

STL_LOG_INLINE void dump_backtrace()
{
    default_logger_raw()->dump_backtrace();
}

STL_LOG_INLINE level::level_enum get_level()
{
    return default_logger_raw()->level();
}

STL_LOG_INLINE bool should_log(level::level_enum log_level)
{
    return default_logger_raw()->should_log(log_level);
}

STL_LOG_INLINE void set_level(level::level_enum log_level)
{
    details::registry::instance().set_level(log_level);
}

STL_LOG_INLINE void flush_on(level::level_enum log_level)
{
    details::registry::instance().flush_on(log_level);
}

STL_LOG_INLINE void set_error_handler(void (*handler)(const std::string &msg))
{
    details::registry::instance().set_error_handler(handler);
}

STL_LOG_INLINE void register_logger(std::shared_ptr<logger> logger)
{
    details::registry::instance().register_logger(std::move(logger));
}

STL_LOG_INLINE void apply_all(const std::function<void(std::shared_ptr<logger>)> &fun)
{
    details::registry::instance().apply_all(fun);
}

STL_LOG_INLINE void drop(const std::string &name)
{
    details::registry::instance().drop(name);
}

STL_LOG_INLINE void drop_all()
{
    details::registry::instance().drop_all();
}

STL_LOG_INLINE void shutdown()
{
    details::registry::instance().shutdown();
}

STL_LOG_INLINE void set_automatic_registration(bool automatic_registration)
{
    details::registry::instance().set_automatic_registration(automatic_registration);
}

STL_LOG_INLINE std::shared_ptr<stl::logger> default_logger()
{
    return details::registry::instance().default_logger();
}

STL_LOG_INLINE stl::logger *default_logger_raw()
{
    return details::registry::instance().get_default_raw();
}

STL_LOG_INLINE void set_default_logger(std::shared_ptr<stl::logger> default_logger)
{
    details::registry::instance().set_default_logger(std::move(default_logger));
}

STL_LOG_INLINE void apply_logger_env_levels(std::shared_ptr<logger> logger)
{
    details::registry::instance().apply_logger_env_levels(std::move(logger));
}

} // namespace stl
