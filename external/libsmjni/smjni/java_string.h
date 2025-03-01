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

#ifndef HEADER_JAVA_STRING_INCLUDED
#define HEADER_JAVA_STRING_INCLUDED

#include "java_ref.h"
#include "java_exception.h"
#include "utf_util.h"

#include <string>

namespace smjni
{
    inline local_java_ref<jstring> java_string_create(JNIEnv * env, const jchar * str, jsize len)
    {
        jstring ret = env->NewString(str, len);
        if (!ret)
        {
            java_exception::check(env);
            THROW_JAVA_PROBLEM("cannot create java string");
        }
        return jattach(env, ret);
    } 
        
    local_java_ref<jstring> java_string_create(JNIEnv * env, const char * str);
    local_java_ref<jstring> java_string_create(JNIEnv * env, const std::string & str);
        
    inline jsize java_string_get_length(JNIEnv * env, const auto_java_ref<jstring> & str)
    {
        if (!str)
            return 0;
        jsize ret = env->GetStringLength(str.c_ptr());
        if (ret < 0)
        {
            java_exception::check(env);
            THROW_JAVA_PROBLEM("invalid string size");
        }
        return ret;
    }

    inline void java_string_get_region(JNIEnv * env, const auto_java_ref<jstring> & str, jsize start, jsize len, jchar * buf)
    {
        env->GetStringRegion(str.c_ptr(), start, len, buf);
        java_exception::check(env);
    }

    std::string java_string_to_cpp(JNIEnv * env, const auto_java_ref<jstring> & str);

    class java_string_access
    {
    public:
        typedef jchar element_type;
        
        typedef const element_type * iterator;
        typedef const element_type * const_iterator;
        typedef jsize size_type;
        typedef element_type value_type;
    public:
        java_string_access(JNIEnv * env, const auto_java_ref<jstring> & str):
            m_env(env),
            m_str(str.c_ptr()),
            m_length(java_string_get_length(env, str))
        {
            if (!str)
                return;
            m_data = env->GetStringChars(m_str, nullptr);
            if (!m_data)
            {
                java_exception::check(env);
                THROW_JAVA_PROBLEM("cannot access java string");
            }
        }
        java_string_access(const java_string_access &) = delete;
        java_string_access & operator=(const java_string_access &) = delete;
        ~java_string_access()
        {
            if (m_data)
                m_env->ReleaseStringChars(m_str, m_data);
        }
        
        const element_type * begin() const
        {
            return m_data;
        }
        const element_type * end() const
        {
            return m_data + m_length;
        }
        jsize size() const
        {
            return m_length;
        }
        const element_type & operator[](jsize idx) const
        {
            return m_data[idx];
        }
    private:
        JNIEnv * m_env = nullptr;
        jstring m_str = nullptr;
        jsize m_length = 0;
        const element_type * m_data = nullptr;
    };
}

#endif
