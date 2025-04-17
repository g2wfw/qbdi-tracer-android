/*
The MIT License (MIT)

Copyright (c) 2025 g2wfw

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.*/

#include "back_trace_manager.h"
#include <backtrace/backtrace_constants.h>
#include <backtrace/Backtrace.h>
#include <backtrace/BacktraceMap.h>
#include <xdl.h>

namespace stl {

// Collect/print the call stack (function, file, line) traces for a single thread.
    class CallStack {
    public:

        explicit CallStack() {
            this->update(6);
        }

        CallStack(int32_t ignoreDepth) {
            this->update(ignoreDepth + 1);
        }

        ~CallStack() {

        }

        // Reset the stack frames (same as creating an empty call stack).
        void clear() { mFrameLines.clear(); }

        // Immediately collect the stack traces for the specified thread.
        // The default is to dump the stack of the current call.
        void update(int32_t ignoreDepth = 1, pid_t tid = BACKTRACE_CURRENT_THREAD) {
            mFrameLines.clear();
            std::unique_ptr<Backtrace> backtrace(Backtrace::Create(BACKTRACE_CURRENT_PROCESS, tid));
            if (!backtrace->Unwind(ignoreDepth)) {
                __android_log_print(ANDROID_LOG_WARN, "CallStack", "%s: Failed to unwind callstack.", __FUNCTION__);
            }
            void *cache = NULL;
            for (size_t i = 0; i < backtrace->NumFrames(); i++) {
                auto *frame = (backtrace->GetFrame(i));
                xdl_info_t xdlInfo;

                memset(&xdlInfo, 0, sizeof(xdlInfo));
                xdl_addr((void *) frame->pc, &xdlInfo, &cache);
                if (xdlInfo.dli_fname != nullptr) {
                    frame->map->name = xdlInfo.dli_fname;
                }
                if (xdlInfo.dli_sname != nullptr && frame->func_name.empty()) {
                    frame->func_name = xdlInfo.dli_sname;
                }
                mFrameLines.push_back(backtrace->FormatFrameData(i));
            }
            xdl_addr_clean(&cache);
        }

        const std::vector<std::string> &getMFrameLines() const {
            return mFrameLines;
        }


        // Get the count of stack frames that are in this call stack.
        size_t size() const { return mFrameLines.size(); }

    private:
        std::vector<std::string> mFrameLines;
    };

    BackTraceManager *BackTraceManager::getInstance() {
        static BackTraceManager backTraceManager;
        return &backTraceManager;
    }

    BackTraceManager::BackTraceManager() {

    }

    std::vector<std::string> BackTraceManager::get_backtrace_lines() {
        CallStack callStack;
        return callStack.getMFrameLines();
    }

    void BackTraceManager::print_backtrace(const char *tag) {
        __android_log_print(ANDROID_LOG_INFO, tag,
                            "--------------------------------------------stack frame--------------------------------------------");

        auto lines = get_backtrace_lines();
        for (const auto &line: lines) {
            __android_log_print(ANDROID_LOG_INFO, tag, "| %s", line.c_str());
        }
        __android_log_print(ANDROID_LOG_INFO, tag,
                            "---------------------------------------------------------------------------------------------------");
    }


}