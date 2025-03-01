/*
 Copyright 2014 Smartsheet Inc.
 Copyright 2019 SmJNI Contributors
 
 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at

     http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
*/

#include "stdpch.h"

#ifdef __ANDROID__
    #include <android/log.h>
#endif

#include <stdexcept>
#include <stdlib.h>

#include "smjni/java_externals.h"

using namespace smjni;

static void (*g_throw_problem)(const char *, const char *, va_list) = nullptr;
static void (*g_log_error)(const std::exception &, const char *, va_list) noexcept = nullptr;

void smjni::set_externals(void (*thrower)(const char *, const char *, va_list), 
                          void (*logger)(const std::exception &, const char *, va_list) noexcept)
{
    g_throw_problem = thrower;
    g_log_error = logger;
}

static std::string print_to_string(const char * format, va_list vl)
{
    va_list vlOrig;
    va_copy(vlOrig, vl);
    auto size = vsnprintf(nullptr, 0, format, vl);
    if (size <= 0)
        return std::string();
    std::string buf(size + 1, '\0');
    size = vsnprintf(&buf[0], buf.size(), format, vlOrig);
    va_end(vlOrig);
    if (size <= 0)
        return std::string();
    buf.resize(size);
    return buf;
}

[[noreturn]] void smjni::internal::do_throw_problem(const char * file_line, const char * format, ...)
{
    va_list vl;
    va_start(vl, format);
    
    if (g_throw_problem)
        g_throw_problem(file_line, format, vl);
    
    std::string message = print_to_string(format, vl);
    message += "at ";
    message += file_line;
    throw std::runtime_error(message);
}

void smjni::internal::do_log_error(const std::exception & ex, const char * format, ...) noexcept
{
    va_list vl;
    va_start(vl, format);
    if (g_log_error)
    {
        g_log_error(ex, format, vl);
    }
    else
    {
#ifdef __ANDROID__
        __android_log_print(ANDROID_LOG_ERROR, "smjni", "%s\n", ex.what());
        if (format)
            __android_log_vprint(ANDROID_LOG_ERROR, "smjni", format, vl);
#else
        fprintf(stderr, "%s\n", ex.what());
        if (format)
            vfprintf(stderr, format, vl);
#endif
    }
    va_end(vl);
}