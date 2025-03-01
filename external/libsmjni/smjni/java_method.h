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

#ifndef HEADER_JAVA_METHOD_H_INCLUDED
#define	HEADER_JAVA_METHOD_H_INCLUDED

#include "java_type_traits.h"
#include "java_class.h"
#include "java_exception.h"

namespace smjni
{
    class java_method_id_base
    {
    public:
        java_method_id_base() noexcept : m_id(0)
        {}
        java_method_id_base(jmethodID id) noexcept: m_id(id)
        {}
        
        
        static java_method_id_base get(JNIEnv * jenv, jclass clazz, const char * name, const char * signature);
        static java_method_id_base get_static(JNIEnv * jenv, jclass clazz, const char * name, const char * signature);
        
        jmethodID get() const noexcept
            { return m_id; }
        
    private:
        jmethodID m_id;
    };

    enum method_kind
    {
        static_method,
        instance_method,
        constructor
    };
    
    template<method_kind Kind, typename ReturnType, typename... ArgType>
    class java_method_id : public java_method_id_base
    {
    private:
        typedef java_method_id_base super;
        
        template <bool Val, typename... Deps>
        static inline constexpr bool dependent_value = Val;
    
    public:
        java_method_id() noexcept = default;
        
        template<typename ClassType>
        java_method_id(JNIEnv * jenv, const java_class<ClassType> & clazz, const char * name,
                       std::enable_if_t<dependent_value<Kind == static_method, ClassType>, class nonexistent1> * = nullptr):
            super(java_method_id::get_static(jenv, clazz.c_ptr(), name, internal::java_method_signature<ReturnType, ArgType...>()))
        {
        }
        
        template<typename ClassType>
        java_method_id(JNIEnv * jenv, const java_class<ClassType> & clazz, const char * name,
                       std::enable_if_t<dependent_value<Kind == instance_method, ClassType>, class nonexistent2> * = nullptr):
            super(java_method_id::get(jenv, clazz.c_ptr(), name, internal::java_method_signature<ReturnType, ArgType...>()))
        {
        }
        
        template<typename ClassType>
        java_method_id(JNIEnv * jenv, const java_class<ClassType> & clazz,
                       std::enable_if_t<dependent_value<std::is_same_v<ReturnType, void>, ClassType> &&
                                              dependent_value<Kind == constructor, ClassType>, class nonexistent3> * = nullptr):
            super(java_method_id::get(jenv, clazz.c_ptr(), "<init>", internal::java_method_signature<ReturnType, ArgType...>()))
        {
        }
    };
    
    template<typename ReturnType, typename ThisType, typename... ArgType>
    class java_method
    {
    private:
        typedef java_method_id<instance_method, ReturnType, ArgType...> id_type;
        typedef java_type_traits<ReturnType> traits;
    public:
        typedef typename traits::return_type return_type;
    public:
        java_method() = default;
        
        java_method(JNIEnv * jenv, const java_class<ThisType> & clazz, const char* name):
            m_id(jenv, clazz, name)
        {
        }
        
        return_type operator()(JNIEnv * jenv, typename java_type_traits<ThisType>::arg_type object, 
                               typename java_type_traits<ArgType>::arg_type... params) const
        {
            auto ret = traits::call_method(jenv,
                                           object.c_ptr(),
                                           this->m_id.get(),
                                           argument_to_java(params)...);
            if (!ret)
                java_exception::check(jenv);
            return return_value_from_java(jenv, ret);
        }
        
        template<typename ClassType>
        return_type call_non_virtual(JNIEnv * jenv, typename java_type_traits<ThisType>::arg_type object,
                                     const java_class<ClassType> & clazz, 
                                     typename java_type_traits<ArgType>::arg_type... params) const
        {
            auto ret = traits::call_non_virtual_method(jenv,
                                                       argument_to_java(object),
                                                       clazz.c_ptr(),
                                                       this->m_id.get(),
                                                       argument_to_java(params)...);
            if (!ret)
                java_exception::check(jenv);
            return return_value_from_java(jenv, ret);
        }
        
    private:
        id_type m_id;
    };
    
    template<typename ReturnType, typename ClassType, typename... ArgType>
    class java_static_method
    {
    private:
        typedef java_method_id<static_method, ReturnType, ArgType...> id_type;
        typedef java_type_traits<ReturnType> traits;
    public:
        typedef typename traits::return_type return_type;
    public:
        java_static_method() = default;
        
        java_static_method(JNIEnv * jenv, const java_class<ClassType> & clazz, const char* name):
            m_id(jenv, clazz, name)
        {
        }
        
        return_type operator()(JNIEnv * jenv, const java_class<ClassType> & clazz, typename java_type_traits<ArgType>::arg_type... params) const
        {
            auto ret = traits::call_static_method(jenv,
                                                  clazz.c_ptr(),
                                                  this->m_id.get(),
                                                  argument_to_java(params)...);
            if (!ret)
                java_exception::check(jenv);
            return return_value_from_java(jenv, ret);
        }
    private:
        id_type m_id;
    };
    
    template<typename ReturnType, typename... ArgType>
    class java_constructor
    {
    private:
        typedef java_method_id<constructor, void, ArgType...> id_type;
        typedef java_type_traits<ReturnType> traits;
    public:
        typedef typename traits::return_type return_type;
    public:
        java_constructor() = default;
        java_constructor(JNIEnv * jenv, const java_class<ReturnType> & clazz):
            m_id(jenv, clazz)
        {
        }
        
        return_type operator()(JNIEnv * jenv, const java_class<ReturnType> & clazz, typename java_type_traits<ArgType>::arg_type... params) const
        {
            auto ret = traits::new_object(jenv,
                                          clazz.c_ptr(),
                                          this->m_id.get(),
                                          argument_to_java(params)...);
            if (!ret)
                java_exception::check(jenv);
            return return_value_from_java(jenv, ret);
        }
    private:
            id_type m_id;
    };
}

#endif	//HEADER_JAVA_METHOD_H_INCLUDED


