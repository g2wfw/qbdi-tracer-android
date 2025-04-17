// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#ifndef STL_LOG_HEADER_ONLY
#    include "basic_file_sink.h"
#endif

#include "../common.h"
#include "../details/os.h"

namespace stl {
namespace sinks {

template<typename Mutex>
STL_LOG_INLINE basic_file_sink<Mutex>::basic_file_sink(const filename_t &filename, bool truncate, const file_event_handlers &event_handlers)
    : file_helper_{event_handlers}
{
    file_helper_.open(filename, truncate);
}

template<typename Mutex>
STL_LOG_INLINE const filename_t &basic_file_sink<Mutex>::filename() const
{
    return file_helper_.filename();
}

template<typename Mutex>
STL_LOG_INLINE void basic_file_sink<Mutex>::sink_it_(const details::log_msg &msg)
{
    memory_buf_t formatted;
    base_sink<Mutex>::formatter_->format(msg, formatted);
    file_helper_.write(formatted);
}

template<typename Mutex>
STL_LOG_INLINE void basic_file_sink<Mutex>::flush_()
{
    file_helper_.flush();
}

} // namespace sinks
} // namespace stl
