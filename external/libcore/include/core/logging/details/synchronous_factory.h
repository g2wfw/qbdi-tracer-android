// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#include "registry.h"

namespace stl {

// Default logger factory-  creates synchronous loggers
class logger;

struct synchronous_factory
{
    template<typename Sink, typename... SinkArgs>
    static std::shared_ptr<stl::logger> create(std::string logger_name, SinkArgs &&...args)
    {
        auto sink = std::make_shared<Sink>(std::forward<SinkArgs>(args)...);
        auto new_logger = std::make_shared<stl::logger>(std::move(logger_name), std::move(sink));
        details::registry::instance().initialize_logger(new_logger);
        return new_logger;
    }
};
} // namespace stl
