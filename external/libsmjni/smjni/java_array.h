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

#ifndef HEADER_JAVA_ARRAY_H_INCLUDED
#define HEADER_JAVA_ARRAY_H_INCLUDED

#include "jni_provider.h"
#include "java_ref.h"

namespace smjni
{
    template<typename T>
    class java_array_access_base
    {
    public:
        java_array_access_base(const java_array_access_base &) = delete;
        java_array_access_base & operator=(const java_array_access_base &) = delete;

        constexpr jsize size() const noexcept
        {
            return m_length;
        }
        constexpr bool empty() const noexcept
        {
            return m_length == 0;
        }
    protected:
        java_array_access_base(JNIEnv * env, const auto_java_ref<T> & array):
            m_env(env),
            m_array(array.c_ptr()),
            m_length(m_array ? m_env->GetArrayLength(m_array) : 0)
        {
            java_exception::check(env);
        }
        
    protected:
        JNIEnv * const m_env = nullptr;
        const T m_array = nullptr;
        const jsize m_length = 0;
    };


    template<typename T, bool IsObject = std::is_convertible<typename java_type_traits<T>::element_type, jobject>::value>
    class java_array_access;
    
    template<typename T, bool IsObject = std::is_convertible<typename java_type_traits<T>::element_type, jobject>::value>
    java_array_access(JNIEnv * env, T array) -> java_array_access<T, IsObject>;

    template<typename T, typename Traits, bool IsObject = std::is_convertible<typename java_type_traits<T>::element_type, jobject>::value> 
    java_array_access(JNIEnv * env, const java_ref<T, Traits> & array) -> java_array_access<T, IsObject>;

    template<typename T>
    class java_array_access<T, /*is_object*/ true> : public java_array_access_base<T>
    {
    public:
        typedef typename java_type_traits<T>::element_type element_type;
        
        class const_iterator;
        class iterator;
    private:
        
        class proxy
        {
            friend class const_iterator;
            friend class iterator;
            friend class java_array_access;
        public:
            operator local_java_ref<element_type>() const
            {
                return jattach(m_parent->m_env, m_parent->get(m_idx));
            }
            void operator=(element_type el)
            {
                m_parent->set(m_idx, el);
            }
            void operator=(const local_java_ref<element_type> & el)
            {
                m_parent->set(m_idx, el.c_ptr());
            }
            friend void swap(proxy lhs, proxy rhs) noexcept //proxy is a "reference", this is a swap for referrent
            {
                local_java_ref<element_type> temp = lhs;
                lhs = local_java_ref<element_type>(rhs);
                rhs = temp;
            }
        private:
            proxy() noexcept:
                m_parent(nullptr),
                m_idx(0)
            {}
            proxy(java_array_access & parent, jsize idx) noexcept:
                m_parent(&parent),
                m_idx(idx)
            {}
            
        private:
            java_array_access * const m_parent;
            jsize m_idx;
        };
    public:
        class iterator 
        {
            friend class const_iterator;
            friend class java_array_access;
        public:
            typedef std::random_access_iterator_tag  iterator_category;
            typedef local_java_ref<element_type>  value_type;
            typedef std::make_signed_t<jsize>  difference_type;
            typedef void pointer;
            typedef proxy reference;
        public:
            iterator() noexcept = default;
            
            proxy operator*() const
            {
                return proxy(*m_parent, m_idx);
            }
            proxy operator[](jsize dist) const
            {
                return proxy(*m_parent, m_idx + dist);
            }
            iterator & operator++() noexcept
            {
                ++m_idx;
                return *this;
            }
            iterator & operator--() noexcept
            {
                --m_idx;
                return *this;
            }
            iterator & operator+=(jsize dist) noexcept
            {
                m_idx += dist;
                return *this;
            }
            iterator & operator-=(jsize dist) noexcept
            {
                m_idx -= dist;
                return *this;
            }
            iterator operator++(int) noexcept
            {
                const_iterator ret(*this);
                ++m_idx;
                return ret;
            }
            iterator operator--(int) noexcept
            {
                const_iterator ret(*this);
                --m_idx;
                return ret;
            }
            iterator operator+(jsize dist) const noexcept
            {
                return iterator(*m_parent, m_idx + dist);
            }
            iterator operator-(jsize dist) const noexcept
            {
                return iterator(*m_parent, m_idx - dist);
            }
            jsize operator-(const iterator & rhs) const noexcept
            {
                return m_idx - rhs.m_idx;
            }

