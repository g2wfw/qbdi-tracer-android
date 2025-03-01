/*
 * MIT License
 * 
 * Copyright (c) 2024 g2wfw
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */


#include "dispatch_libz.h"
#include <unordered_map>
#include <zlib.h>
#include <spdlog/fmt/fmt.h>
#include <sys/mman.h>

static std::unordered_map<uintptr_t, std::pair<const char *, std::function<void(
        inst_trace_info_t *info)>>> libz_handlers;

DispatchLibz *DispatchLibz::get_instance() {
    static DispatchLibz dispatchLibz;
    return &dispatchLibz;
}

bool DispatchLibz::dispatch_args(inst_trace_info_t *info) {
    if (!is_module_address(info->fun_call->fun_address)) {
        return false;
    }
    info->fun_call->call_module_name = "libz.so";
    auto handler = libz_handlers.find(info->fun_call->fun_address);
    if (handler != libz_handlers.end()) {
        auto pair = handler->second;
        info->fun_call->fun_name = pair.first;
        pair.second(info);
        return true;
    }
    auto fun_name = get_address_symbol(info->fun_call->fun_address);
    if (fun_name.empty()) {
        LOGE("fun_name is empty %p", (void *) info->fun_call->fun_address);
        return true;
    }
    info->fun_call->fun_name = fun_name;
    add_common_reg_values(info);


    return true;
}

bool DispatchLibz::dispatch_ret(inst_trace_info_t *info, const QBDI::GPRState *ret_status) {
    if (!is_module_address(info->fun_call->fun_address)) {
        return false;
    }
    auto ret_type = info->fun_call->ret_type;
    auto ret_value = get_ret_register_value(ret_status, 0);
    switch (ret_type) {
        case kUnknown:
        case kNumber: {
            info->fun_call->ret_value = fmt::format("{:#x}", ret_value);
        }
            break;
        case kPointer: {
            info->fun_call->ret_value = (fmt::format("{}",
                                                     read_buffer_hexdump_from_address(
                                                             ret_value)));
            break;
        }
            break;
        case kString: {
            info->fun_call->ret_value = (
                    fmt::format("{}", read_string_from_address(ret_value)));
        }
            break;
        default:
            break;
    }
    return true;
}

DispatchLibz::DispatchLibz() {
    auto module = gum_process_find_module_by_name("libz.so");
    auto range = gum_module_get_range(module);
    module_range.base = range->base_address;
    module_range.end = range->size + range->base_address;
    //ZEXTERN int ZEXPORT deflate OF((z_streamp strm, int flush));
    REGISTER_HANDLER(libz_handlers, deflate, {
        auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
        auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
        trace_info->fun_call->args.push_back(fmt::format("strm={:#x}", arg0));
        trace_info->fun_call->args.push_back(fmt::format("flush={:#x}", arg1));
        trace_info->fun_call->ret_type = kNumber;
    });
    REGISTER_HANDLER(libz_handlers, inflate, {
        auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
        auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
        trace_info->fun_call->args.push_back(fmt::format("strm={:#x}", arg0));
        trace_info->fun_call->args.push_back(fmt::format("flush={:#x}", arg1));
        trace_info->fun_call->ret_type = kNumber;
    });
    REGISTER_HANDLER(libz_handlers, deflateEnd, {
        auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
        trace_info->fun_call->args.push_back(fmt::format("strm={:#x}", arg0));
        trace_info->fun_call->ret_type = kNumber;
    });
    REGISTER_HANDLER(libz_handlers, inflateEnd, {
        auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
        trace_info->fun_call->args.push_back(fmt::format("strm={:#x}", arg0));
        trace_info->fun_call->ret_type = kNumber;
    });
    REGISTER_HANDLER(libz_handlers, deflateReset, {
        auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
        trace_info->fun_call->args.push_back(fmt::format("strm={:#x}", arg0));
        trace_info->fun_call->ret_type = kNumber;
    });
    /*   ZEXTERN int ZEXPORT deflateInit_ OF((z_streamp strm, int level,
                                                   const char *version, int stream_size));

       ZEXTERN int ZEXPORT deflateInit2_ OF((z_streamp strm, int  level, int  method,
               int windowBits, int memLevel,
               int strategy, const char *version,
                                                    int stream_size));
       ZEXTERN int ZEXPORT inflateInit2_ OF((z_streamp strm, int  windowBits,const char *version, int stream_size));*/
    REGISTER_HANDLER(libz_handlers, deflateInit2_, {
        auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
        auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
        auto arg2 = get_arg_register_value(&trace_info->pre_status, 2);
        auto arg3 = get_arg_register_value(&trace_info->pre_status, 3);
        auto arg4 = get_arg_register_value(&trace_info->pre_status, 4);
        auto arg5 = get_arg_register_value(&trace_info->pre_status, 5);
        auto arg6 = get_arg_register_value(&trace_info->pre_status, 6);
        auto arg7 = get_arg_register_value(&trace_info->pre_status, 7);
        trace_info->fun_call->args.push_back(fmt::format("strm={:#x}", arg0));
        trace_info->fun_call->args.push_back(fmt::format("level={:#x}", arg1));
        trace_info->fun_call->args.push_back(fmt::format("method={:#x}", arg2));
        trace_info->fun_call->args.push_back(fmt::format("windowBits={:#x}", arg3));
        trace_info->fun_call->args.push_back(fmt::format("memLevel={:#x}", arg4));
        trace_info->fun_call->args.push_back(fmt::format("strategy={:#x}", arg5));
        trace_info->fun_call->args.push_back(fmt::format("version={:#x}", arg6));
        trace_info->fun_call->args.push_back(fmt::format("stream_size={:#x}", arg7));
        trace_info->fun_call->ret_type = kNumber;
    });
/*    ZEXTERN int ZEXPORT inflateInit_ OF((z_streamp strm,const char *version, int stream_size));*/
    REGISTER_HANDLER(libz_handlers, inflateInit2_, {
        auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
        auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
        auto arg2 = get_arg_register_value(&trace_info->pre_status, 2);

        trace_info->fun_call->args.push_back(fmt::format("strm={:#x}", arg0));
        trace_info->fun_call->args.push_back(fmt::format("version={}", read_string_from_address(arg1)));
        trace_info->fun_call->args.push_back(fmt::format("stream_size={:#x}", arg2));
        trace_info->fun_call->ret_type = kNumber;
    });
    //ZEXTERN int ZEXPORT inflateInit2_ OF((z_streamp strm, int  windowBits,const char *version, int stream_size));
    REGISTER_HANDLER(libz_handlers, inflateInit2_, {
        auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
        auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
        auto arg2 = get_arg_register_value(&trace_info->pre_status, 2);
        auto arg3 = get_arg_register_value(&trace_info->pre_status, 3);
        trace_info->fun_call->args.push_back(fmt::format("strm={:#x}", arg0));
        trace_info->fun_call->args.push_back(fmt::format("windowBits={:#x}", arg1));
        trace_info->fun_call->args.push_back(fmt::format("version={}", read_string_from_address(arg2)));
        trace_info->fun_call->args.push_back(fmt::format("stream_size={:#x}", arg3));
        trace_info->fun_call->ret_type = kNumber;
    });
    //mmap
    mmap()


    gum_module_enumerate_exports(module, dispatch_export_func, this);
    g_object_unref(module);
}
