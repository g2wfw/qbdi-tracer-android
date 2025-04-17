// The MIT License (MIT)
//
// Copyright (c) 2025  g2wfw
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
#include <core/linker.h>
#include <core/library.h>
#include <trace/instruction_tracer_manager.h>
#include <android/log.h>

#define LOG_TAG "linker-test"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)


//so_info is nullptr
void
linker_dlopen_pre(const char *path, stl::linker_type_t type, void *, void *user_data) {
    LOGI("dlopen %s", path);
}

void
linker_init(const char *path, stl::linker_type_t type, void *so_info, void *user_data) {
    LOGI("init %s", path);
}

void
linker_init_post(const char *path, stl::linker_type_t type, void *so_info, void *user_data) {
    LOGI("init post %s", path);
}

void
linker_init_array(const char *path, stl::linker_type_t type, void *so_info, void *user_data) {
    LOGI("init %s", path);
}

void
linker_init_array_post(const char *path, stl::linker_type_t type, void *so_info, void *user_data) {
    LOGI("init post %s", path);

}


void
linker_dlopen_post(const char *path, stl::linker_type_t type, void *so_info, void *user_data) {
    LOGI("dlopen post %s", path);
    stl::Library library(path, so_info);
    //get elf base addr
    auto base = library.get_load_bias();
    LOGI("%s base 0x%lx", path, base);

    auto instance = InstructionTracerManager::get_instance();
    if (!instance->init(base + 0x6884C)) {
        LOGE("init qdbi fail");
        return;
    }
    instance->get_info_manager()->set_enable_to_file(true);
    instance->get_info_manager()->set_enable_to_logcat(true);
    instance->get_info_manager()->set_memory_dump_to_file(true);
    instance->run_attach();
}


void linker_monitor() {
    //add monitor when dlopen target library
    stl::Linker::getInstance()->add_library_monitor("libtest.so", stl::DLOPEN,
                                                    linker_dlopen_pre, nullptr);
    //add monitor after dlopen target library
    stl::Linker::getInstance()->add_library_monitor("libtest.so", stl::DLOPEN_POST,
                                                    linker_dlopen_post, nullptr);

    //add monitor before init func call
    stl::Linker::getInstance()->add_library_monitor("libtest.so", stl::INIT_PROC,
                                                    linker_init, nullptr);

    //add monitor after init func call
    stl::Linker::getInstance()->add_library_monitor("libtest.so", stl::INIT_PROC_POST,
                                                    linker_init_post, nullptr);
    //add monitor before init array func call
    stl::Linker::getInstance()->add_library_monitor("libtest.so", stl::INIT_ARRAY_POST,
                                                    linker_init_array, nullptr);
    //add monitor after init array func call
    stl::Linker::getInstance()->add_library_monitor("libtest.so", stl::INIT_ARRAY_POST,
                                                    linker_init_array_post, nullptr);

}








