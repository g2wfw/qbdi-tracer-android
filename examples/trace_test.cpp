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
#include <jni.h>
#include <fcntl.h>
#include <unistd.h>
#include <trace/instruction_tracer_manager.h>
#include "linux_syscall_support.h"

#define LOG_TAG "trace-test"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

void libc_call() {
    uint8_t *ptr = static_cast<uint8_t *>(malloc(100));
    LOGI("ptr:%p", ptr);
    for (int i = 0; i < 10; ++i) {
        ptr[i] = i;
    }
    free(ptr);
}


void run_qbdi_call() {
    auto instance = InstructionTracerManager::get_instance();
    if (!instance->init((uintptr_t) &libc_call)) {
        LOGE("init qdbi fail");
    }
    instance->get_info_manager()->set_enable_to_logcat(true);
    instance->get_info_manager()->set_memory_dump_to_file(true);
    if (!instance->run()) {
        LOGE("run attach fail");
    }
}

void run_qbdi_attach() {
    auto instance = InstructionTracerManager::get_instance();
    if (!instance->init("libmtguard.so", 0x6884C)) {
        LOGE("init qdbi fail");
        return;
    }
    instance->get_info_manager()->set_enable_to_file(true);
    instance->get_info_manager()->set_enable_to_logcat(true);
    instance->get_info_manager()->set_memory_dump_to_file(true);
    instance->run_attach();
}


void test() {
    run_qbdi_call();
    run_qbdi_attach();
}

