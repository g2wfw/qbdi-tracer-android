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


#ifndef QBDI_TRACER_LOGGER_MANAGER_H
#define QBDI_TRACER_LOGGER_MANAGER_H

#include <string>
#include <spdlog/spdlog.h>
#include <QBDI.h>
#include "common.h"

class LoggerManager {
public:
    LoggerManager(std::string module_name, module_range_t module_range) : module_name(std::move(module_name)),
                                                                          module_range(module_range) {

    }

    void write_trace_info(const inst_trace_info_t *info, const QBDI::InstAnalysis *instAnalysis,
                          std::vector<QBDI::MemoryAccess> &memoryAccesses);

    void set_enable_to_logcat(bool enable);

    void set_enable_to_file(bool enable);

private:
    static bool check_and_mkdir(std::string &path);

    void write_info(std::string &line) const;

    void
    format_register_info(std::string &result, const inst_trace_info_t *info,
                         const QBDI::InstAnalysis *instAnalysis);

    void format_call_info(std::string &result, const inst_trace_info_t *info,
                          const QBDI::InstAnalysis *instAnalysis);

    void format_access_info(std::string &result, std::vector<QBDI::MemoryAccess> &memoryAccesses);

    inline bool is_address_in_module_range(uintptr_t addr) {
        return addr >= this->module_range.base && addr < this->module_range.end;
    };

private:
    std::shared_ptr<spdlog::logger> logcat;
    std::shared_ptr<spdlog::logger> file_log;
    std::string trace_log_file;
    std::string trace_log_base;
    std::string module_name;
    module_range_t module_range;
};


#endif //QBDI_TRACER_LOGGER_MANAGER_H
