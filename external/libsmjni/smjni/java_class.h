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

#ifndef HEADER_JAVA_CLASS_H_INCLUDED
#define	HEADER_JAVA_CLASS_H_INCLUDED

#include <memory>
#include <mutex>
#include <functional>

#include "java_ref.h"
#include "java_type_traits.h"

#include "java_exception.h"
#include <type_traits>

namespace smjni
{
    namespace internal
    {
        template<typename T>
        const char * java_field_signature()
        {
            using internal::string_array;
            static constexpr const auto sig = java_type_traits<T>::signature();
            return sig.c_str();
        }
    
        template<typename ReturnType, typename... ArgType>
        const char * java_method_signature()
        {
            using internal::string_array;
            static constexpr const auto sig = (make_string_array("(") + ... + java_type_traits<ArgType>::signature()) + make_string_array(")") + java_type_traits<ReturnType>::signature();
            return sig.c_str();
        }
    
        class java_class_holder
        {
        public:
            java_class_holder(global_java_ref<jclass> && clazz):
                m_class(std::move(clazz))
            {}
                
            jclass c_ptr() const
            {
                return m_class.c_ptr();
            }

            bool is_instance_of(JNIEnv * jenv, const auto_java_ref<jobject> & obj) const
            {
                return jenv->IsInstanceOf(obj.c_ptr(), c_ptr());
            }
            
        private:
            global_java_ref<jclass> m_class;
        };
    }



    
    template<typename T>
    class java_class 
    {
    public:
        template<class Callable>
        static constexpr bool is_loader = std::is_invocable_r_v<local_java_ref<jclass>, Callable, JNIEnv *> ||
                                          std::is_invocable_r_v<global_java_ref<jclass>, Callable, JNIEnv *>;
    public:
        java_class(const auto_java_ref<jclass> & clazz):
            m_holder(init([clazz] () { return clazz; }))
        {}
        
        template<class Loader>
        java_class(JNIEnv * jenv, Loader loader,
                   std::enable_if_t<is_loader<Loader>> * = nullptr):
            m_holder(init([loader, jenv] () { return loader(jenv); }))
        {}
        
        jclass c_ptr() const
        {
            return m_holder->c_ptr();
        }
        
        bool is_instance_of(JNIEnv * jenv, const auto_java_ref<jobject> & obj) const
        {
            return m_holder->is_instance_of(jenv, obj);
        }

        template<typename ReturnType, typename... ArgType>
        static JNINativeMethod bind_native(const char * name, ReturnType (JNICALL *func)(JNIEnv *, jclass, ArgType...)) noexcept
        {
            using name_type = decltype(JNINativeMethod::name);
            using signature_type = decltype(JNINativeMethod::signature);
            using method_type = decltype(JNINativeMethod::fnPtr);

            const char * signature = internal::java_method_signature<ReturnType, ArgType...>();
            return {name_type(name), signature_type(signature), (method_type)func};
        }

        template<typename ReturnType, typename... ArgType>
        static JNINativeMethod bind_native(const char * name, ReturnType (JNICALL *func)(JNIEnv *, T, ArgType...)) noexcept
        {
            using name_type = decltype(JNINativeMethod::name);
            using signature_type = decltype(JNINativeMethod::signature);
            using method_type = decltype(JNINativeMethod::fnPtr);

            const char * signature = internal::java_method_signature<ReturnType, ArgType...>();
            return {name_type(name), signature_type(signature), (method_type)func};
        }

        template<size_t N>
        void register_natives(JNIEnv * jenv, const JNINativeMethod (&methods)[N]) const
        {
            int res = jenv->RegisterNatives(c_ptr(), methods, size_to_java(N));
            if (res != 0)
            {
                java_exception::check(jenv);
                THROW_JAVA_PROBLEM("unable to register native methods, error %d", res);
            }
        }
        
    private:
        template<class Loader>
        static
        std::shared_ptr<internal::java_class_holder> init(Loader loader)
        {
            std::lock_guard<std::mutex> lock(s_holder_mutex);
            auto ret = s_holder.lock();
            if (!ret)
            {
                ret = std::make_shared<internal::java_class_holder>(loader());
                s_holder = ret;
            }
            return ret;
        }
        
    private:
        const std::shared_ptr<internal::java_class_holder> m_holder;
        
        static std::mutex s_holder_mutex;
        static std::weak_ptr<internal::java_class_holder> s_holder;
    };
    template<typename T>
    std::mutex java_class<T>::s_holder_mutex;
    template<typename T>
    std::weak_ptr<internal::java_class_holder> java_class<T>::s_holder;
}

#endif	//HEADER_JAVA_CLASS_H_INCLUDED