            void swap(iterator & other) noexcept
            {
                std::swap(m_parent, other.m_parent);
                std::swap(m_idx, other.m_idx);
            }
            friend void swap(iterator & lhs, iterator & rhs) noexcept
            {
                lhs.swap(rhs);
            }
            
            bool operator==(const iterator & rhs) const noexcept
            {
                return m_idx == rhs.m_idx;
            }
            bool operator!=(const iterator & rhs) const noexcept
            {
                return !(*this == rhs);
            }
            bool operator<(const iterator & rhs) const noexcept
            {
                return m_idx < rhs.m_idx;
            }
            bool operator<=(const iterator & rhs) const noexcept
            {
                return m_idx <= rhs.m_idx;
            }
            bool operator>(const iterator & rhs) const noexcept
            {
                return !(*this <= rhs);
            }
            bool operator>=(const iterator & rhs) const noexcept
            {
                return !(*this < rhs);
            }
        private:
            iterator(java_array_access & parent, jsize idx) noexcept:
                m_parent(&parent), 
                m_idx(idx)
            {}
        private:
            java_array_access * m_parent = nullptr;
            jsize m_idx = 0;
        };
        
        class const_iterator 
        {
            friend class java_array_access;
        public:
            typedef std::random_access_iterator_tag  iterator_category;
            typedef local_java_ref<element_type>  value_type;
            typedef std::make_signed_t<jsize>  difference_type;
            typedef void pointer;
            typedef local_java_ref<element_type> reference;
        public:
            const_iterator() noexcept = default;
            
            const_iterator(const iterator & it) noexcept:
                m_parent(it.m_parent),
                m_idx(it.m_idx)
            {}
            void swap(const_iterator & other) noexcept
            {
                std::swap(m_parent, other.m_parent);
                std::swap(m_idx, other.m_idx);
            }
            friend constexpr void swap(const_iterator & lhs, const_iterator & rhs) noexcept
            {
                lhs.swap(rhs);
            }

            local_java_ref<element_type> operator*() const
            {
                return jattach(m_parent->m_env, m_parent->get(m_idx));
            }
            local_java_ref<element_type> operator[](jsize dist) const
            {
                return jattach(m_parent->m_env, m_parent->get(m_idx + dist));
            }
            const_iterator & operator++() noexcept
            {
                ++m_idx;
                return *this;
            }
            const_iterator & operator--() noexcept
            {
                --m_idx;
                return *this;
            }
            const_iterator & operator+=(jsize dist) noexcept
            {
                m_idx += dist;
                return *this;
            }
            const_iterator & operator-=(jsize dist) noexcept
            {
                m_idx -= dist;
                return *this;
            }
            const_iterator operator++(int) noexcept
            {
                const_iterator ret(*this);
                ++m_idx;
                return ret;
            }
            const_iterator operator--(int) noexcept
            {
                const_iterator ret(*this);
                --m_idx;
                return ret;
            }
            const_iterator operator+(jsize dist) const noexcept
            {
                return const_iterator(m_parent, m_idx + dist);
            }
            const_iterator operator-(jsize dist) const noexcept
            {
                return const_iterator(m_parent, m_idx - dist);
            }
            jsize operator-(const const_iterator & rhs) const noexcept
            {
                return m_idx - rhs.m_idx;
            }

            bool operator==(const const_iterator & rhs) const noexcept
            {
                return m_idx == rhs.m_idx;
            }
            bool operator!=(const const_iterator & rhs) const noexcept
            {
                return !(*this == rhs);
            }
            bool operator<(const iterator & rhs) const noexcept
            {
                return m_idx < rhs.m_idx;
            }
            bool operator<=(const iterator & rhs) const noexcept
            {
                return m_idx <= rhs.m_idx;
            }
            bool operator>(const iterator & rhs) const noexcept
            {
                return !(*this <= rhs);
            }
            bool operator>=(const iterator & rhs) const noexcept
            {
                return !(*this < rhs);
            }
        private:
            const_iterator(const java_array_access & parent, jsize idx) noexcept:
                m_parent(&parent), 
                m_idx(idx)
            {}
        private:
            const java_array_access * m_parent = nullptr;
            jsize m_idx = 0;
        };
        
        typedef std::reverse_iterator<iterator> reverse_iterator;
        typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
        
        typedef jsize size_type;
        typedef jlong difference_type;
        
        typedef local_java_ref<element_type> value_type;
        typedef proxy reference;
        typedef local_java_ref<element_type> const_reference;
        typedef void pointer;
        typedef void const_pointer;
    public:
        java_array_access(JNIEnv * env, const auto_java_ref<T> & array):
            java_array_access_base<T>(env, array)
        {
        }
        
