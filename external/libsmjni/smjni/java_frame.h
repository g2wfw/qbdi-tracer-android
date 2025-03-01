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

#ifndef HEADER_SMJNI_JAVA_FRAME_H_INCLUDED
#define	HEADER_SMJNI_JAVA_FRAME_H_INCLUDED

#include "config.h"

namespace smjni
{
    class java_frame
    {
    public:
        java_frame(JNIEnv * env, jint capacity):
            m_env(env),
            m_pushed(false)
        {
            int ret = m_env->PushLocalFrame(capacity);
            if (ret != 0)
            {
                java_exception::check(env);
                THROW_JAVA_PROBLEM("cannot push local frame");
            }
            m_pushed = true;
        }
            
        ~java_frame()
        {
            if (m_pushed)
                m_env->PopLocalFrame(nullptr);
        }
        
        template<typename T>
        T pop(T obj)
        {
            if (!m_pushed)
                THROW_JAVA_PROBLEM("frame not pushed");
            T ret = static_cast<T>(m_env->PopLocalFrame(obj));
            m_pushed = false;
            return ret;
        }
        
        
        java_frame(const java_frame &) = delete;
        java_frame & operator=(const java_frame &) = delete;
        java_frame(java_frame &&) = delete;
        java_frame & operator=(java_frame &&) = delete;
    private:
        JNIEnv * m_env;
        bool m_pushed;
    };
}


#endif	

