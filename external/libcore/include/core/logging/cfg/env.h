// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once
#include "helpers.h"
#include "../details/registry.h"
#include "../details/os.h"

//
// Init levels and patterns from env variables STL_LOG_LEVEL
// Inspired from Rust's "env_logger" crate (https://crates.io/crates/env_logger).
// Note - fallback to "info" level on unrecognized levels
//
// Examples:
//
// set global level to debug:
// export STL_LOG_LEVEL=debug
//
// turn off all logging except for logger1:
// export STL_LOG_LEVEL="*=off,logger1=debug"
//

// turn off all logging except for logger1 and logger2:
// export STL_LOG_LEVEL="off,logger1=debug,logger2=info"

namespace stl {
namespace cfg {
inline void load_env_levels()
{
    auto env_val = details::os::getenv("STL_LOG_LEVEL");
    if (!env_val.empty())
    {
        helpers::load_levels(env_val);
    }
}

} // namespace cfg
} // namespace stl