        const_iterator begin() const noexcept
        {
            return const_iterator(*this, 0);
        }
        iterator begin() noexcept
        {
            return iterator(*this, 0);
        }
        const_iterator cbegin() const noexcept
        {
            return const_iterator(*this, 0);
        }
        const_iterator end() const noexcept
        {
            return const_iterator(*this, this->m_length);
        }
        iterator end() noexcept
        {
            return iterator(*this, this->m_length);
        }
        const_iterator cend() const noexcept
        {
            return const_iterator(*this, this->m_length);
        }
        const_reverse_iterator rbegin() const noexcept
        {
            return const_reverse_iterator(*this, this->m_length);
        }
        reverse_iterator rbegin() noexcept
        {
            return reverse_iterator(*this, this->m_length);
        }
        const_reverse_iterator crbegin() const noexcept
        {
            return const_reverse_iterator(*this, this->m_length);
        }
        const_reverse_iterator rend() const noexcept
        {
            return const_reverse_iterator(*this, 0);
        }
        reverse_iterator rend() noexcept
        {
            return reverse_iterator(*this, 0);
        }
        const_reverse_iterator crend() const noexcept
        {
            return const_reverse_iterator(*this, 0);
        }
        
        local_java_ref<element_type> operator[](jsize idx) const
        {
            return jattach(this->m_env, get(idx));
        }
        proxy operator[](jsize idx) noexcept
        {
            return proxy(*this, idx);
        }
        local_java_ref<element_type> at(jsize idx) const
        {
            if (idx < 0 || idx >= this->m_length)
                throw std::out_of_range("index out of range");
            return (*this)[idx];
        }
        proxy at(jsize idx) noexcept
        {
            if (idx < 0 || idx >= this->m_length)
                throw std::out_of_range("index out of range");
            return (*this)[idx];
        }
        local_java_ref<element_type> front() const
        {
            return (*this)[0];
        }
        proxy front() noexcept
        {
            return (*this)[0];
        }
        local_java_ref<element_type> back() const
        {
            return (*this)[this->m_length - 1];
        }
        proxy back() noexcept
        {
            return (*this)[this->m_length - 1];
        }
        
    
    private:
        element_type get(jsize index) const
        {
            element_type ret = static_cast<element_type>(this->m_env->GetObjectArrayElement(this->m_array, index));
            if (!ret)
                java_exception::check(this->m_env);
            return ret;
        }
        
        void set(jsize index, element_type value)
        {
            this->m_env->SetObjectArrayElement(this->m_array, index, value);
            java_exception::check(this->m_env);
        }
    };
        
    template<typename T>
    class java_array_access<T, /*is_object*/ false> : public java_array_access_base<T>
    {
    public:
        typedef typename java_type_traits<T>::element_type element_type;
        
        typedef element_type * iterator;
        typedef const element_type * const_iterator;
        typedef std::reverse_iterator<iterator> reverse_iterator;
        typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
        
        typedef jsize size_type;
        typedef std::make_signed_t<jlong> difference_type;
        
        typedef element_type value_type;
        typedef element_type & reference;
        typedef element_type & const_reference;
        typedef element_type * pointer;
        typedef const element_type * const_pointer;
    public:
        java_array_access(JNIEnv * env, const auto_java_ref<T> & array):
            java_array_access_base<T>(env, array),
            m_data(array ? java_type_traits<T>::get_array_elements(env, array.c_ptr(), nullptr) : nullptr)
        {
            if (array && !this->m_data)
            {
                java_exception::check(env);
                THROW_JAVA_PROBLEM("cannot access java array");
            }
        }
        
        java_array_access(const java_array_access &) = delete;
        
        java_array_access(java_array_access && src) noexcept:
            java_array_access_base<T>(std::move(src)),
            m_data(src.m_data)
        {
            src.m_data = nullptr;
        }
        ~java_array_access()
        {
            if (this->m_data)
                java_type_traits<T>::release_array_elements(this->m_env, this->m_array, this->m_data, JNI_ABORT);
        }
        
        void commit(bool done = true)
        {
            if (done)
            {
                java_type_traits<T>::release_array_elements(this->m_env, this->m_array, this->m_data, 0);
                m_data = nullptr;
            }
            else
            {
                java_type_traits<T>::release_array_elements(this->m_env, this->m_array, this->m_data, JNI_COMMIT);
            }
        }
        
