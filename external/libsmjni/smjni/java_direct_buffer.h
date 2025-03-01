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

#ifndef HEADER_JAVA_DIRECT_BUFFER_H_INCLUDED
#define HEADER_JAVA_DIRECT_BUFFER_H_INCLUDED

#include "java_ref.h"
#include "java_exception.h"

#include <stdexcept>

DEFINE_JAVA_TYPE(jByteBuffer, "java.nio.ByteBuffer")

namespace smjni
{
    template<typename T>
    class java_direct_buffer
    {
    public:
        typedef T * iterator;
        typedef const T * const_iterator;
        typedef std::reverse_iterator<iterator> reverse_iterator;
        typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
        
        typedef jlong size_type;
        typedef std::make_signed_t<jlong> difference_type;
        
        typedef T value_type;
        typedef T & reference;
        typedef T & const_reference;
        typedef T * pointer;
        typedef const T * const_pointer;
        
    public:
        constexpr java_direct_buffer(T * ptr, jlong size):
            m_ptr(ptr),
            m_size(size)  
        {
        }
            
        java_direct_buffer(JNIEnv * env, const auto_java_ref<jByteBuffer> & obj)
        {
            this->m_ptr = static_cast<T *>(env->GetDirectBufferAddress(obj.c_ptr()));
            if (!this->m_ptr)
            {
                java_exception::check(env); 
                THROW_JAVA_PROBLEM("invalid buffer");
            }
            jlong byte_size = env->GetDirectBufferCapacity(obj.c_ptr());
            if (byte_size == -1)
            {
                java_exception::check(env); 
                THROW_JAVA_PROBLEM("invalid buffer");
            }
            this->m_size = byte_size / jlong(sizeof(T));
        }
        
        local_java_ref<jByteBuffer> to_java(JNIEnv * env)
        {
            jobject ret = env->NewDirectByteBuffer(this->m_ptr, this->m_size * sizeof(T));
            if (!ret)
            {
                java_exception::check(env);
                THROW_JAVA_PROBLEM("cannot create java buffer");
            }
            return jattach(env, ret);
        } 
        
        constexpr const value_type * begin() const noexcept
        {
            return this->m_ptr;
        }
        constexpr value_type * begin() noexcept
        {
            return this->m_ptr;
        }
        constexpr const value_type * cbegin() const noexcept
        {
            return this->m_ptr;
        }
        constexpr const value_type * end() const noexcept
        {
            return this->m_ptr + this->m_size;
        }
        constexpr value_type * end() noexcept
        {
            return this->m_ptr + this->m_size;
        }
        constexpr const value_type * cend() const noexcept
        {
            return this->m_ptr + this->m_size;
        }
        constexpr const_reverse_iterator rbegin() const noexcept
        {
            return this->m_ptr + this->m_size;
        }
        constexpr reverse_iterator rbegin() noexcept
        {
            return this->m_ptr + this->m_size;
        }
        constexpr const_reverse_iterator crbegin() const noexcept
        {
            return this->m_ptr + this->m_size;
        }
        constexpr const_reverse_iterator rend() const noexcept
        {
            return this->m_ptr;
        }
        constexpr reverse_iterator rend() noexcept
        {
            return this->m_ptr;
        }
        constexpr const_reverse_iterator crend() const noexcept
        {
            return this->m_ptr;
        }
        constexpr jlong size() const noexcept
        {
            return this->m_size;
        }
        constexpr bool empty() const noexcept
        {
            return this->m_size == 0;
        }
        constexpr const value_type & operator[](jsize idx) const noexcept
        {
            return this->m_ptr[idx];
        }
        constexpr value_type & operator[](jsize idx) noexcept
        {
            return this->m_ptr[idx];
        }
        constexpr const value_type & at(jsize idx) const noexcept
        {
            if (idx < 0 || idx >= this->m_size)
                throw std::out_of_range("index out of range");
            return this->m_ptr[idx];
        }
        constexpr value_type & at(jsize idx) noexcept
        {
            if (idx < 0 || idx >= this->m_size)
                throw std::out_of_range("index out of range");
            return this->m_ptr[idx];
        }
        constexpr const value_type & front() const noexcept
        {
            return this->m_ptr[0];
        }
        constexpr value_type & front() noexcept
        {
            return this->m_ptr[0];
        }
        constexpr const value_type & back() const noexcept
        {
            return this->m_ptr[this->m_size - 1];
        }
        constexpr value_type & back() noexcept
        {
            return this->m_ptr[this->m_size - 1];
        }
        constexpr const value_type * data() const noexcept
        {
            return this->m_ptr;
        }
        constexpr value_type * data() noexcept
        {
            return this->m_ptr;
        }
        constexpr void swap(java_direct_buffer & other) noexcept
        {
            std::swap(this->m_ptr, other.m_ptr);
            std::swap(this->m_size, other.m_size);
        }
        friend constexpr void swap(java_direct_buffer & lhs, java_direct_buffer & rhs) noexcept
        {
            lhs.swap(rhs);
        }
    private:
        T * m_ptr = nullptr;
        jlong m_size = 0;    
    };
}

#endif
