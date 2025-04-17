/*
 * @author: xiaobai
 * @email: xiaobaiyey@outlook.com
 * @date: 2023/11/15 14:23
 * @version: 1.0
 * @description: 
*/
#include <cstdio>
#include <chrono>

void load_levels_example();

void stdout_logger_example();

void basic_example();

void rotating_example();

void daily_example();

void callback_example();

void async_example();

void binary_example();

void vector_example();

void stopwatch_example();

void trace_example();

void multi_sink_example();

void user_defined_example();

void err_handler_example();


void custom_flags_example();

void file_events_example();

void replace_default_logger_example();

#include <core/logging/stllog.h>


int main2(int, char *[]) {
    // Log levels can be loaded from argv/env using "STL_LEVEL"
    load_levels_example();
    stl::warn("Easy padding in numbers like {:08d}", 12);
    stl::critical("Support for int: {0:d};  hex: {0:x};  oct: {0:o}; bin: {0:b}", 42);
    stl::info("Support for floats {:03.2f}", 1.23456);
    stl::info("Positional args are {1} {0}..", "too", "supported");
    stl::info("{:>8} aligned, {:<8} aligned", "right", "left");

    // Runtime log levels
    stl::set_level(stl::level::info);  // Set global log level to info
    stl::debug("This message should not be displayed!");
    stl::set_level(stl::level::trace);  // Set specific logger's log level
    stl::debug("This message should be displayed..");

    // Customize msg format for all loggers
    stl::set_pattern("[%H:%M:%S %z] [%^%L%$] [thread %t] %v");
    stl::info("This an info message with custom format");
    stl::set_pattern("%+");  // back to default format
    stl::set_level(stl::level::info);

    // Backtrace support
    // Loggers can store in a ring buffer all messages (including debug/trace) for later inspection.
    // When needed, call dump_backtrace() to see what happened:
    stl::enable_backtrace(10);  // create ring buffer with capacity of 10  messages
    for (int i = 0; i < 100; i++) {
        stl::debug("Backtrace message {}", i);  // not logged..
    }
    // e.g. if some error happened:
    stl::dump_backtrace();  // log them now!

    try {
        stdout_logger_example();
        basic_example();
        rotating_example();
        daily_example();
        callback_example();
        async_example();
        binary_example();
        vector_example();
        multi_sink_example();
        user_defined_example();
        err_handler_example();
        trace_example();
        stopwatch_example();

        custom_flags_example();
        file_events_example();
        replace_default_logger_example();

        // Flush all *registered* loggers using a worker thread every 3 seconds.
        // note: registered loggers *must* be thread safe for this to work correctly!
        stl::flush_every(std::chrono::seconds(3));

        // Apply some function on all registered loggers
        stl::apply_all([&](std::shared_ptr<stl::logger> l) { l->info("End of example."); });

        // Release all stl resources, and drop all loggers in the registry.
        // This is optional (only mandatory if using windows + async log).
        stl::shutdown();
    }

        // Exceptions will only be thrown upon failed logger or sink construction (not during logging).
    catch (const stl::spdlog_ex &ex) {
        std::printf("Log initialization failed: %s\n", ex.what());
        return 1;
    }
}

#include "core/logging/sinks/stdout_color_sinks.h"

// or #include "core/logging/sinks/stdout_sinks.h" if no colors needed.
void stdout_logger_example() {
    // Create color multi threaded logger.
    auto console = stl::stdout_color_mt("console");
    // or for stderr:
    // auto console = stl::stderr_color_mt("error-logger");
}

#include "core/logging/sinks/basic_file_sink.h"

void basic_example() {
    // Create basic file logger (not rotated).
    auto my_logger = stl::basic_logger_mt("file_logger", "logs/basic-log.txt", true);
}

#include "core/logging/sinks/rotating_file_sink.h"

void rotating_example() {
    // Create a file rotating logger with 5mb size max and 3 rotated files.
    auto rotating_logger =
            stl::rotating_logger_mt("some_logger_name", "logs/rotating.txt", 1048576 * 5, 3);
}

#include "core/logging/sinks/daily_file_sink.h"

void daily_example() {
    // Create a daily logger - a new file is created every day on 2:30am.
    auto daily_logger = stl::daily_logger_mt("daily_logger", "logs/daily.txt", 2, 30);
}

