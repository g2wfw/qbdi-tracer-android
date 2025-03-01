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

#ifndef HEADER_JAVA_CAST_H_INCLUDED
#define	HEADER_JAVA_CAST_H_INCLUDED

#include <type_traits>

namespace smjni
{
    template<typename Dest, typename Source>
    inline
    std::enable_if_t<std::is_convertible_v<typename std::remove_pointer_t<Source>,
                                           typename std::remove_pointer_t<jobject>> 
                     &&
                     std::is_convertible_v<typename std::remove_pointer_t<Source>,
                                           typename std::remove_pointer_t<Dest>>,
    Dest> jstatic_cast(Source src) noexcept
    {
        return src;
    }

    template<typename Dest>
    inline
    std::enable_if_t<std::is_convertible_v<typename std::remove_pointer_t<Dest>, 
                                           typename std::remove_pointer_t<jobject>>,
    Dest> jstatic_cast(jobject src) noexcept
    {
        return static_cast<Dest>(src);
    }

    template<class From, class To, class Enable = void>
    struct is_java_castable: std::false_type {};

    template<class From, class To>
    struct is_java_castable<From, To, std::void_t<decltype(jstatic_cast<To>((From)nullptr))>> : std::true_type {};

    template<class From, class To>
    constexpr bool is_java_castable_v = is_java_castable<From, To>::value;
}

#define DEFINE_JAVA_CONVERSION(T1, T2) \
    namespace smjni\
    {\
        inline T1 jstatic_cast(T2 src)\
        {\
            return static_cast<T1>(static_cast<jobject>(src));\
        }\
        inline T2 jstatic_cast(T1 src)\
        {\
            return static_cast<T2>(static_cast<jobject>(src));\
        }\
    }


#endif
