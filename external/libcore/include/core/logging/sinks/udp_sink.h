// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#include "../common.h"
#include "base_sink.h"
#include "../details/null_mutex.h"
#ifdef _WIN32
#    include "../details/udp_client-windows.h"
#else
#    include "../details/udp_client.h"
#endif

#include <mutex>
#include <string>
#include <chrono>
#include <functional>

// Simple udp client sink
// Sends formatted log via udp

namespace stl {
namespace sinks {

struct udp_sink_config
{
    std::string server_host;
    uint16_t server_port;

    udp_sink_config(std::string host, uint16_t port)
        : server_host{std::move(host)}
        , server_port{port}
    {}
};

template<typename Mutex>
class udp_sink : public stl::sinks::base_sink<Mutex>
{
public:
    // host can be hostname or ip address
    explicit udp_sink(udp_sink_config sink_config)
        : client_{sink_config.server_host, sink_config.server_port}
    {}

    ~udp_sink() override = default;

protected:
    void sink_it_(const stl::details::log_msg &msg) override
    {
        stl::memory_buf_t formatted;
        stl::sinks::base_sink<Mutex>::formatter_->format(msg, formatted);
        client_.send(formatted.data(), formatted.size());
    }

    void flush_() override {}
    details::udp_client client_;
};

using udp_sink_mt = udp_sink<std::mutex>;
using udp_sink_st = udp_sink<stl::details::null_mutex>;

} // namespace sinks

//
// factory functions
//
template<typename Factory = stl::synchronous_factory>
inline std::shared_ptr<logger> udp_logger_mt(const std::string &logger_name, sinks::udp_sink_config skin_config)
{
    return Factory::template create<sinks::udp_sink_mt>(logger_name, skin_config);
}

} // namespace stl
