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

#ifndef HEADER_JAVA_REF_H_INCLUDED
#define	HEADER_JAVA_REF_H_INCLUDED

#include <utility>
#include <exception>

#include "jni_provider.h"
#include "java_externals.h"
#include "java_cast.h"

namespace smjni
{
    template<typename T, typename Traits>
    class java_ref;
    
    namespace internal
    {
        template<typename Traits, typename T> java_ref<T, Traits> new_java_ref(JNIEnv * env, T ptr);
        template<typename Traits, typename T> java_ref<T, Traits> attach_java_ref(JNIEnv * env, T ptr);
    }
    
    template<typename T, typename Traits>
    class java_ref : private Traits
    {
    friend java_ref<T, Traits> internal::new_java_ref<Traits, T>(JNIEnv * env, T ptr);
    friend java_ref<T, Traits> internal::attach_java_ref<Traits, T>(JNIEnv * env, T ptr);
    template<typename Y, typename YTraits> friend class java_ref;
    private:
        typedef Traits traits;

        template<typename X>
        using allow_conversion_from = typename Traits:: template allow_conversion_from<X>;
    public:
        java_ref() noexcept:
            traits(nullptr),
            m_obj(nullptr)
        {}

        java_ref(std::nullptr_t) noexcept:
            traits(nullptr),
            m_obj(nullptr)
        {}

        
        
        template<typename X>
        java_ref(X * ptr, std::enable_if_t<allow_conversion_from<X *>::value &&
                                           is_java_castable_v<X *, T>, void *> = nullptr) noexcept:
            traits(nullptr),
            m_obj(this->new_ref(jstatic_cast<T>(ptr)))
        {}
            
        java_ref(const java_ref & src) noexcept:
            traits(src),
            m_obj(this->new_ref(src.m_obj))
        {}
        
        template<typename Y, typename YTraits>
        java_ref(const java_ref<Y, YTraits> & src, std::enable_if_t<is_java_castable_v<Y, T>, void*> = nullptr) noexcept:
            traits(src),
            m_obj(this->new_ref(jstatic_cast<T>(src.c_ptr())))
        {}
        
        java_ref(java_ref && src) noexcept:
            traits(std::move(static_cast<traits &>(src))),
            m_obj(src.m_obj)
        {
            src.m_obj = nullptr;
        }
        template<typename Y>
        java_ref(java_ref<Y, Traits> && src, std::enable_if_t<is_java_castable_v<Y, T>, void*> = nullptr) noexcept:
            traits(std::move(static_cast<traits &>(src))),
            m_obj(jstatic_cast<T>(src.m_obj))
        {
            src.m_obj = nullptr;
        }

        java_ref & operator=(const java_ref & src) 
        {
            java_ref(src).swap(*this);
            return *this;
        }
        template<typename Y, typename YTraits>
        std::enable_if_t < is_java_castable_v<Y, T>, 
        java_ref> & operator=(const java_ref<Y, YTraits> & src)
        {
            java_ref(src).swap(*this);
            return *this;
        }
        java_ref & operator=(java_ref && src) noexcept
        {
            this->delete_ref();
            this->m_obj = nullptr;
            static_cast<traits &>(*this) = std::move(static_cast<traits &>(src));
            this->m_obj = src.m_obj;
            src.m_obj = nullptr;
            return *this;
        }
        template<typename Y>
        std::enable_if_t < is_java_castable_v<Y, T>,
        java_ref> & operator=(java_ref<Y, Traits> && src) noexcept
        {
            this->delete_ref();
            this->m_obj = nullptr;
            static_cast<traits &>(*this) = std::move(static_cast<traits &>(src));
            this->m_obj = jstatic_cast<T>(src.m_obj);
            src.m_obj = nullptr;
            return *this;
        }
        ~java_ref() noexcept
            { this->delete_ref(); }
    
        void swap(java_ref & other) noexcept
        { 
            using std::swap;
            
            swap(static_cast<traits &>(*this), static_cast<traits &>(other));
            swap(m_obj, other.m_obj); 
        }
            
        decltype(auto) c_ptr() const noexcept 
            { return traits::c_ptr(this->m_obj); }
        
        explicit operator bool() const noexcept
            { return this->m_obj != nullptr; }
        
        decltype(auto) release() noexcept 
        {
            T ret = this->m_obj;
            this->m_obj = 0;
            return traits::c_ptr(ret);
        }
        
    private:
        enum attach_tag { attach };
        
        java_ref(JNIEnv * env, T obj, attach_tag) noexcept:
            traits(env),
            m_obj(obj)
        {}
        java_ref(JNIEnv * env, T obj) noexcept:
            traits(env),
            m_obj(java_ref::new_ref(obj))
        {}
    private:
        T new_ref(T obj) noexcept
        {
            try
            {
                return static_cast<T>(traits::new_ref(obj));
            }
            catch(std::exception & ex)
            {
                internal::do_log_error(ex, nullptr);
                std::terminate();
            }
        }
        void delete_ref() noexcept
        {
            try
            {
                traits::delete_ref(this->m_obj);
            }
            catch(std::exception & ex)
            {
                internal::do_log_error(ex, nullptr);
            }
        }
    private:
        T m_obj;     
    };
    
    template<typename T, typename Traits>
    inline
    void swap(java_ref<T, Traits> & lhs, java_ref<T, Traits> & rhs) noexcept
    {
        lhs.swap(rhs);
    }
    
    namespace internal
    {
        struct java_ref_traits
        {
        };

