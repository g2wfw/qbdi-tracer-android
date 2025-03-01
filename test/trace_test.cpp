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
#include <jni_provider.h>
#include <fcntl.h>
#include <unistd.h>
#include "core/common.h"
#include "core/instruction_tracer_manager.h"
#include "linux_syscall_support.h"

void libc_call() {
    /*  auto fd = strlen("/proc/self/cmdline");
      LOGI("len:%lu", fd);
      uint8_t *buffer = new uint8_t[fd];
      for (int i = 0; i < fd; ++i) {
          buffer[i] = i & 0xff;
      }
      delete[] buffer;*/

    auto fd2 = sys_open("/proc/self/cmdline", O_RDONLY, 0);
    if (fd2 < 0) {
        LOGE("open fail");
    }
    sys_close(fd2);

}


void run_qbdi() {
    auto instance = InstructionTracerManager::get_instance();
    if (!instance->init((uintptr_t) &libc_call)) {
        LOGE("init qdbi fail");
    }
    instance->get_info_manager()->set_enable_to_logcat(true);
    if (!instance->run()) {
        LOGE("run attach fail");
    }
}


JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
    JNIEnv *env;
    if (vm->GetEnv((void **) &env, JNI_VERSION_1_6) != JNI_OK) {
        return JNI_ERR;
    }
    smjni::jni_provider::init(env);
    LOGI("init qdbi trace .....");
    //run_qbdi();
    return JNI_VERSION_1_6;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_com_g2w_Itrace_trace_1test(JNIEnv *env, jclass clazz) {
    auto instance = InstructionTracerManager::get_instance();
    if (!instance->init("libxxx.so", 0x846B8)) {
        LOGE("init qdbi fail");
        return;
    }
    instance->add_record_range(0x131680, 0x134EE4);
    instance->add_record_range(0x134F70, 0x1362B8);
    instance->get_info_manager()->set_enable_to_file(true);
    instance->get_info_manager()->set_enable_to_logcat(true);
    instance->run_attach();
}