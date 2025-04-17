// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

// spdlog main header file.
// see example.cpp for usage example

#ifndef STL_LOG_H
#define STL_LOG_H

#pragma once

#include "common.h"
#include "details/registry.h"
#include "logger.h"
#include "version.h"
#include "details/synchronous_factory.h"
#include "check.h"
#include <chrono>
#include <functional>
#include <memory>
#include <string>

namespace stl {

    using default_factory = synchronous_factory;

// Create and register a logger with a templated sink type
// The logger's level, formatter and flush level will be set according the
// global settings.
//
// Example:
//   stl::create<daily_file_sink_st>("logger_name", "dailylog_filename", 11, 59);
    template<typename Sink, typename... SinkArgs>
    inline std::shared_ptr<stl::logger> create(std::string logger_name, SinkArgs &&...sink_args) {
        return default_factory::create<Sink>(std::move(logger_name), std::forward<SinkArgs>(sink_args)...);
    }

// Initialize and register a logger,
// formatter and flush level will be set according the global settings.
//
// Useful for initializing manually created loggers with the global settings.
//
// Example:
//   auto mylogger = std::make_shared<stl::logger>("mylogger", ...);
//   stl::initialize_logger(mylogger);
    STL_LOG_API void initialize_logger(std::shared_ptr<logger> logger);

// Return an existing logger or nullptr if a logger with such name doesn't
// exist.
// example: stl::get("my_logger")->info("hello {}", "world");
    STL_LOG_API std::shared_ptr<logger> get(const std::string &name);

// Set global formatter. Each sink in each logger will get a clone of this object
    STL_LOG_API void set_formatter(std::unique_ptr<stl::formatter> formatter);

// Set global format string.
// example: stl::set_pattern("%Y-%m-%d %H:%M:%S.%e %l : %v");
    STL_LOG_API void set_pattern(std::string pattern, pattern_time_type time_type = pattern_time_type::local);

// enable global backtrace support
    STL_LOG_API void enable_backtrace(size_t n_messages);

// disable global backtrace support
    STL_LOG_API void disable_backtrace();

// call dump backtrace on default logger
    STL_LOG_API void dump_backtrace();

// Get global logging level
    STL_LOG_API level::level_enum get_level();

// Set global logging level
    STL_LOG_API void set_level(level::level_enum log_level);

// Determine whether the default logger should log messages with a certain level
    STL_LOG_API bool should_log(level::level_enum lvl);

// Set global flush level
    STL_LOG_API void flush_on(level::level_enum log_level);

// Start/Restart a periodic flusher thread
// Warning: Use only if all your loggers are thread safe!
    template<typename Rep, typename Period>
    inline void flush_every(std::chrono::duration<Rep, Period> interval) {
        details::registry::instance().flush_every(interval);
    }

// Set global error handler
    STL_LOG_API void set_error_handler(void (*handler)(const std::string &msg));

// Register the given logger with the given name
    STL_LOG_API void register_logger(std::shared_ptr<logger> logger);

// Apply a user defined function on all registered loggers
// Example:
// stl::apply_all([&](std::shared_ptr<stl::logger> l) {l->flush();});
    STL_LOG_API void apply_all(const std::function<void(std::shared_ptr<logger>)> &fun);

// Drop the reference to the given logger
    STL_LOG_API void drop(const std::string &name);

// Drop all references from the registry
    STL_LOG_API void drop_all();

// stop any running threads started by spdlog and clean registry loggers
    STL_LOG_API void shutdown();

// Automatic registration of loggers when using stl::create() or stl::create_async
    STL_LOG_API void set_automatic_registration(bool automatic_registration);

// API for using default logger (stdout_color_mt),
// e.g: stl::info("Message {}", 1);
//
// The default logger object can be accessed using the stl::default_logger():
// For example, to add another sink to it:
// stl::default_logger()->sinks().push_back(some_sink);
//
// The default logger can replaced using stl::set_default_logger(new_logger).
// For example, to replace it with a file logger.
//
// IMPORTANT:
// The default API is thread safe (for _mt loggers), but:
// set_default_logger() *should not* be used concurrently with the default API.
// e.g do not call set_default_logger() from one thread while calling stl::info() from another.

    STL_LOG_API std::shared_ptr<stl::logger> default_logger();

    STL_LOG_API stl::logger *default_logger_raw();

