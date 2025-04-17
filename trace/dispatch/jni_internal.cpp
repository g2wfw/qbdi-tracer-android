//
//
//

#include <cstring>
#include <cstdlib>
#include "jni.h"
#include "jni_internal.h"


std::string jstring_to_string(JNIEnv *env, jstring data) {
    if (data == nullptr) {
        return "";
    }
    const char *c_str = env->GetStringUTFChars(data, nullptr);
    if (c_str == nullptr) {
        return "";
    }
    struct UtfCharsReleaser {
        JNIEnv *env;
        jstring str;
        const char *chars;

        ~UtfCharsReleaser() {
            if (chars != nullptr) {
                env->ReleaseStringUTFChars(str, chars);
            }
        }
    } releaser{env, data, c_str};
    std::string result(c_str);
    return result;
}

std::string get_jni_class_or_java_class_name(JNIEnv *env, jclass clazz) {
    if (clazz == nullptr) {
        return "nullptr";
    }
    auto java_Class_clazz = env->FindClass("java/lang/Class");
    jmethodID getName_mid = env->GetMethodID(java_Class_clazz, "getName", "()Ljava/lang/String;");
    env->DeleteLocalRef(java_Class_clazz);
    jstring classNameJString = (jstring) env->CallObjectMethod(clazz, getName_mid);
    auto result = jstring_to_string(env, classNameJString);
    env->DeleteLocalRef(classNameJString);
    if (env->ExceptionCheck()) {
        env->ExceptionDescribe();
    }
    return result;
}

std::string get_jni_type(JNIEnv *env, jobject java_class) {
    if (java_class == nullptr) {
        return "";
    }
    jclass java_class_clazz = env->GetObjectClass(java_class);
    jmethodID is_array_method = env->GetMethodID(java_class_clazz, "isArray", "()Z");
    if (env->ExceptionCheck()) {
        env->ExceptionDescribe();
    }
    if (env->CallBooleanMethod(java_class, is_array_method)) {
        std::string result = "[";
        //getComponentType
        jmethodID getComponentType = env->GetMethodID(java_class_clazz, "getComponentType",
                                                      "()Ljava/lang/Class;");
        jclass java_class_ = static_cast<jclass>(env->CallObjectMethod(java_class,
                                                                       getComponentType));
        result.append(get_jni_type(env, java_class_));
        env->DeleteLocalRef(java_class_);
        env->DeleteLocalRef(java_class_clazz);
        return result;
    }
    if (env->ExceptionCheck()) {
        env->ExceptionDescribe();
    }
    jmethodID isPrimitive = env->GetMethodID(java_class_clazz, "isPrimitive", "()Z");
    if (env->ExceptionCheck()) {
        env->ExceptionDescribe();
    }
    if (env->CallBooleanMethod(java_class, isPrimitive)) {
        auto name = get_jni_class_or_java_class_name(env, (jclass) java_class);
        if (name == "byte") {
            return "B";
        } else if (name == "short") {
            return "S";
        } else if (name == "int") {
            return "I";
        } else if (name == "boolean") {
            return "Z";
        } else if (name == "char") {
            return "C";
        } else if (name == "long") {
            return "J";
        } else if (name == "float") {
            return "F";
        } else if (name == "double") {
            return "D";
        } else if (name == "void") {
            return "V";
        } else if (name == "java.lang.String") {
            return "Ljava/lang/String;";
        } else if (name == "java.lang.Object") {
            return "Ljava/lang/Object;";
        } else if (name == "java.lang.Class") {
            return "Ljava/lang/Class;";
        } else if (name == "java.lang.Throwable") {
            return "Ljava/lang/Throwable;";
        } else {
            //return "L" + clazz.getName().replace('.', '/') + ";";
            std::replace(name.begin(), name.end(), '.', '/');
            return "L" + name + ";";
        }
    }
    return "";
}


std::string get_reflected_method_info(JNIEnv *env, jobject method) {
    if (method == nullptr) {
        return "nullptr";
    }
    std::string info = "";
    jclass reflected_method_class = env->GetObjectClass(method);
    std::string class_name = get_jni_class_or_java_class_name(env, reflected_method_class);
    jmethodID reflected_method_mid_get_name = env->GetMethodID(reflected_method_class, "getName",
                                                               "()Ljava/lang/String;");
    jstring method_name_jstring = static_cast<jstring>(env->CallObjectMethod(method,
                                                                             reflected_method_mid_get_name));
    info.append(jstring_to_string(env, method_name_jstring));
    env->DeleteLocalRef(method_name_jstring);
    if (env->ExceptionCheck()) {
        env->ExceptionDescribe();
    }
    info.append("(");
    jmethodID reflected_method_mid_get_parameter_types = env->GetMethodID(reflected_method_class,
                                                                          "getParameterTypes",
                                                                          "()[Ljava/lang/Class;");
    jobjectArray parameter_types_jarray = static_cast<jobjectArray>(env->CallObjectMethod(method,
                                                                                          reflected_method_mid_get_parameter_types));

    for (int i = 0; i < env->GetArrayLength(parameter_types_jarray); i++) {
        jclass java_class = static_cast<jclass>(env->GetObjectArrayElement(parameter_types_jarray,
                                                                           i));
        info.append(get_jni_type(env, java_class));
        env->DeleteLocalRef(java_class);
    }
    if (env->ExceptionCheck()) {
        env->ExceptionDescribe();
    }
    info.append(")");
    //判断是否为构造函数
    if (class_name == "java.lang.reflect.Constructor") {
        info.append("V");
    } else {
        jmethodID returnType = env->GetMethodID(reflected_method_class, "getReturnType",
                                                "()Ljava/lang/Class;");
        jobject ret = env->CallObjectMethod(method, returnType);

        info.append(get_jni_type(env, ret));
        env->DeleteLocalRef(ret);
    }

    return info;
}