        const element_type * begin() const noexcept
        {
            return this->m_data;
        }
        element_type * begin() noexcept
        {
            return this->m_data;
        }
        const element_type * cbegin() const noexcept
        {
            return this->m_data;
        }
        const element_type * end() const noexcept
        {
            return this->m_data + this->m_length;
        }
        element_type * end() noexcept
        {
            return this->m_data + this->m_length;
        }
        const element_type * cend() const noexcept
        {
            return this->m_data + this->m_length;
        }
        const_reverse_iterator rbegin() const noexcept
        {
            return this->m_data + this->m_length;
        }
        reverse_iterator rbegin() noexcept
        {
            return this->m_data + this->m_length;
        }
        const_reverse_iterator crbegin() const noexcept
        {
            return this->m_data + this->m_length;
        }
        const_reverse_iterator rend() const noexcept
        {
            return this->m_data;
        }
        reverse_iterator rend() noexcept
        {
            return this->m_data;
        }
        const_reverse_iterator crend() const noexcept
        {
            return this->m_data;
        }
        
        const element_type & operator[](jsize idx) const noexcept
        {
            return this->m_data[idx];
        }
        element_type & operator[](jsize idx) noexcept
        {
            return this->m_data[idx];
        }
        const element_type & at(jsize idx) const noexcept
        {
            if (idx < 0 || idx >= this->m_length)
                throw std::out_of_range("index out of range");
            return this->m_data[idx];
        }
        element_type & at(jsize idx) noexcept
        {
            if (idx < 0 || idx >= this->m_length)
                throw std::out_of_range("index out of range");
            return this->m_data[idx];
        }
        const element_type & front() const noexcept
        {
            return this->m_data[0];
        }
        element_type & front() noexcept
        {
            return this->m_data[0];
        }
        const element_type & back() const noexcept
        {
            return this->m_data[this->m_length - 1];
        }
        element_type & back() noexcept
        {
            return this->m_data[this->m_length - 1];
        }
        const element_type * data() const noexcept
        {
            return this->m_data;
        }
        element_type * data() noexcept
        {
            return this->m_data;
        }
    private:
        element_type * m_data = nullptr;
    };
    
    template<typename T>  
    std::enable_if_t<std::is_convertible<T, jobject>::value,
    local_java_ref<java_array_type_of_t<T>>> java_array_create(JNIEnv * env, const java_class<T> & cls, jsize size,
                                                               typename java_type_traits<T>::arg_type initial_value = nullptr)
    {
        auto ret = static_cast<java_array_type_of_t<T>>(env->NewObjectArray(size, cls.c_ptr(), argument_to_java(initial_value)));
        if (!ret)
        {
            java_exception::check(env);
            THROW_JAVA_PROBLEM("cannot create java array");
        }
        return jattach(env, ret);
    }   

    template<typename T>  
    std::enable_if_t<!std::is_convertible<T, jobject>::value,  
    local_java_ref<java_array_type_of_t<T>>> java_array_create(JNIEnv * env, jsize size)
    {
        auto array = java_type_traits<T>::new_array(env, size);
        if (!array)
        {
            java_exception::check(env);
            THROW_JAVA_PROBLEM("cannot create java array");
        }
        return jattach(env, array);
    }   
    
    template<typename T, typename RanIt>  
    std::enable_if_t<!std::is_convertible<T, jobject>::value,  
    local_java_ref<java_array_type_of_t<T>>> java_array_create(JNIEnv * env, RanIt first, RanIt last)
    {
        auto res = java_array_create<T>(env, size_to_java(last - first));
        java_array_access<java_array_type_of_t<T>> res_access(env, res.c_ptr());
        std::copy(first, last, res_access.begin());
        res_access.commit(env);
        return res;
    }

    template<typename T>
    std::enable_if_t<!std::is_convertible<T, jobject>::value,
    void> java_array_set_region(JNIEnv * env, const auto_java_ref<java_array_type_of_t<T>> & array, jsize start, jsize len, const T * buf)
    {
        java_type_traits<T>::set_array_region(env, array.c_ptr(), start, len, buf);
        java_exception::check(env);
    }

    template<typename T>
    std::enable_if_t<!std::is_convertible<T, jobject>::value,
    void> java_array_get_region(JNIEnv * env, const auto_java_ref<java_array_type_of_t<T>> & array, jsize start, jsize len, T * buf)
    {
        java_type_traits<T>::get_array_region(env, array.c_ptr(), start, len, buf);
        java_exception::check(env);
    }
}

#endif //HEADER_JAVA_ARRAY_H_INCLUDED