#include "core/logging/sinks/callback_sink.h"

void callback_example() {
    // Create the logger
    auto logger = stl::callback_logger_mt("custom_callback_logger",
                                          [](const stl::details::log_msg & /*msg*/) {
                                              // do what you need to do with msg
                                          });
}

#include "core/logging/cfg/env.h"

void load_levels_example() {
    // Set the log level to "info" and mylogger to "trace":
    // STL_LEVEL=info,mylogger=trace && ./example
    stl::cfg::load_env_levels();
    // or from command line:
    // ./example STL_LEVEL=info,mylogger=trace
    // #include "core/logging/cfg/argv.h" // for loading levels from argv
    // stl::cfg::load_argv_levels(args, argv);
}

#include "core/logging/async.h"

void async_example() {
    // Default thread pool settings can be modified *before* creating the async logger:
    // stl::init_thread_pool(32768, 1); // queue with max 32k items 1 backing thread.
    auto async_file =
            stl::basic_logger_mt<stl::async_factory>("async_file_logger", "logs/async_log.txt");
    // alternatively:
    // auto async_file =
    // stl::create_async<stl::sinks::basic_file_sink_mt>("async_file_logger",
    // "logs/async_log.txt");

    for (int i = 1; i < 101; ++i) {
        async_file->info("Async message #{}", i);
    }
}

// Log binary data as hex.
// Many types of std::container<char> types can be used.
// Iterator ranges are supported too.
// Format flags:
// {:X} - print in uppercase.
// {:s} - don't separate each byte with space.
// {:p} - don't print the position on each line start.
// {:n} - don't split the output to lines.

#if !defined STL_USE_STD_FORMAT || defined(_MSC_VER)

#include "core/logging/fmt/bin_to_hex.h"

void binary_example() {
    std::vector<char> buf(80);
    for (int i = 0; i < 80; i++) {
        buf.push_back(static_cast<char>(i & 0xff));
    }
    stl::info("Binary example: {}", stl::to_hex(buf));
    stl::info("Another binary example:{:n}",
              stl::to_hex(std::begin(buf), std::begin(buf) + 10));
    // more examples:
    // logger->info("uppercase: {:X}", stl::to_hex(buf));
    // logger->info("uppercase, no delimiters: {:Xs}", stl::to_hex(buf));
    // logger->info("uppercase, no delimiters, no position info: {:Xsp}", stl::to_hex(buf));
    // logger->info("hexdump style: {:a}", stl::to_hex(buf));
    // logger->info("hexdump style, 20 chars per line {:a}", stl::to_hex(buf, 20));
}

#else
void binary_example() {
    // not supported with std::format yet
}
#endif

// Log a vector of numbers
#ifndef STL_USE_STD_FORMAT

#include "core/logging/fmt/ranges.h"

void vector_example() {
    std::vector<int> vec = {1, 2, 3};
    stl::info("Vector example: {}", vec);
}

#else
void vector_example() {}
#endif

// ! DSTL_USE_STD_FORMAT

// Compile time log levels.
// define STL_ACTIVE_LEVEL to required level (e.g. STL_LEVEL_TRACE)
void trace_example() {
    // trace from default logger

}

// stopwatch example
#include "core/logging/stopwatch.h"
#include <thread>

void stopwatch_example() {
    stl::stopwatch sw;
    std::this_thread::sleep_for(std::chrono::milliseconds(123));
    stl::info("Stopwatch: {} seconds", sw);
}


// A logger with multiple sinks (stdout and file) - each with a different format and log level.
void multi_sink_example() {
    auto console_sink = std::make_shared<stl::sinks::stdout_color_sink_mt>();
    console_sink->set_level(stl::level::warn);
    console_sink->set_pattern("[multi_sink_example] [%^%l%$] %v");

    auto file_sink =
            std::make_shared<stl::sinks::basic_file_sink_mt>("logs/multisink.txt", true);
    file_sink->set_level(stl::level::trace);

    stl::logger logger("multi_sink", {console_sink, file_sink});
    logger.set_level(stl::level::debug);
    logger.warn("this should appear in both console and file");
    logger.info("this message should not appear in the console, only in the file");
}

