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

#ifndef HEADER_JAVA_TYPE_TRAITS_H_INCLUDED
#define HEADER_JAVA_TYPE_TRAITS_H_INCLUDED


#include "java_ref.h"
#include "java_types.h"
#include "java_cast.h"
#include "java_cast.h"
#include "ct_string.h"

namespace smjni
{

    SMJNI_FORCE_INLINE jsize size_to_java(size_t size)
    {
        if (sizeof(size_t) >= sizeof(jsize))
        {
            if (size > size_t(std::numeric_limits<jsize>::max()))
                std::terminate();
            
        }
        return jsize(size);
    }

    SMJNI_FORCE_INLINE size_t java_size_to_cpp(jsize size)
    {
        if (size < 0)
            std::terminate();
        if (sizeof(size_t) < sizeof(jsize))
        {
            if (size_t(size) > std::numeric_limits<size_t>::max())
                std::terminate();
            
        }
        return size_t(size);
    }

    template<typename T> SMJNI_FORCE_INLINE constexpr T argument_to_java(T val) noexcept
        { return val; }
    template<typename T> SMJNI_FORCE_INLINE T argument_to_java(const auto_java_ref<T> & val) noexcept
        { return val.c_ptr(); }

    template<typename T> SMJNI_FORCE_INLINE constexpr T return_value_from_java(JNIEnv *, T val) noexcept
        { return val; }
    template<typename T> SMJNI_FORCE_INLINE local_java_ref<T *> return_value_from_java(JNIEnv * env, T * val) noexcept
        { return jattach(env, val); }

    //Allows uniform handling of void return
    struct VoidResult
    {
        //allows uniform handling of if (!ret)
        operator bool() const noexcept
        { return false; }
    };

    SMJNI_FORCE_INLINE void return_value_from_java(JNIEnv * env, VoidResult val) noexcept
        { }

    template<size_t N>
    constexpr inline decltype(auto) object_signature_from_name(const char (&name)[N])
    {
        using internal::string_array;
        return string_array("L") + transform(string_array(name), [](char c) {return (c != '.' ? c : '/');}) + string_array(";");
    }

    template<size_t N>
    constexpr inline decltype(auto) array_signature_from_name(const char (&name)[N])
    {
        using internal::string_array;
        return string_array("[") + object_signature_from_name(name);
    }
    

    template<typename T>
    class java_type_traits;

    template<typename T>
    class java_array_type_of;

    template<typename T>
    using java_array_type_of_t = typename java_array_type_of<T>::type;

    template<>
    class java_type_traits<void>
    {
    public:
        typedef void return_type;

        java_type_traits() = delete;
        java_type_traits(const java_type_traits &) = delete;
        java_type_traits & operator=(const java_type_traits &) = delete;

        static constexpr decltype(auto) signature()
        {
            return internal::string_array("V");
        }

        static VoidResult call_method(JNIEnv * jenv, jobject object, jmethodID method, ...)
        {
            va_list vl;
            va_start(vl, method);
            jenv->CallVoidMethodV(object, method, vl);
            va_end(vl);
            return {};
        }

        static VoidResult call_static_method(JNIEnv * jenv, jclass clazz, jmethodID method, ...)
        {
            va_list vl;
            va_start(vl, method);
            jenv->CallStaticVoidMethodV(clazz, method, vl);
            va_end(vl);
            return {};
        }

        static VoidResult call_non_virtual_method(JNIEnv * jenv, jobject object, jclass clazz, jmethodID method, ...)
        {
            va_list vl;
            va_start(vl, method);
            jenv->CallNonvirtualVoidMethodV(object, clazz, method, vl);
            va_end(vl);
            return {};
        }
    };
}

