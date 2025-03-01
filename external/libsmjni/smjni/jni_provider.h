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

#ifndef HEADER_JNI_PROVIDER_H_INCLUDED
#define HEADER_JNI_PROVIDER_H_INCLUDED

#include <jni.h>

#include <memory>

#include "config.h"

namespace smjni {
    namespace internal {
        class jni_record;
    };

    class jni_provider {
    public:
        jni_provider(const jni_provider &) = delete;

        jni_provider &operator=(const jni_provider &) = delete;

        static void init(JNIEnv *initialEnv);

        static void init(JavaVM *vm);

        static JNIEnv *get_jni();

        JavaVM *vm() const { return m_vm; }

    private:
        jni_provider(JavaVM *vm) :
                m_vm(vm) {}

        ~jni_provider() = default;

    private:
        JavaVM *m_vm;
        static thread_local std::unique_ptr<internal::jni_record> m_record;
    };
}

#endif //HEADER_JNI_PROVIDER_H_INCLUDED