        struct auto_ref_traits : public java_ref_traits
        {
            //MSVC needs dependent constant for enable_if
            template<class X>
            using allow_conversion_from = std::is_same<X, X>; //dependent true
            
            auto_ref_traits(JNIEnv * env = nullptr) noexcept
            {}
            auto_ref_traits(const java_ref_traits & ) noexcept
            {}

            static jobject new_ref(jobject obj) noexcept
                { return obj; }
            static void delete_ref(jobject obj) noexcept
                {  }
            template<typename T>
            static T c_ptr(T obj) noexcept
                { return obj; }

            friend void swap(auto_ref_traits & lhs, auto_ref_traits & rhs) noexcept
                { }
        };

        class local_ref_traits : public java_ref_traits
        {
        public:
            //MSVC needs dependent constant for enable_if
            template<class X>
            using allow_conversion_from = std::is_same<X *, class nonexistent>; //dependent false
            
            local_ref_traits(JNIEnv * env = nullptr) noexcept : m_env(env)
            {}
            local_ref_traits(const java_ref_traits & ):
                m_env(jni_provider::get_jni())
            {}

            jobject new_ref(jobject obj) noexcept
                { return obj ? m_env->NewLocalRef(obj) : nullptr; }
            void delete_ref(jobject obj) noexcept
                { if (obj) m_env->DeleteLocalRef(obj); }
            template<typename T>
            static T c_ptr(T obj) noexcept
                { return obj; }

            friend void swap(local_ref_traits & lhs, local_ref_traits & rhs) noexcept
                { std::swap(lhs.m_env, rhs.m_env); }
        private:
            JNIEnv * m_env;
        };

        struct global_ref_traits : public java_ref_traits
        {
            //MSVC needs dependent constant for enable_if
            template<class X>
            using allow_conversion_from = std::is_same<X*, class nonexistent>; //dependent false
            
            global_ref_traits(JNIEnv * env = nullptr)
            {}
            global_ref_traits(const java_ref_traits & ) noexcept
            {}

            static jobject new_ref(jobject obj) 
                { return obj ? jni_provider::get_jni()->NewGlobalRef(obj) : nullptr; }
            static void delete_ref(jobject obj) 
                { if (obj) jni_provider::get_jni()->DeleteGlobalRef(obj); }
            template<typename T>
            static T c_ptr(T obj) noexcept
                { return obj; }

            friend void swap(global_ref_traits & lhs, global_ref_traits & rhs) noexcept
            {}
        };

        struct weak_ref_traits : public java_ref_traits
        {
            //MSVC needs dependent constant for enable_if
            template<class X>
            using allow_conversion_from = std::is_same<X*, class nonexistent>; //dependent false
            
            weak_ref_traits(JNIEnv * env = nullptr)
            {}
            weak_ref_traits(const java_ref_traits & ) noexcept
            {}

            static jweak new_ref(jobject obj)
                { return obj ? jni_provider::get_jni()->NewWeakGlobalRef(obj) : nullptr; }
            static void delete_ref(jobject obj)
                { if (obj) jni_provider::get_jni()->DeleteWeakGlobalRef(obj); }
            template<typename T>
            static jweak c_ptr(T obj) noexcept
                { return obj; }

            friend void swap(weak_ref_traits & lhs, weak_ref_traits & rhs) noexcept
            {}
        };

        template<typename Traits, typename T>
        inline java_ref<T, Traits> new_java_ref(JNIEnv * env, T ptr)
        {
            return java_ref<T, Traits>(env, ptr);
        }

        template<typename Traits, typename T>
        inline java_ref<T, Traits> attach_java_ref(JNIEnv * env, T ptr)
        {
            return java_ref<T, Traits>(env, ptr, java_ref<T, Traits>::attach);
        }
    }
    
    
    template<typename T>
    using auto_java_ref = java_ref<T, internal::auto_ref_traits>;
    template<typename T>
    using local_java_ref = java_ref<T, internal::local_ref_traits>;
    template<typename T>
    using global_java_ref = java_ref<T, internal::global_ref_traits>;
    template<typename T>
    using weak_java_ref = java_ref<T, internal::weak_ref_traits>;
    
    template<typename T>
    inline auto_java_ref<T> jauto(T ptr) noexcept
        { return auto_java_ref<T>(ptr); }
    
    template<typename T>
    inline local_java_ref<T> jref(JNIEnv * env, T ptr) noexcept
        { return internal::new_java_ref<internal::local_ref_traits>(env, ptr); }
    template<typename T>
    inline local_java_ref<T> jattach(JNIEnv * env, T ptr) noexcept
        { return internal::attach_java_ref<internal::local_ref_traits>(env, ptr); }
    
    template<typename T>
    inline global_java_ref<T> jglobal_ref(T ptr)
        { return internal::new_java_ref<internal::global_ref_traits>(nullptr, ptr); }
    template<typename T>
    inline global_java_ref<T> jglobal_attach(T ptr)
        { return internal::attach_java_ref<internal::global_ref_traits>(nullptr, ptr); }
    
    template<typename T>
    inline weak_java_ref<T> jweak_ref(T ptr)
        { return internal::new_java_ref<internal::weak_ref_traits>(nullptr, ptr); }
    template<typename T>
    inline weak_java_ref<T> jweak_attach(T ptr)
        { return internal::attach_java_ref<internal::weak_ref_traits>(nullptr, ptr); }
    
}

#endif	//HEADER_JAVA_REF_H_INCLUDED