    STL_LOG_API void set_default_logger(std::shared_ptr<stl::logger> default_logger);

// Initialize logger level based on environment configs.
//
// Useful for applying STL_LOG_LEVEL to manually created loggers.
//
// Example:
//   auto mylogger = std::make_shared<stl::logger>("mylogger", ...);
//   stl::apply_logger_env_levels(mylogger);
    STL_LOG_API void apply_logger_env_levels(std::shared_ptr<logger> logger);

    template<typename... Args>
    inline void log(source_loc source, level::level_enum lvl, format_string_t<Args...> fmt, Args &&...args) {
        default_logger_raw()->log(source, lvl, fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    inline void log(level::level_enum lvl, format_string_t<Args...> fmt, Args &&...args) {
        default_logger_raw()->log(source_loc{}, lvl, fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    __attribute__((always_inline)) inline bool fatal(source_loc source, format_string_t<Args...> fmt, Args &&...args) {
        default_logger_raw()->log(source, level::level_enum::fatal, fmt, std::forward<Args>(args)...);
        abort();
        return false;
    }
    template<typename... Args>
    __attribute__((always_inline)) inline bool fatal( format_string_t<Args...> fmt, Args &&...args) {
        default_logger_raw()->log(source_loc{}, level::level_enum::fatal, fmt, std::forward<Args>(args)...);
        abort();
        return false;
    }
    template<typename... Args>
    inline void trace(format_string_t<Args...> fmt, Args &&...args) {
        default_logger_raw()->trace(fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    inline void debug(format_string_t<Args...> fmt, Args &&...args) {
        default_logger_raw()->debug(fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    inline void info(format_string_t<Args...> fmt, Args &&...args) {
        default_logger_raw()->info(fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    inline void warn(format_string_t<Args...> fmt, Args &&...args) {
        default_logger_raw()->warn(fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    inline void error(format_string_t<Args...> fmt, Args &&...args) {
        default_logger_raw()->error(fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    inline void critical(format_string_t<Args...> fmt, Args &&...args) {
        default_logger_raw()->critical(fmt, std::forward<Args>(args)...);
    }

    template<typename T>
    inline void log(source_loc source, level::level_enum lvl, const T &msg) {
        default_logger_raw()->log(source, lvl, msg);
    }

    template<typename T>
    __attribute__((always_inline)) inline void fatal(const T &msg) {
        default_logger_raw()->log(source_loc(__FILE__, __LINE__, __FUNCTION__), level::level_enum::fatal, msg);
        abort();
    }

    template<typename T>
    __attribute__((always_inline)) inline bool fatal(source_loc source, const T &msg) {
        default_logger_raw()->log(source, level::level_enum::fatal, msg);
        abort();
        return false;
    }

    template<typename T>
    inline void log(level::level_enum lvl, const T &msg) {
        default_logger_raw()->log(lvl, msg);
    }

#ifdef STL_LOG_WCHAR_TO_UTF8_SUPPORT
    template<typename... Args>
    inline void log(source_loc source, level::level_enum lvl, wformat_string_t<Args...> fmt, Args &&...args)
    {
        default_logger_raw()->log(source, lvl, fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    inline void log(level::level_enum lvl, wformat_string_t<Args...> fmt, Args &&...args)
    {
        default_logger_raw()->log(source_loc{}, lvl, fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    inline void trace(wformat_string_t<Args...> fmt, Args &&...args)
    {
        default_logger_raw()->trace(fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    inline void debug(wformat_string_t<Args...> fmt, Args &&...args)
    {
        default_logger_raw()->debug(fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    inline void info(wformat_string_t<Args...> fmt, Args &&...args)
    {
        default_logger_raw()->info(fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    inline void warn(wformat_string_t<Args...> fmt, Args &&...args)
    {
        default_logger_raw()->warn(fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    inline void error(wformat_string_t<Args...> fmt, Args &&...args)
    {
        default_logger_raw()->error(fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    inline void critical(wformat_string_t<Args...> fmt, Args &&...args)
    {
        default_logger_raw()->critical(fmt, std::forward<Args>(args)...);
    }
#endif

    template<typename T>
    inline void trace(const T &msg) {
        default_logger_raw()->trace(msg);
    }

    template<typename T>
    inline void debug(const T &msg) {
        default_logger_raw()->debug(msg);
    }

    template<typename T>
    inline void info(const T &msg) {
        default_logger_raw()->info(msg);
    }

    template<typename T>
    inline void warn(const T &msg) {
        default_logger_raw()->warn(msg);
    }

    template<typename T>
    inline void error(const T &msg) {
        default_logger_raw()->error(msg);
    }

    template<typename T>
    inline void critical(const T &msg) {
        default_logger_raw()->critical(msg);
    }

} // namespace stl

//
// enable/disable log calls at compile time according to global level.
//
// define STL_LOG_ACTIVE_LEVEL to one of those (before including spdlog.h):
// STL_LOG_LEVEL_TRACE,
// STL_LOG_LEVEL_DEBUG,
// STL_LOG_LEVEL_INFO,
// STL_LOG_LEVEL_WARN,
// STL_LOG_LEVEL_ERROR,
// STL_LOG_LEVEL_CRITICAL,
// STL_LOG_LEVEL_OFF
//

#ifndef STL_LOG_NO_SOURCE_LOC
#    define STL_LOG_LOGGER_CALL(logger, level, ...) (logger)->log(stl::source_loc{__FILE__, __LINE__, STL_LOG_FUNCTION}, level, __VA_ARGS__)
#else
#    define STL_LOG_LOGGER_CALL(logger, level, ...) (logger)->log(stl::source_loc{}, level, __VA_ARGS__)
#endif

#if STL_LOG_ACTIVE_LEVEL <= STL_LOG_LEVEL_TRACE
#    define STL_LOG_LOGGER_TRACE(logger, ...) STL_LOG_LOGGER_CALL(logger, stl::level::trace, __VA_ARGS__)
#    define STL_LOG_TRACE(...) STL_LOG_LOGGER_TRACE(stl::default_logger_raw(), __VA_ARGS__)
#else
#    define STL_LOG_LOGGER_TRACE(logger, ...) (void)0
#    define STL_LOG_TRACE(...) (void)0
#endif

#if STL_LOG_ACTIVE_LEVEL <= STL_LOG_LEVEL_DEBUG
#    define STL_LOG_LOGGER_DEBUG(logger, ...) STL_LOG_LOGGER_CALL(logger, stl::level::debug, __VA_ARGS__)
#    define STL_LOG_DEBUG(...) STL_LOG_LOGGER_DEBUG(stl::default_logger_raw(), __VA_ARGS__)
#else
#    define STL_LOG_LOGGER_DEBUG(logger, ...) (void)0
#    define STL_LOG_DEBUG(...) (void)0
#endif

#if STL_LOG_ACTIVE_LEVEL <= STL_LOG_LEVEL_INFO
#    define STL_LOG_LOGGER_INFO(logger, ...) STL_LOG_LOGGER_CALL(logger, stl::level::info, __VA_ARGS__)
#    define STL_LOG_INFO(...) STL_LOG_LOGGER_INFO(stl::default_logger_raw(), __VA_ARGS__)
#else
#    define STL_LOG_LOGGER_INFO(logger, ...) (void)0
#    define STL_LOG_INFO(...) (void)0
#endif

#if STL_LOG_ACTIVE_LEVEL <= STL_LOG_LEVEL_WARN
#    define STL_LOG_LOGGER_WARN(logger, ...) STL_LOG_LOGGER_CALL(logger, stl::level::warn, __VA_ARGS__)
#    define STL_LOG_WARN(...) STL_LOG_LOGGER_WARN(stl::default_logger_raw(), __VA_ARGS__)
#else
#    define STL_LOG_LOGGER_WARN(logger, ...) (void)0
#    define STL_LOG_WARN(...) (void)0
#endif

#if STL_LOG_ACTIVE_LEVEL <= STL_LOG_LEVEL_ERROR
#    define STL_LOG_LOGGER_ERROR(logger, ...) STL_LOG_LOGGER_CALL(logger, stl::level::err, __VA_ARGS__)
#    define STL_LOG_ERROR(...) STL_LOG_LOGGER_ERROR(stl::default_logger_raw(), __VA_ARGS__)
#else
#    define STL_LOG_LOGGER_ERROR(logger, ...) (void)0
#    define STL_LOG_ERROR(...) (void)0
#endif

#if STL_LOG_ACTIVE_LEVEL <= STL_LOG_LEVEL_CRITICAL
#    define STL_LOG_LOGGER_CRITICAL(logger, ...) STL_LOG_LOGGER_CALL(logger, stl::level::critical, __VA_ARGS__)
#    define STL_LOG_CRITICAL(...) STL_LOG_LOGGER_CRITICAL(stl::default_logger_raw(), __VA_ARGS__)
#else
#    define STL_LOG_LOGGER_CRITICAL(logger, ...) (void)0
#    define STL_LOG_CRITICAL(...) (void)0
#endif

#ifdef STL_LOG_HEADER_ONLY

#    include "stllog-inl.h"

#endif

#endif // STL_LOG_H
