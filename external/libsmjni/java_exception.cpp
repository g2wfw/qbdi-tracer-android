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

#include <vector>
#include <iterator>

#include "smjni/java_string.h"
#include "smjni/java_exception.h"
#include "smjni/java_runtime.h"

using namespace smjni;

const char *java_exception::what() const noexcept {
    if (m_what.empty()) {
        try {
            m_what = do_what();
        }
        catch (std::exception &ex) {
            internal::do_log_error(ex, "exception while trying to produce exception description");
            //ignore
        }
    }
    return m_what.c_str();
}

std::string java_exception::do_what() const {
    std::string ret = "smjni::java_exception";

    if (m_throwable.c_ptr() == nullptr) {
        internal::do_log_error(std::bad_exception(), "m_throwable is nullptr");
        return ret;
    }
    JNIEnv *jenv = jni_provider::get_jni();
    try {
        ret += "\n";
        ret += getStackTrace();
        return ret;
    } catch (std::exception exception) {
        internal::do_log_error(exception, "exception while trying to produce exception description");
        return exception.what();
    }
}

void java_exception::translate(JNIEnv *env, const std::exception &ex) {
    const char *message = ex.what();
    auto java_message = java_string_create(env, message);
    auto exception = java_runtime::throwable().ctor(env, java_message);
    java_exception::raise(env, exception);
}

std::string java_exception::getStackTrace() const {
    JNIEnv *jenv = jni_provider::get_jni();
    jclass throwable_clazz = jenv->GetObjectClass(m_throwable.c_ptr());
    jclass stringwriter_clazz = jenv->FindClass("java/io/StringWriter");
    jmethodID javastringwriter = jenv->GetMethodID(stringwriter_clazz, "<init>", "()V");
    jclass printwriter_clazz = jenv->FindClass("java/io/PrintWriter");
    jmethodID javaprintwriter = jenv->GetMethodID(printwriter_clazz, "<init>", "(Ljava/io/Writer;)V");
    jobject stringwrite_instance = jenv->NewObject(stringwriter_clazz, javastringwriter);
    jobject printwriter_instance = jenv->NewObject(printwriter_clazz, javaprintwriter, stringwrite_instance);


    jmethodID printStackTrace = jenv->GetMethodID(throwable_clazz, "printStackTrace", "(Ljava/io/PrintWriter;)V");
    jenv->CallVoidMethod(m_throwable.c_ptr(), printStackTrace, printwriter_instance);

    jmethodID toString = jenv->GetMethodID(stringwriter_clazz, "toString", "()Ljava/lang/String;");

    jstring result = static_cast<jstring>(jenv->CallObjectMethod(stringwrite_instance, toString));
    auto re = smjni::java_string_to_cpp(jenv, result);
    jenv->DeleteLocalRef(result);
    jenv->DeleteLocalRef(throwable_clazz);
    jenv->DeleteLocalRef(stringwriter_clazz);
    jenv->DeleteLocalRef(printwriter_clazz);
    jenv->DeleteLocalRef(stringwrite_instance);
    jenv->DeleteLocalRef(printwriter_instance);
    return re;
}