// User defined types logging
struct my_type {
    int i = 0;

    explicit my_type(int i)
            : i(i) {};
};

#ifndef STL_USE_STD_FORMAT  // when using fmtlib

template<>
struct fmt::formatter<my_type> : fmt::formatter<std::string> {
    auto format(my_type my, format_context &ctx) -> decltype(ctx.out()) {
        return fmt::format_to(ctx.out(), "[my_type i={}]", my.i);
    }
};

#else  // when using std::format
template <>
struct std::formatter<my_type> : std::formatter<std::string> {
    auto format(my_type my, format_context &ctx) const -> decltype(ctx.out()) {
        return format_to(ctx.out(), "[my_type i={}]", my.i);
    }
};
#endif

void user_defined_example() { stl::info("user defined type: {}", my_type(14)); }

// Custom error handler. Will be triggered on log failure.
void err_handler_example() {
    // can be set globally or per logger(logger->set_error_handler(..))
    stl::set_error_handler([](const std::string &msg) {
        printf("*** Custom log error handler: %s ***\n", msg.c_str());
    });
}

// syslog example (linux/osx/freebsd)
#ifndef _WIN32

#include "core/logging/sinks/syslog_sink.h"

void syslog_example() {
    std::string ident = "stl-example";
    auto syslog_logger = stl::syslog_logger_mt("syslog", ident, LOG_PID);
    syslog_logger->warn("This is warning that will end up in syslog.");
}

#endif

// Android example.
#if defined(__ANDROID__)
#include "core/logging/sinks/android_sink.h"
void android_example() {
    std::string tag = "stl-android";
    auto android_logger = stl::android_logger_mt("android", tag);
    android_logger->critical("Use \"adb shell logcat\" to view this message.");
}
#endif

// Log patterns can contain custom flags.
// this will add custom flag '%*' which will be bound to a <my_formatter_flag> instance
#include "core/logging/pattern_formatter.h"

class my_formatter_flag : public stl::custom_flag_formatter {
public:
    void format(const stl::details::log_msg &,
                const std::tm &,
                stl::memory_buf_t &dest) override {
        std::string some_txt = "custom-flag";
        dest.append(some_txt.data(), some_txt.data() + some_txt.size());
    }

    std::unique_ptr<custom_flag_formatter> clone() const override {
        return stl::details::make_unique<my_formatter_flag>();
    }
};

void custom_flags_example() {
    using stl::details::make_unique;  // for pre c++14
    auto formatter = make_unique<stl::pattern_formatter>();
    formatter->add_flag<my_formatter_flag>('*').set_pattern("[%n] [%*] [%^%l%$] %v");
    // set the new formatter using stl::set_formatter(formatter) or
    // logger->set_formatter(formatter) stl::set_formatter(std::move(formatter));
}

void file_events_example() {
    // pass the stl::file_event_handlers to file sinks for open/close log file notifications
    stl::file_event_handlers handlers;
    handlers.before_open = [](stl::filename_t filename) {
        stl::info("Before opening {}", filename);
    };
    handlers.after_open = [](stl::filename_t filename, std::FILE *fstream) {
        stl::info("After opening {}", filename);
        fputs("After opening\n", fstream);
    };
    handlers.before_close = [](stl::filename_t filename, std::FILE *fstream) {
        stl::info("Before closing {}", filename);
        fputs("Before closing\n", fstream);
    };
    handlers.after_close = [](stl::filename_t filename) {
        stl::info("After closing {}", filename);
    };
    auto file_sink = std::make_shared<stl::sinks::basic_file_sink_mt>("logs/events-sample.txt",
                                                                      true, handlers);
    stl::logger my_logger("some_logger", file_sink);
    my_logger.info("Some log line");
}

void replace_default_logger_example() {
    // store the old logger so we don't break other examples.
    auto old_logger = stl::default_logger();

    auto new_logger =
            stl::basic_logger_mt("new_default_logger", "logs/new-default-log.txt", true);
    stl::set_default_logger(new_logger);
    stl::set_level(stl::level::info);
    stl::debug("This message should not be displayed!");
    stl::set_level(stl::level::trace);
    stl::debug("This message should be displayed..");

    stl::set_default_logger(old_logger);
}