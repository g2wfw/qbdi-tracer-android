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

#ifndef HEADER_JAVA_FIELD_H_INCLUDED
#define HEADER_JAVA_FIELD_H_INCLUDED

#include "java_type_traits.h"
#include "java_class.h"
#include "java_exception.h"

namespace smjni
{
    class java_field_id_base
    {
    public:
        java_field_id_base() noexcept : m_id(0)
        {}
        java_field_id_base(jfieldID id) noexcept: m_id(id)
        {}
        
        static java_field_id_base get(JNIEnv * jenv, jclass clazz, const char * name, const char * signature);
        static java_field_id_base get_static(JNIEnv * jenv, jclass clazz, const char * name, const char * signature);
        
        
        jfieldID get() const
            { return m_id; }
        
    private:
        jfieldID m_id;
    };

    enum field_kind
    {
        static_field,
        instance_field
    };
    
    template<field_kind Kind, typename Type>
    class java_field_id : public java_field_id_base
    {
    private:
        typedef java_field_id_base super;
        
        template <bool Val, typename... Deps>
        static inline constexpr bool dependent_value = Val;
        
    public:
        java_field_id() noexcept = default;
        
        template<typename ClassType>
        java_field_id(JNIEnv * jenv, const java_class<ClassType> & clazz, const char * name,
                        std::enable_if_t<dependent_value<Kind == instance_field, ClassType>> * = nullptr):
            super(java_field_id::get(jenv, clazz.c_ptr(), name, internal::java_field_signature<Type>()))
        {
        }
        
        template<typename ClassType>
        java_field_id(JNIEnv * jenv, const java_class<ClassType> & clazz, const char * name,
                        std::enable_if_t<dependent_value<Kind == static_field, ClassType>> * = nullptr):
            super(java_field_id::get_static(jenv, clazz.c_ptr(), name, internal::java_field_signature<Type>()))
        {
        }
    };
    
    template<typename Type, typename ThisType>
    class java_field
    {
    private:
        typedef java_field_id<instance_field, Type> id_type;
        typedef java_type_traits<Type> traits;
    public:
        typedef typename traits::return_type return_type;
    public:
        java_field() = default;
        java_field(JNIEnv * jenv, const java_class<ThisType> & clazz, const char* name):
            m_id(jenv, clazz, name)
        {
        }
        
        return_type get(JNIEnv * jenv, typename java_type_traits<ThisType>::arg_type object) const
        {
            Type ret = traits::get_field(jenv, argument_to_java(object), this->m_id.get());
            if (!ret)
                java_exception::check(jenv);
            return return_value_from_java(jenv, ret);
        }
        
        void set(JNIEnv * jenv, typename java_type_traits<ThisType>::arg_type object, typename java_type_traits<Type>::arg_type val) const
        {
            traits::set_field(jenv, argument_to_java(object), this->m_id.get(), argument_to_java(val));
            java_exception::check(jenv);
        }
    private:
        id_type m_id;
    };
    
    template<typename Type, typename ClassType>
    class java_static_field
    {
    private:
        typedef java_field_id<static_field, Type> id_type;
        typedef java_type_traits<Type> traits;
    public:
        typedef typename traits::return_type return_type;
    public:
        java_static_field() = default;
        
        java_static_field(JNIEnv * jenv, const java_class<ClassType> & clazz, const char* name):
            m_id(jenv, clazz, name)
        {
        }
        
        return_type get(JNIEnv * jenv, const java_class<ClassType> & clazz) const
        {
            Type ret = traits::get_static_field(jenv, clazz.c_ptr(), this->m_id.get());
            if (!ret)
                java_exception::check(jenv);
            return return_value_from_java(jenv, ret);
        }
        
        void set(JNIEnv * jenv, const java_class<ClassType> & clazz, typename java_type_traits<Type>::arg_type val) const
        {
            traits::set_static_field(jenv, clazz.c_ptr(), this->m_id.get(), argument_to_java(val));
            java_exception::check(jenv);
        }
    private:
        id_type m_id;
    };
}

#endif //HEADER_JAVA_FIELD_H_INCLUDED
