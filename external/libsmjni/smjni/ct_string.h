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

#ifndef HEADER_CT_StRING_H_INCLUDED
#define HEADER_CT_StRING_H_INCLUDED

#include <utility>
#include <cassert>
#include <type_traits>

namespace smjni
{
    namespace internal
    {
        template<int N>
        class string_ref_impl
        {
        public:
            constexpr char operator[](int i) const
            {
                return assert(i >= 0 && i < N), m_ref[i];
            }
            
            constexpr auto data() const -> const char (&)[N + 1]
                { return m_ref; }

            constexpr const char * c_str() const
                { return &m_ref[0]; }
        protected:
            constexpr string_ref_impl(const char (&literal)[N + 1]):
                m_ref((assert(literal[N] == '\0'), literal))
            {}
            
        private:
            const char (&m_ref)[N + 1];
        };

        template<int N>
        class string_own_impl
        {
        public:
            constexpr char operator[](int i) const
            {
                return assert(i >= 0 && i < N), m_array[i];
            }
            
            constexpr auto data() const -> const char (&)[N + 1]
                { return m_array; }

            constexpr const char * c_str() const
                { return &m_array[0]; }
        protected:
            template <int N1, template<int> class Src1, template<int> class Src2, int... PACK1, int... PACK2>
            constexpr string_own_impl(const Src1<N1>&     s1,
                                    const Src2<N - N1>& s2,
                                    std::integer_sequence<int, PACK1...>,
                                    std::integer_sequence<int, PACK2...>):
                m_array{ s1[PACK1]..., s2[PACK2]..., '\0' }
            {
            }
            
            template<template<int> class Src, class Transformer, int... PACK>
            constexpr string_own_impl(const Src<N> & s, Transformer tr, std::integer_sequence<int, PACK...>):
                m_array{ tr(s[PACK])..., '\0' }
            {
            }
            
        private:
            char m_array[N + 1];
        };
        
        template<int N, template<int> class Impl>
        class string_array : public Impl<N>
        {
        private:
            using super = Impl<N>;
        public:
            constexpr string_array(const char (&literal)[N + 1]):
                super(literal)
            {}
            
            constexpr std::size_t size() const
                { return N; }
            
            template<int N1, int N2, template<int> class Impl1, template<int> class Impl2>
            constexpr string_array(const string_array<N1,Impl1> & s1, const string_array<N2, Impl2> & s2):
                super(s1.impl(), s2.impl(),
                      std::make_integer_sequence<int, N1>(),
                      std::make_integer_sequence<int, (assert(N2 == N - N1),N2)>() )
            {
            }
            
            template<class Transformer, template<int> class Impl1>
            constexpr string_array(const string_array<N,Impl1> & s, Transformer tr):
                super{s.impl(), tr, std::make_integer_sequence<int, N>() }
            {
            }
            
            constexpr const super & impl() const
                { return (const super &)*this; }
        };
        

        template<int N>
        string_array(const char (&literal)[N]) -> string_array<N - 1, string_ref_impl>;

        template<int N>
        constexpr auto make_string_array(const char (&literal)[N]) -> string_array<N - 1, string_ref_impl>
        {
            return string_array<N - 1, string_ref_impl>(literal);
        }
        
        template <int N1, int N2, template<int> class Impl1, template<int> class Impl2>
        constexpr string_array<N1 + N2, string_own_impl> operator+(const string_array<N1, Impl1>& s1, const string_array<N2, Impl2>& s2)
        {
            return string_array<N1 + N2, string_own_impl>(s1, s2);
        }

        template<class Transformer, int N, template<int> class Impl>
        constexpr string_array<N, string_own_impl> transform(const string_array<N, Impl> & s, Transformer tr)
        {
            return string_array<N, string_own_impl>(s, tr);
        }
    }
}

#endif