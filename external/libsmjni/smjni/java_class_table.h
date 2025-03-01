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

#ifndef HEADER_JAVA_CLASS_TABLE_H_INCLUDED
#define HEADER_JAVA_CLASS_TABLE_H_INCLUDED

#include "config.h"

namespace smjni
{
    template<typename... Classes>
    class java_class_table
    {
    private:
        template<typename... T>
        static JNIEnv * tuple_forward(JNIEnv * env)
            { return env; }
        
        template <typename Func, typename... T>
        static void tuple_for_each(std::tuple<T...> &ts, Func func) 
        {
            int unused[] = {(func(std::get<T>(ts)),0)...};
            (void)unused;
        }
        
        template <template<typename> class Transform, typename... T>
        struct tuple_transform
        {
            typedef std::tuple<Transform<T>...> type;
        };
        
        class registrator
        {
        public:
            registrator(JNIEnv * env):
                m_env(env)
            {}

            template<typename X>
            void operator()(const X & cls) const
            {
                if constexpr (can_register<X>)
                {
                    cls.register_methods(m_env);
                }
            }

        private:
            template <typename T> static std::true_type can_register_helper( decltype(&T::register_methods) );
            template <typename T> static std::false_type can_register_helper(...);
            
            template<typename T>
            static constexpr bool can_register = decltype(can_register_helper<T>(nullptr))::value;

            JNIEnv * const m_env;
        };
        
        struct table : public std::tuple<Classes...>
        {
            table(JNIEnv * env):
                std::tuple<Classes...>(tuple_forward<Classes>(env)...)
            {
                tuple_for_each(*this, registrator(env));
            }
        };
    public:
        static void init(JNIEnv * env)
        {
            s_instance = new java_class_table(env);
        }
        static void term()
        {
            delete s_instance;
        }

        template<typename T>
        static const T & get()
        {
            return std::get<T>(s_instance->m_table);
        }
        
        template <template<typename> class Transform>
        using transformed_type = typename tuple_transform<Transform, Classes...>::type;
    private:
        java_class_table(JNIEnv * env):
            m_table(env)
        {
        }
    private:
        table m_table;
        
        static java_class_table * s_instance;
    };
    
    template<typename... Classes>
    java_class_table<Classes...> * java_class_table<Classes...>::s_instance = nullptr;
}

#endif
