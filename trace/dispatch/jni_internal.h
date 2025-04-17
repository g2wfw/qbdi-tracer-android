//
//
//

#ifndef ITRACE_NATIVE_JNI_INTERNAL_H
#define ITRACE_NATIVE_JNI_INTERNAL_H

#include <string>
#include <jni.h>

/*jclass == java/lang/Class */
std::string get_jni_type(JNIEnv *env, jobject java_class);

std::string get_jni_jobject_name(JNIEnv *env, jobject clazz);

std::string get_jni_class_or_java_class_name(JNIEnv *env, jclass clazz);

std::string get_reflected_method_info(JNIEnv *env, jobject method);

std::string get_reflected_field_info(JNIEnv *env, jobject filed);

std::string jstring_to_string(JNIEnv *env, jstring data);

std::string get_jni_method_signature(JNIEnv *env, jmethodID method);

std::string get_jni_field_signature(JNIEnv *env, jfieldID field);

std::string get_jni_object_to_string(JNIEnv *env, jobject obj);

#endif //ITRACE_NATIVE_JNI_INTERNAL_H