std::string get_jni_jobject_name(JNIEnv *env, jobject object) {
    if (object == nullptr) {
        return "nullptr";
    }
    //check is java.lang.Class
    jclass java_lang_Class_clazz = env->FindClass("java/lang/Class");
    if (env->IsInstanceOf(object, java_lang_Class_clazz)) {
        env->DeleteLocalRef(java_lang_Class_clazz);
        return get_jni_class_or_java_class_name(env, (jclass) object);
    }
    env->DeleteLocalRef(java_lang_Class_clazz);
    jclass clazz = env->GetObjectClass(object);;
    auto result = get_jni_class_or_java_class_name(env, clazz);
    env->DeleteLocalRef(clazz);
    if (env->ExceptionCheck()) {
        env->ExceptionDescribe();
    }
    return result;
}


std::string get_jni_method_signature(JNIEnv *env, jmethodID method) {
    if (method == nullptr || env == nullptr) {
        return "";
    }
    auto java_Class_clazz = env->FindClass("java/lang/Class");
    auto method_object = env->ToReflectedMethod(java_Class_clazz, method, false);
    if (env->ExceptionCheck()) {
        env->ExceptionDescribe();
    }
    if (method_object== nullptr){
        return "";
    }
    env->DeleteLocalRef(java_Class_clazz);
    auto res = get_reflected_method_info(env, method_object);
    env->DeleteLocalRef(method_object);
    if (env->ExceptionCheck()) {
        env->ExceptionDescribe();
    }
    return res;
}

std::string get_reflected_field_info(JNIEnv *env, jobject filed) {
    if (filed == nullptr) {
        return "nullptr";
    }
    std::string info = "";
    jclass reflected_method_class = env->GetObjectClass(filed);
    jmethodID reflected_method_mid_get_name = env->GetMethodID(reflected_method_class, "getName",
                                                               "()Ljava/lang/String;");
    jstring field_name_jstring = static_cast<jstring>(env->CallObjectMethod(filed,
                                                                            reflected_method_mid_get_name));
    info.append(jstring_to_string(env, field_name_jstring));
    env->DeleteLocalRef(field_name_jstring);

    jmethodID reflected_method_mid_get_type = env->GetMethodID(reflected_method_class, "getType",
                                                               "()Ljava/lang/Class;");
    jobject java_class = (env->CallObjectMethod(filed,
                                                reflected_method_mid_get_type));

    info.append(":");
    info.append(get_jni_type(env, java_class));
    env->DeleteLocalRef(java_class);
    if (env->ExceptionCheck()) {
        env->ExceptionDescribe();
    }
    return info;
}

std::string get_jni_field_signature(JNIEnv *env, jfieldID field) {
    if (field == nullptr) {
        return "nullptr";
    }
    auto java_Class_clazz = env->FindClass("java/lang/Class");
    auto filed_object = env->ToReflectedField(java_Class_clazz, field, false);
    auto res = get_reflected_field_info(env, filed_object);
    env->DeleteLocalRef(filed_object);
    env->DeleteLocalRef(java_Class_clazz);
    if (env->ExceptionCheck()) {
        env->ExceptionDescribe();
    }
    return res;
}

std::string get_jni_object_to_string(JNIEnv *env, jobject obj) {
    if (obj == nullptr) return "";  // 处理空对象

    // 获取 Object 类
    jclass objectClass = env->GetObjectClass(obj);
    if (!objectClass) return "";

    // 获取 toString() 方法 ID
    jmethodID toStringMethod = env->GetMethodID(objectClass, "toString", "()Ljava/lang/String;");
    if (!toStringMethod) return "";

    // 调用 toString() 方法
    jstring jstr = (jstring) env->CallObjectMethod(obj, toStringMethod);
    if (!jstr) return "";

    // 转换 jstring 到 C++ std::string
    const char *chars = env->GetStringUTFChars(jstr, nullptr);
    std::string result(chars);
    env->ReleaseStringUTFChars(jstr, chars);  // 释放资源
    if (env->ExceptionCheck()) {
        env->ExceptionDescribe();
    }
    return result;
}