#define HANDLE_PRIMITIVE_JAVA_TYPE(jtype, name, sig) \
    namespace smjni \
    {\
        template<> \
        class java_type_traits<jtype> \
        { \
        public: \
            typedef jtype return_type; \
            typedef jtype arg_type; \
            \
            java_type_traits() = delete; \
            java_type_traits(const java_type_traits &) = delete; \
            java_type_traits & operator=(const java_type_traits &) = delete; \
            \
            static constexpr decltype(auto) signature() \
            { \
                return internal::string_array(sig); \
            } \
            \
            static jtype call_method(JNIEnv * jenv, jobject object, jmethodID method, ...) \
            { \
                va_list vl; \
                va_start(vl, method); \
                jtype ret = jenv->Call##name##MethodV(object, method, vl); \
                va_end(vl); \
                return ret;\
            }\
            \
            static jtype call_static_method(JNIEnv * jenv, jclass clazz, jmethodID method, ...)\
            {\
                va_list vl;\
                va_start(vl, method);\
                jtype ret = jenv->CallStatic##name##MethodV(clazz, method, vl);\
                va_end(vl);\
                return ret;\
            }\
            \
            static jtype call_non_virtual_method(JNIEnv * jenv, jobject object, jclass clazz, jmethodID method, ...)\
            {\
                va_list vl;\
                va_start(vl, method);\
                jtype ret = jenv->CallNonvirtual##name##MethodV(object, clazz, method, vl);\
                va_end(vl);\
                return ret;\
            }\
            static jtype get_field(JNIEnv * jenv, jobject object, jfieldID field)\
            {\
               jtype ret = jenv->Get##name##Field(object, field);\
               return ret;\
            }\
            static void set_field(JNIEnv * jenv, jobject object, jfieldID field, jtype val)\
            {\
               jenv->Set##name##Field(object, field, val);\
            }\
            static jtype get_static_field(JNIEnv * jenv, jclass clazz, jfieldID field)\
            {\
               jtype ret = jenv->GetStatic##name##Field(clazz, field);\
               return ret;\
            }\
            static void set_static_field(JNIEnv * jenv, jclass clazz, jfieldID field, jtype val)\
            {\
               jenv->SetStatic##name##Field(clazz, field, val);\
            }\
            static jtype##Array new_array(JNIEnv * jenv, jsize size)\
            {\
               return jenv->New##name##Array(size);\
            }\
            static void set_array_region(JNIEnv * jenv, jtype##Array array, jsize start, jsize len, const jtype * buf)\
            {\
                jenv->Set##name##ArrayRegion(array, start, len, buf);\
            }\
            static void get_array_region(JNIEnv * jenv, jtype##Array array, jsize start, jsize len, jtype * buf)\
            {\
                jenv->Get##name##ArrayRegion(array, start, len, buf);\
            }\
        };\
    }

HANDLE_PRIMITIVE_JAVA_TYPE(jboolean,    Boolean,    "Z");
HANDLE_PRIMITIVE_JAVA_TYPE(jbyte,       Byte,       "B");
HANDLE_PRIMITIVE_JAVA_TYPE(jchar,       Char,       "C");
HANDLE_PRIMITIVE_JAVA_TYPE(jshort,      Short,      "S");
HANDLE_PRIMITIVE_JAVA_TYPE(jint,        Int,        "I");
HANDLE_PRIMITIVE_JAVA_TYPE(jlong,       Long,       "J");
HANDLE_PRIMITIVE_JAVA_TYPE(jfloat,      Float,      "F");
HANDLE_PRIMITIVE_JAVA_TYPE(jdouble,     Double,     "D");

#undef HANDLE_PRIMITIVE_JAVA_TYPE

namespace smjni
{
    template<typename T>
    class java_object_type_base
    {
    public:
        typedef local_java_ref<T> return_type;
        typedef const auto_java_ref<T> & arg_type; 

        java_object_type_base() = delete;
        java_object_type_base(const java_object_type_base &) = delete;
        java_object_type_base & operator=(const java_object_type_base &) = delete;

        static T call_method(JNIEnv * jenv, jobject object, jmethodID method, ...)
        {
            va_list vl;
            va_start(vl, method);
            T ret = static_cast<T>(jenv->CallObjectMethodV(object, method, vl)); 
            va_end(vl);
            return ret;
        }

        static T call_static_method(JNIEnv * jenv, jclass clazz, jmethodID method, ...)
        {
            va_list vl;
            va_start(vl, method);
            T ret = static_cast<T>(jenv->CallStaticObjectMethodV(clazz, method, vl));
            va_end(vl);
            return ret;
        }

        static T call_non_virtual_method(JNIEnv * jenv, jobject object, jclass clazz, jmethodID method, ...)
        {
            va_list vl;
            va_start(vl, method);
            T ret = static_cast<T>(jenv->CallNonvirtualObjectMethodV(object, clazz, method, vl));
            va_end(vl);
            return ret;
        }

        static T get_field(JNIEnv * jenv, jobject object, jfieldID field)
        {
            T ret = static_cast<T>(jenv->GetObjectField(object, field));
            return ret;
        }

        static void set_field(JNIEnv * jenv, jobject object, jfieldID field, T val)
        {
            jenv->SetObjectField(object, field, val);
        }

        static T get_static_field(JNIEnv * jenv, jclass clazz, jfieldID field)
        {
            T ret = static_cast<T>(jenv->GetStaticObjectField(clazz, field));
            return ret;
        }

        static void set_static_field(JNIEnv * jenv, jclass clazz, jfieldID field, T val)
        {
            jenv->SetStaticObjectField(clazz, field, val);
        }

        static T new_object(JNIEnv * jenv, jclass clazz, jmethodID method, ...)
        {
            va_list vl;
            va_start(vl, method);
            T ret = static_cast<T>(jenv->NewObjectV(clazz, method, vl));
            va_end(vl);
            return ret;
        }
    };
}

#define HANDLE_OBJECT_JAVA_TYPE(jtype, name) \
    namespace smjni\
    {\
        template<> \
        class java_type_traits<jtype> : public java_object_type_base<jtype> \
        {\
        public:\
            static constexpr decltype(auto) signature() \
            {\
                return object_signature_from_name(name); \
            }\
            \
            static constexpr decltype(auto) class_name() \
            {\
                return name;\
            }\
        };\
    }

HANDLE_OBJECT_JAVA_TYPE(jobject,     "java.lang.Object");
HANDLE_OBJECT_JAVA_TYPE(jstring,     "java.lang.String");
HANDLE_OBJECT_JAVA_TYPE(jthrowable,  "java.lang.Throwable");
HANDLE_OBJECT_JAVA_TYPE(jclass,      "java.lang.Class");

#define HANDLE_PRIMITIVE_ARRAY_JAVA_TYPE(jtype, name, sig) \
    namespace smjni \
    {\
        template<> \
        class java_type_traits<jtype##Array> : public java_object_type_base<jtype##Array> \
        {\
        public:\
            typedef jtype element_type;\
            \
            static constexpr decltype(auto) signature() \
            {\
                return internal::string_array(sig); \
            }\
            static void release_array_elements(JNIEnv * env, jtype##Array ar, jtype * data, jint flags)\
            {\
                env->Release##name##ArrayElements(ar, data, flags);\
            }\
            static jtype * get_array_elements(JNIEnv * env, jtype##Array ar, jboolean * copied)\
            {\
                return env->Get##name##ArrayElements(ar, copied);\
            }\
        };\
        template<>\
        class java_array_type_of<jtype>\
        {\
            public: typedef jtype##Array type;\
        };\
    }

HANDLE_PRIMITIVE_ARRAY_JAVA_TYPE(jboolean,    Boolean,    "[Z");
HANDLE_PRIMITIVE_ARRAY_JAVA_TYPE(jbyte,       Byte,       "[B");
HANDLE_PRIMITIVE_ARRAY_JAVA_TYPE(jchar,       Char,       "[C");
HANDLE_PRIMITIVE_ARRAY_JAVA_TYPE(jshort,      Short,      "[S");
HANDLE_PRIMITIVE_ARRAY_JAVA_TYPE(jint,        Int,        "[I");
HANDLE_PRIMITIVE_ARRAY_JAVA_TYPE(jlong,       Long,       "[J");
HANDLE_PRIMITIVE_ARRAY_JAVA_TYPE(jfloat,      Float,      "[F");
HANDLE_PRIMITIVE_ARRAY_JAVA_TYPE(jdouble,     Double,     "[D");

#undef HANDLE_PRIMITIVE_ARRAY_JAVA_TYPE

#define HANDLE_OBJECT_ARRAY_JAVA_TYPE(jtype) \
    namespace smjni \
    {\
        template<> \
        class java_type_traits<jtype##Array> : public java_object_type_base<jtype##Array> \
        {\
        public:\
            typedef jtype element_type;\
            \
            static constexpr decltype(auto) signature() \
            {\
                return array_signature_from_name(java_type_traits<jtype>::class_name()); \
            }\
        };\
        template<>\
        class java_array_type_of<jtype>\
        {\
            public: typedef jtype##Array type;\
        };\
    }

HANDLE_OBJECT_ARRAY_JAVA_TYPE(jobject);


#define DEFINE_JAVA_TYPE(type, name) \
        class _##type : public _jobject {};\
        typedef _##type * type;\
        HANDLE_OBJECT_JAVA_TYPE(type, name)

#define DEFINE_ARRAY_JAVA_TYPE(type) \
        class _##type##Array : public _jobjectArray {};\
        typedef _##type##Array * type##Array;\
        HANDLE_OBJECT_ARRAY_JAVA_TYPE(type)



#endif //HEADER_JAVA_TYPE_TRAITS_H_INCLUDED
