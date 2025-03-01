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
#include "smjni/java_string.h"
#include "smjni/java_type_traits.h"

#include <iterator>
#include <cstring>

using namespace smjni;

static thread_local std::vector<jchar> g_utf16_buffer;
static constexpr size_t g_max_buffer_size = 64 * 1024;

static local_java_ref<jstring> java_string_create(JNIEnv * env, const char * str, size_t size)
{
    g_utf16_buffer.clear();
    utf8_to_utf16(str, str + size, std::back_inserter(g_utf16_buffer));
    
    jsize length = size_to_java(g_utf16_buffer.size());
    const jchar * start = length != 0 ? g_utf16_buffer.data() : nullptr;
    local_java_ref<jstring> ret = java_string_create(env, start, length);
    
    if (g_utf16_buffer.size() > g_max_buffer_size)
    {
        g_utf16_buffer.resize(g_max_buffer_size);
        g_utf16_buffer.shrink_to_fit();
    }
    
    return ret;
}

local_java_ref<jstring> smjni::java_string_create(JNIEnv * env, const char * str)
{
    return java_string_create(env, str, (str ? strlen(str) : 0));
}

local_java_ref<jstring> smjni::java_string_create(JNIEnv * env, const std::string & str)
{
    return java_string_create(env, str.data(), str.size());
}

std::string smjni::java_string_to_cpp(JNIEnv * env, const auto_java_ref<jstring> & str)
{
    java_string_access access(env, str);
    std::string ret;
    utf16_to_utf8(access.begin(), access.end(), std::back_inserter(ret));
    return ret;
}
