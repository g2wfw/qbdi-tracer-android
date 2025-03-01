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

#include "smjni/jni_provider.h"
#include "smjni/java_externals.h"

using namespace smjni;

namespace smjni
{
    namespace internal
    {
        class jni_record
        {
        public:
            jni_record();
            ~jni_record();

            JNIEnv * env() const noexcept
                { return m_env; }
        private:
            JNIEnv * m_env;
            bool m_attached;
        };

        jni_provider * g_provider = nullptr;
    }
}

thread_local std::unique_ptr<internal::jni_record> jni_provider::m_record;

void jni_provider::init(JNIEnv * initialEnv)
{
    if (internal::g_provider)
        return;
    
    JavaVM * vm = nullptr;
    jint res = initialEnv->GetJavaVM(&vm);
    if (res != 0)
        THROW_JAVA_PROBLEM("failed to obtain Java VM, error %d", res);
    internal::g_provider = new jni_provider(vm);
}

void jni_provider::init(JavaVM * vm)
{
    if (internal::g_provider)
        return;
    
    internal::g_provider = new jni_provider(vm);
}

JNIEnv * jni_provider::get_jni()
{ 
    internal::jni_record * ptr = m_record.get();
    if (!ptr)
    {
        ptr = new internal::jni_record;
        m_record.reset(ptr);
    }
    return ptr->env();
}

template<typename T>
T ** AttachCurrentThreadAsDaemonOutputTypeDetector(jint (JavaVM::*)(T **, void *));

internal::jni_record::jni_record():
    m_env(nullptr),
    m_attached(false)
{ 
    JavaVM * vm = g_provider->vm();

    jint get_res = vm->GetEnv(reinterpret_cast<void**>(&m_env), JNI_VERSION_1_6);
    if (get_res != JNI_OK)
    {
        JavaVMAttachArgs args = {
            JNI_VERSION_1_6,
            nullptr,
            nullptr
        };

        using env_ret_type = decltype(AttachCurrentThreadAsDaemonOutputTypeDetector(&JavaVM::AttachCurrentThreadAsDaemon));
        jint attach_res = vm->AttachCurrentThreadAsDaemon((env_ret_type)&m_env, &args);
        if (attach_res != JNI_OK)
            THROW_JAVA_PROBLEM("failed to obtain JNIEnv, error %d and failed to attach Java VM to current thread, error %d", get_res, attach_res);
        m_attached = true;
    } 
}

internal::jni_record::~jni_record()
{
    if (m_attached && g_provider)
    {
        g_provider->vm()->DetachCurrentThread();
    }
}
