//
//
//

#include <jni.h>
#include <spdlog/fmt/fmt.h>
#include <smjni.h>
#include <core/library.h>
#include "dispatch_jni_env.h"
#include "jni_internal.h"
#include "core/logging/check.h"
#include <core/containers/stl_Collections.h>

static std::unordered_map<uintptr_t, std::string> env_fun_table;

DispatchJNIEnv *DispatchJNIEnv::get_instance() {
    static DispatchJNIEnv dispatchJniEnv;
    return &dispatchJniEnv;
}


DispatchJNIEnv::DispatchJNIEnv() {
    init_env_fun_table();
    auto module = stl::Library::find_library("libart.so");
    auto range = module->get_library_range();
    module_range.base = range.start();
    module_range.end = range.end();
    module->enumerate_exports(dispatch_export_func, this);
}


inline bool starts_with(const std::string &str, const std::string &prefix) {
    return str.size() >= prefix.size() && str.compare(0, prefix.size(), prefix) == 0;
}

inline bool ends_with(const std::string &str, const std::string &suffix) {
    if (suffix.size() > str.size())
        return false;
    return std::equal(suffix.rbegin(), suffix.rend(), str.rbegin());
}

static void FindClass(inst_trace_info_t *trace_info) {
    auto *env = reinterpret_cast<JNIEnv *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 0));
    const char *name = reinterpret_cast<const char *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 1));
    //LOGI("FindClass: name=%s", name);
    trace_info->fun_call->args.push_back(fmt::format("name={}", name));
}

static void FromReflectedMethod(inst_trace_info_t *trace_info) {
    auto *env = reinterpret_cast<JNIEnv *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 0));
    auto method = reinterpret_cast<jobject>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 1));
    auto info = get_reflected_method_info(env, method);
    //LOGI("FromReflectedMethod: method=%s", info.c_str());
    trace_info->fun_call->args.push_back(fmt::format("method={} {:#x}", info, (uintptr_t) method));
}

static void FromReflectedField(inst_trace_info_t *trace_info) {
    auto *env = reinterpret_cast<JNIEnv *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 0));
    auto field = reinterpret_cast<jobject>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 1));
    auto info = get_reflected_field_info(env, field);
    //LOGI("FromReflectedField: field=%s", info.c_str());
    trace_info->fun_call->args.push_back(
            fmt::format("field={}", info));
}

// static jobject ToReflectedField(JNIEnv *env, jclass clazz, jfieldID fid, jboolean isStatic)
static void
ToReflectedField(inst_trace_info_t *trace_info) {
    auto *env = reinterpret_cast<JNIEnv *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 0));
    auto clazz = reinterpret_cast<jclass>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 1));
    auto fid = reinterpret_cast<jfieldID>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 2));
    jboolean isStatic = DispatchBase::get_arg_register_value(&trace_info->pre_status, 3) & 0xff;
    auto clazz_name = get_jni_class_or_java_class_name(env, clazz);
    auto field_sign = get_jni_field_signature(env, fid);
    //LOGI("ToReflectedField: clazz=%s, fid=%s, isStatic=%d", clazz_name.c_str(), field_sign.c_str(), isStatic);
    trace_info->fun_call->args.push_back(fmt::format("clazz={}", clazz_name));
    trace_info->fun_call->args.push_back(fmt::format("fid={}", field_sign));
    trace_info->fun_call->args.push_back(fmt::format("isStatic={}", isStatic));
}

static void
ToReflectedMethod(inst_trace_info_t *trace_info) {
    auto *env = reinterpret_cast<JNIEnv *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 0));
    auto clazz = reinterpret_cast<jclass>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 1));
    jmethodID mid = reinterpret_cast<jmethodID>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 2));
    jboolean isStatic = DispatchBase::get_arg_register_value(&trace_info->pre_status, 3) & 0xff;

    auto class_name = get_jni_class_or_java_class_name(env, clazz);
    auto method_sign = get_jni_method_signature(env, mid);
    //LOGI("ToReflectedMethod: clazz=%s, mid=%s, isStatic=%d", class_name.c_str(), method_sign.c_str(), isStatic);
    trace_info->fun_call->args.push_back(fmt::format("clazz={}", class_name));
    trace_info->fun_call->args.push_back(fmt::format("mid={}", method_sign));
    trace_info->fun_call->args.push_back(fmt::format("isStatic={}", isStatic));
}

//static jclass GetObjectClass(JNIEnv *env, jobject java_object)
static void GetObjectClass(inst_trace_info_t *trace_info) {
    auto *env = reinterpret_cast<JNIEnv *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 0));
    auto java_object = reinterpret_cast<jobject>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 1));
    auto object_name = get_jni_jobject_name(env, java_object);
    //LOGI("GetObjectClass: java_object=%s", object_name.c_str());
    trace_info->fun_call->args.push_back(fmt::format("object={} {:#x}", object_name, (uintptr_t) java_object));
}

// static jclass GetSuperclass(JNIEnv *env, jclass java_class)
static void GetSuperclass(inst_trace_info_t *trace_info) {
    auto *env = reinterpret_cast<JNIEnv *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 0));
    auto java_class = reinterpret_cast<jclass>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 1));
    auto clazz_info = get_jni_class_or_java_class_name(env, java_class);
    //LOGI("GetSuperclass: java_class=%s %p", clazz_info.c_str(), java_class);
    trace_info->fun_call->args.push_back(fmt::format("java_class={} {:#x}", clazz_info, (uintptr_t) java_class));
}

//static jboolean IsAssignableFrom(JNIEnv *env, jclass java_class1, jclass java_class2)
static void IsAssignableFrom(inst_trace_info_t *trace_info) {
    auto *env = reinterpret_cast<JNIEnv *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 0));
    auto java_class1 = reinterpret_cast<jclass>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 1));
    auto java_class2 = reinterpret_cast<jclass>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 2));
    auto clazz1_info = get_jni_class_or_java_class_name(env, java_class1);
    auto clazz2_info = get_jni_class_or_java_class_name(env, java_class2);
    //LOGI("IsAssignableFrom: java_class1=%s, java_class2=%s", clazz1_info.c_str(), clazz2_info.c_str());
    trace_info->fun_call->args.push_back(fmt::format("java_class1={} {:#x}", clazz1_info, (uintptr_t) java_class1));
    trace_info->fun_call->args.push_back(fmt::format("java_class2={} {:#x}", clazz2_info, (uintptr_t) java_class2));
}

//static jint Throw(JNIEnv *env, jthrowable java_exception)
static void Throw(inst_trace_info_t *trace_info) {
    auto *env = reinterpret_cast<JNIEnv *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 0));
    auto java_exception = reinterpret_cast<jthrowable>(DispatchBase::get_arg_register_value(&trace_info->pre_status,
                                                                                            1));
    auto java_exception_msg = get_jni_jobject_name(env, java_exception);
    //LOGI("Throw: java_exception=%s", java_exception_msg.c_str());
    trace_info->fun_call->args.push_back(fmt::format("java_exception={} {:#x}", java_exception_msg,
                                                     (uintptr_t) java_exception));
}

//static jint ThrowNew(JNIEnv *env, jclass c, const char *msg) {
static void ThrowNew(inst_trace_info_t *trace_info) {
    auto *env = reinterpret_cast<JNIEnv *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 0));
    auto c = reinterpret_cast<jclass>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 1));
    auto msg = reinterpret_cast<const char *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 2));
    auto clazz_info = get_jni_class_or_java_class_name(env, c);
    //LOGI("ThrowNew: java_class=%s, msg=%s", clazz_info.c_str(), msg);
    trace_info->fun_call->args.push_back(fmt::format("java_class={} {:#x}", clazz_info, (uintptr_t) c));
    trace_info->fun_call->args.push_back(fmt::format("msg={}", msg));
}

//static jint PushLocalFrame(JNIEnv *env, jint capacity)
static void PushLocalFrame(inst_trace_info_t *trace_info) {
    auto *env = reinterpret_cast<JNIEnv *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 0));
    auto capacity = DispatchBase::get_arg_register_value(&trace_info->pre_status, 1);
    //LOGI("PushLocalFrame: capacity=%lu", capacity);
    trace_info->fun_call->args.push_back(fmt::format("capacity={}", capacity));
}

//static jobject PopLocalFrame(JNIEnv *env, jobject java_survivor) {
static void PopLocalFrame(inst_trace_info_t *trace_info) {
    auto *env = reinterpret_cast<JNIEnv *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 0));
    auto java_survivor = reinterpret_cast<jobject>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 1));
    auto object_name = get_jni_jobject_name(env, java_survivor);
    //LOGI("PopLocalFrame: java_survivor=%s", object_name.c_str());
    trace_info->fun_call->args.push_back(fmt::format("java_survivor={} {:#x}", object_name, (uintptr_t) java_survivor));
}

//static jobject NewGlobalRef(JNIEnv *env, jobject obj)
static void NewGlobalRef(inst_trace_info_t *trace_info) {
    auto *env = reinterpret_cast<JNIEnv *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 0));
    auto obj = reinterpret_cast<jobject>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 1));
    auto object_name = get_jni_jobject_name(env, obj);
    //LOGI("NewGlobalRef: obj=%s", object_name.c_str());
    trace_info->fun_call->args.push_back(fmt::format("obj={} {:#x}", object_name, (uintptr_t) obj));
}

//static void DeleteGlobalRef(JNIEnv *env, jobject obj)
static void DeleteGlobalRef(inst_trace_info_t *trace_info) {
    auto *env = reinterpret_cast<JNIEnv *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 0));
    auto obj = reinterpret_cast<jobject>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 1));
    auto object_name = get_jni_jobject_name(env, obj);
    //LOGI("DeleteGlobalRef: obj=%s", object_name.c_str());
    trace_info->fun_call->args.push_back(fmt::format("obj={} {:#x}", object_name, (uintptr_t) obj));
}

//static void DeleteLocalRef(JNIEnv *env, jobject obj)
static void DeleteLocalRef(inst_trace_info_t *trace_info) {
    auto *env = reinterpret_cast<JNIEnv *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 0));
    auto obj = reinterpret_cast<jobject>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 1));
    auto object_name = get_jni_jobject_name(env, obj);
    //LOGI("DeleteLocalRef: obj=%s", object_name.c_str());
    trace_info->fun_call->args.push_back(fmt::format("obj={} {:#x}", object_name, (uintptr_t) obj));
}

//static jboolean IsSameObject(JNIEnv *env, jobject obj1, jobject obj2)
static void IsSameObject(inst_trace_info_t *trace_info) {
    auto *env = reinterpret_cast<JNIEnv *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 0));
    auto obj1 = reinterpret_cast<jobject>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 1));
    auto obj2 = reinterpret_cast<jobject>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 2));
    auto object_name1 = get_jni_jobject_name(env, obj1);
    auto object_name2 = get_jni_jobject_name(env, obj2);
    //LOGI("IsSameObject: obj1=%s, obj2=%s", object_name1.c_str(), object_name2.c_str());
    trace_info->fun_call->args.push_back(fmt::format("obj1={} {:#x}", object_name1, (uintptr_t) obj1));
    trace_info->fun_call->args.push_back(fmt::format("obj2={} {:#x}", object_name2, (uintptr_t) obj2));
}

//static jobject NewLocalRef(JNIEnv *env, jobject obj)
static void NewLocalRef(inst_trace_info_t *trace_info) {
    auto *env = reinterpret_cast<JNIEnv *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 0));
    auto obj = reinterpret_cast<jobject>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 1));
    auto object_name = get_jni_jobject_name(env, obj);
    //LOGI("NewLocalRef: obj=%s", object_name.c_str());
    trace_info->fun_call->args.push_back(fmt::format("obj={} {:#x}", object_name, (uintptr_t) obj));
}

//static jint EnsureLocalCapacity(JNIEnv *env, jint capacity)
static void EnsureLocalCapacity(inst_trace_info_t *trace_info) {
    auto *env = reinterpret_cast<JNIEnv *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 0));
    auto capacity = DispatchBase::get_arg_register_value(&trace_info->pre_status, 1);
    //LOGI("EnsureLocalCapacity: capacity=%lu", capacity);
    trace_info->fun_call->args.push_back(fmt::format("capacity={}", capacity));
}

//static jobject AllocObject(JNIEnv *env, jclass java_class)
static void AllocObject(inst_trace_info_t *trace_info) {
    auto *env = reinterpret_cast<JNIEnv *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 0));
    auto java_class = reinterpret_cast<jclass>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 1));
    auto clazz_info = get_jni_class_or_java_class_name(env, java_class);
    //LOGI("AllocObject: java_class=%s", clazz_info.c_str());
    trace_info->fun_call->args.push_back(fmt::format("java_class={} {:#x}", clazz_info, (uintptr_t) java_class));
}

// static jobject NewObject(JNIEnv *env, jclass java_class, jmethodID mid, ...)
static void NewObject(inst_trace_info_t *trace_info) {
    auto *env = reinterpret_cast<JNIEnv *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 0));
    auto java_class = reinterpret_cast<jclass>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 1));
    auto mid = reinterpret_cast<jmethodID>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 2));
    auto clazz_info = get_jni_class_or_java_class_name(env, java_class);
    auto method_info = get_jni_method_signature(env, mid);
    //LOGI("NewObject: java_class=%s, mid=%s", clazz_info.c_str(), method_info.c_str());
    trace_info->fun_call->args.push_back(fmt::format("java_class={} {:#x}", clazz_info, (uintptr_t) java_class));
    trace_info->fun_call->args.push_back(fmt::format("mid={} {:#x}", method_info, (uintptr_t) mid));
}

//static jobject NewObjectA(JNIEnv *env, jclass java_class, jmethodID mid, const jvalue *args)
static void NewObjectA(inst_trace_info_t *trace_info) {
    auto *env = reinterpret_cast<JNIEnv *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 0));
    auto java_class = reinterpret_cast<jclass>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 1));
    auto mid = reinterpret_cast<jmethodID>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 2));
    auto clazz_info = get_jni_class_or_java_class_name(env, java_class);
    auto method_info = get_jni_method_signature(env, mid);
    //LOGI("NewObjectA: java_class=%s, mid=%s", clazz_info.c_str(), method_info.c_str());
    trace_info->fun_call->args.push_back(fmt::format("java_class={} {:#x}", clazz_info, (uintptr_t) java_class));
    trace_info->fun_call->args.push_back(fmt::format("mid={} {:#x}", method_info, (uintptr_t) mid));
}

//static jobject NewObjectV(JNIEnv *env, jclass java_class, jmethodID mid, va_list args)
static void NewObjectV(inst_trace_info_t *trace_info) {
    auto *env = reinterpret_cast<JNIEnv *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 0));
    auto java_class = reinterpret_cast<jclass>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 1));
    auto mid = reinterpret_cast<jmethodID>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 2));
    auto clazz_info = get_jni_class_or_java_class_name(env, java_class);
    auto method_info = get_jni_method_signature(env, mid);
    //LOGI("NewObjectV: java_class=%s, mid=%s", clazz_info.c_str(), method_info.c_str());
    trace_info->fun_call->args.push_back(fmt::format("java_class={} {:#x}", clazz_info, (uintptr_t) java_class));
    trace_info->fun_call->args.push_back(fmt::format("mid={} {:#x}", method_info, (uintptr_t) mid));
}

//static jboolean IsInstanceOf(JNIEnv *env, jobject jobj, jclass java_class)
static void IsInstanceOf(inst_trace_info_t *trace_info) {
    auto *env = reinterpret_cast<JNIEnv *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 0));
    auto jobj = reinterpret_cast<jobject>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 1));
    auto java_class = reinterpret_cast<jclass>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 2));
    auto clazz_info = get_jni_class_or_java_class_name(env, java_class);
    auto object_name = get_jni_jobject_name(env, jobj);
    //LOGI("IsInstanceOf: jobj=%s, java_class=%s", object_name.c_str(), clazz_info.c_str());
    trace_info->fun_call->args.push_back(fmt::format("jobj={} {:#x}", object_name, (uintptr_t) jobj));
    trace_info->fun_call->args.push_back(fmt::format("java_class={} {:#x}", clazz_info, (uintptr_t) java_class));
}

//static jmethodID GetMethodID(JNIEnv *env, jclass java_class, const char *name, const char *sig)
static void GetMethodID(inst_trace_info_t *trace_info) {
    auto *env = reinterpret_cast<JNIEnv *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 0));
    auto java_class = reinterpret_cast<jclass>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 1));
    auto name = reinterpret_cast<const char *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 2));
    auto sig = reinterpret_cast<const char *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 3));
    auto clazz_info = get_jni_class_or_java_class_name(env, java_class);
    //LOGI("GetMethodID: java_class=%s, name=%s, sig=%s", clazz_info.c_str(), name, sig);
    trace_info->fun_call->args.push_back(fmt::format("java_class={} {:#x}", clazz_info, (uintptr_t) java_class));
    trace_info->fun_call->args.push_back(fmt::format("name={}", name));
    trace_info->fun_call->args.push_back(fmt::format("sig={}", sig));
}


#define PROXY_CALL_TYPE_METHOD(_jname) \
    static void Call##_jname##Method(inst_trace_info_t *trace_info)\
    {\
        auto *env = reinterpret_cast<JNIEnv *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 0));\
        auto obj = reinterpret_cast<jobject>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 1));\
        auto mid = reinterpret_cast<jmethodID>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 2));\
        auto object_name = get_jni_jobject_name(env, obj);\
        auto method_info = get_jni_method_signature(env, mid);\
        trace_info->fun_call->args.push_back(fmt::format("obj={} {:#x}", object_name, (uintptr_t) obj));\
        trace_info->fun_call->args.push_back(fmt::format("mid={} {:#x}", method_info, (uintptr_t) mid));\
    }
#define PROXY_CALL_TYPE_METHODV(_jname) \
   static void Call##_jname##MethodV(inst_trace_info_t *trace_info) \
   {\
    auto *env = reinterpret_cast<JNIEnv *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 0));\
    auto obj = reinterpret_cast<jobject>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 1));\
    auto mid = reinterpret_cast<jmethodID>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 2));\
    auto object_name = get_jni_jobject_name(env, obj);\
    auto method_info = get_jni_method_signature(env, mid);\
    trace_info->fun_call->args.push_back(fmt::format("obj={} {:#x}", object_name, (uintptr_t) obj));\
    trace_info->fun_call->args.push_back(fmt::format("mid={} {:#x}", method_info, (uintptr_t) mid));\
   }
#define PROXY_CALL_TYPE_METHODA(_jname) \
   static void Call##_jname##MethodA(inst_trace_info_t *trace_info) \
    {\
        auto *env = reinterpret_cast<JNIEnv *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 0));\
        auto obj = reinterpret_cast<jobject>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 1));\
        auto mid = reinterpret_cast<jmethodID>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 2));\
        auto object_name = get_jni_jobject_name(env, obj);\
        auto method_info = get_jni_method_signature(env, mid);\
        trace_info->fun_call->args.push_back(fmt::format("obj={} {:#x}", object_name, (uintptr_t) obj));\
        trace_info->fun_call->args.push_back(fmt::format("mid={} {:#x}", method_info, (uintptr_t) mid));\
    }


#define PROXY_CALL_TYPE(_jname)                                           \
    PROXY_CALL_TYPE_METHOD(_jname)                                        \
    PROXY_CALL_TYPE_METHODV(_jname)                                       \
    PROXY_CALL_TYPE_METHODA(_jname)


PROXY_CALL_TYPE(Object)

PROXY_CALL_TYPE(Boolean)

PROXY_CALL_TYPE(Byte)

PROXY_CALL_TYPE(Char)

PROXY_CALL_TYPE(Short)

PROXY_CALL_TYPE(Int)

PROXY_CALL_TYPE(Long)

PROXY_CALL_TYPE(Float)

PROXY_CALL_TYPE(Double)

PROXY_CALL_TYPE(Void)


//JNIEnv*env,jobject obj, jclass clazz,jmethodID methodID
#define PROXY_CALL_NONVIRT_TYPE_METHOD(_jname) \
   static void CallNonvirtual##_jname##Method(inst_trace_info_t *trace_info) \
    {\
        auto *env = reinterpret_cast<JNIEnv *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 0));\
        auto obj = reinterpret_cast<jobject>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 1));\
        auto clazz = reinterpret_cast<jclass>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 2));\
        auto mid = reinterpret_cast<jmethodID>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 3));\
        auto object_name = get_jni_jobject_name(env, obj);\
        auto clazz_info = get_jni_class_or_java_class_name(env, clazz);\
        auto method_info = get_jni_method_signature(env, mid);\
        trace_info->fun_call->args.push_back(fmt::format("obj={} {:#x}", object_name, (uintptr_t) obj));\
        trace_info->fun_call->args.push_back(fmt::format("clazz={} {:#x}", clazz_info, (uintptr_t) clazz));\
        trace_info->fun_call->args.push_back(fmt::format("mid={} {:#x}", method_info, (uintptr_t) mid));\
    }
#define PROXY_CALL_NONVIRT_TYPE_METHODV(_jname)                                                 \
   static void CallNonvirtual##_jname##MethodV(inst_trace_info_t *trace_info)\
    { \
        auto *env = reinterpret_cast<JNIEnv *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 0));\
        auto obj = reinterpret_cast<jobject>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 1));\
        auto clazz = reinterpret_cast<jclass>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 2));\
        auto mid = reinterpret_cast<jmethodID>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 3));\
        auto object_name = get_jni_jobject_name(env, obj);\
        auto clazz_info = get_jni_class_or_java_class_name(env, clazz);\
        auto method_info = get_jni_method_signature(env, mid);\
        trace_info->fun_call->args.push_back(fmt::format("obj={} {:#x}", object_name, (uintptr_t) obj));\
        trace_info->fun_call->args.push_back(fmt::format("clazz={} {:#x}", clazz_info, (uintptr_t) clazz));\
        trace_info->fun_call->args.push_back(fmt::format("mid={} {:#x}", method_info, (uintptr_t) mid));\
    }

#define PROXY_CALL_NONVIRT_TYPE_METHODA(_jname) \
   static void CallNonvirtual##_jname##MethodA(inst_trace_info_t *trace_info)  \
    {\
        auto *env = reinterpret_cast<JNIEnv *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 0));\
        auto obj = reinterpret_cast<jobject>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 1));\
        auto clazz = reinterpret_cast<jclass>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 2));\
        auto mid = reinterpret_cast<jmethodID>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 3));\
        auto object_name = get_jni_jobject_name(env, obj);\
        auto clazz_info = get_jni_class_or_java_class_name(env, clazz);\
        auto method_info = get_jni_method_signature(env, mid);\
        trace_info->fun_call->args.push_back(fmt::format("obj={} {:#x}", object_name, (uintptr_t) obj));\
        trace_info->fun_call->args.push_back(fmt::format("clazz={} {:#x}", clazz_info, (uintptr_t) clazz));\
        trace_info->fun_call->args.push_back(fmt::format("mid={} {:#x}", method_info, (uintptr_t) mid));\
    }

#define PROXY_CALL_NONVIRT_TYPE(_jname)                                   \
    PROXY_CALL_NONVIRT_TYPE_METHOD( _jname)                                \
    PROXY_CALL_NONVIRT_TYPE_METHODV( _jname)                               \
    PROXY_CALL_NONVIRT_TYPE_METHODA( _jname)

PROXY_CALL_NONVIRT_TYPE(Object)

PROXY_CALL_NONVIRT_TYPE(Boolean)

PROXY_CALL_NONVIRT_TYPE(Byte)

PROXY_CALL_NONVIRT_TYPE(Char)

PROXY_CALL_NONVIRT_TYPE(Short)

PROXY_CALL_NONVIRT_TYPE(Int)

PROXY_CALL_NONVIRT_TYPE(Long)

PROXY_CALL_NONVIRT_TYPE(Float)

PROXY_CALL_NONVIRT_TYPE(Double)

PROXY_CALL_NONVIRT_TYPE(Void)

//static jfieldID GetFieldID(JNIEnv *env, jclass java_class, const char *name, const char *sig)
static void GetFieldID(inst_trace_info_t *trace_info) {
    auto *env = reinterpret_cast<JNIEnv *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 0));
    auto java_class = reinterpret_cast<jclass>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 1));
    auto name = reinterpret_cast<const char *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 2));
    auto sig = reinterpret_cast<const char *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 3));
    auto clazz_info = get_jni_class_or_java_class_name(env, java_class);
    trace_info->fun_call->args.push_back(fmt::format("clazz={} {:#x}", clazz_info, (uintptr_t) java_class));
    trace_info->fun_call->args.push_back(fmt::format("name={}", name));
    trace_info->fun_call->args.push_back(fmt::format("sig={}", sig));
}

//static jobject GetObjectField(JNIEnv *env, jobject obj, jfieldID fid)
#define PROXY_GET_FIELD(_jname) \
    static void Get##_jname##Field(inst_trace_info_t* trace_info) \
    { \
        auto* env = reinterpret_cast<JNIEnv*>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 0));\
        auto obj = reinterpret_cast<jobject>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 1));\
        auto fid = reinterpret_cast<jfieldID>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 2));\
        auto object_name = get_jni_jobject_name(env, obj);\
        auto field_info = get_jni_field_signature(env, fid);\
        trace_info->fun_call->args.push_back(fmt::format("obj={} {:#x}", object_name, (uintptr_t)obj));\
        trace_info->fun_call->args.push_back(fmt::format("fid={} {:#x}", field_info, (uintptr_t)fid));\
    }

PROXY_GET_FIELD(Object)

PROXY_GET_FIELD(Boolean)

PROXY_GET_FIELD(Byte)

PROXY_GET_FIELD(Char)

PROXY_GET_FIELD(Short)

PROXY_GET_FIELD(Int)

PROXY_GET_FIELD(Long)

PROXY_GET_FIELD(Float)

PROXY_GET_FIELD(Double)

PROXY_GET_FIELD(Void)

// void SetObjectField(JNIEnv *env, jobject obj, jfieldID fieldID, jobject value)
static void SetObjectField(inst_trace_info_t *trace_info) {
    auto *env = reinterpret_cast<JNIEnv *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 0));
    auto obj = reinterpret_cast<jobject>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 1));
    auto fid = reinterpret_cast<jfieldID>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 2));
    auto value = reinterpret_cast<jobject>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 3));
    auto object_name = get_jni_jobject_name(env, obj);
    auto field_info = get_jni_field_signature(env, fid);
    auto value_info = get_jni_object_to_string(env, value);
    //LOGI("%s: obj=%s, fid=%s, value=%s", "SetObjectField", object_name.c_str(), field_info.c_str(), value_info.c_str());
    trace_info->fun_call->args.push_back(fmt::format("obj={} {:#x}", object_name, (uintptr_t) obj));
    trace_info->fun_call->args.push_back(fmt::format("fid={} {:#x}", field_info, (uintptr_t) fid));
    trace_info->fun_call->args.push_back(fmt::format("value={} {:#x}", value_info, (uintptr_t) value));
}

// static void SetBooleanField(JNIEnv *env, jobject obj, jfieldID fieldID, jboolean value)
static void SetBooleanField(inst_trace_info_t *trace_info) {
    auto *env = reinterpret_cast<JNIEnv *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 0));
    auto obj = reinterpret_cast<jobject>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 1));
    auto fid = reinterpret_cast<jfieldID>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 2));
    auto value = (jboolean) (DispatchBase::get_arg_register_value(&trace_info->pre_status, 3) & 0xff);
    auto object_name = get_jni_jobject_name(env, obj);
    auto field_info = get_jni_field_signature(env, fid);
    //LOGI("%s: obj=%s, fid=%s, value=%s", "SetBooleanField", object_name.c_str(), field_info.c_str(),
    //value ? "true" : "false");
    trace_info->fun_call->args.push_back(fmt::format("obj={} {:#x}", object_name, (uintptr_t) obj));
    trace_info->fun_call->args.push_back(fmt::format("fid={} {:#x}", field_info, (uintptr_t) fid));
    trace_info->fun_call->args.push_back(fmt::format("value={}", value ? "true" : "false"));
}

static void SetByteField(inst_trace_info_t *trace_info) {
    auto *env = reinterpret_cast<JNIEnv *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 0));
    auto obj = reinterpret_cast<jobject>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 1));
    auto fid = reinterpret_cast<jfieldID>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 2));
    auto value = (jbyte) (DispatchBase::get_arg_register_value(&trace_info->pre_status, 3) & 0xff);
    auto object_name = get_jni_jobject_name(env, obj);
    auto field_info = get_jni_field_signature(env, fid);
    //LOGI("%s: obj=%s, fid=%s, value=%d", "SetByteField", object_name.c_str(), field_info.c_str(), value);
    trace_info->fun_call->args.push_back(fmt::format("obj={} {:#x}", object_name, (uintptr_t) obj));
    trace_info->fun_call->args.push_back(fmt::format("fid={} {:#x}", field_info, (uintptr_t) fid));
    trace_info->fun_call->args.push_back(fmt::format("value={}", value));
}

static void SetCharField(inst_trace_info_t *trace_info) {
    auto *env = reinterpret_cast<JNIEnv *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 0));
    auto obj = reinterpret_cast<jobject>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 1));
    auto fid = reinterpret_cast<jfieldID>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 2));
    auto value = (jchar) (DispatchBase::get_arg_register_value(&trace_info->pre_status, 3) & 0xffff);
    auto object_name = get_jni_jobject_name(env, obj);
    auto field_info = get_jni_field_signature(env, fid);
    //LOGI("%s: obj=%s, fid=%s, value=%d", "SetCharField", object_name.c_str(), field_info.c_str(), value);
    trace_info->fun_call->args.push_back(fmt::format("obj={} {:#x}", object_name, (uintptr_t) obj));
    trace_info->fun_call->args.push_back(fmt::format("fid={} {:#x}", field_info, (uintptr_t) fid));
    trace_info->fun_call->args.push_back(fmt::format("value={}", value));
}

static void SetShortField(inst_trace_info_t *trace_info) {
    auto *env = reinterpret_cast<JNIEnv *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 0));
    auto obj = reinterpret_cast<jobject>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 1));
    auto fid = reinterpret_cast<jfieldID>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 2));
    auto value = (jshort) (DispatchBase::get_arg_register_value(&trace_info->pre_status, 3) & 0xffff);
    auto object_name = get_jni_jobject_name(env, obj);
    auto field_info = get_jni_field_signature(env, fid);
    //LOGI("%s: obj=%s, fid=%s, value=%d", "SetShortField", object_name.c_str(), field_info.c_str(), value);
    trace_info->fun_call->args.push_back(fmt::format("obj={} {:#x}", object_name, (uintptr_t) obj));
    trace_info->fun_call->args.push_back(fmt::format("fid={} {:#x}", field_info, (uintptr_t) fid));
    trace_info->fun_call->args.push_back(fmt::format("value={}", value));
}

static void SetIntField(inst_trace_info_t *trace_info) {
    auto *env = reinterpret_cast<JNIEnv *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 0));
    auto obj = reinterpret_cast<jobject>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 1));
    auto fid = reinterpret_cast<jfieldID>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 2));
    auto value = (jint) (DispatchBase::get_arg_register_value(&trace_info->pre_status, 3) & 0xffffffff);
    auto object_name = get_jni_jobject_name(env, obj);
    auto field_info = get_jni_field_signature(env, fid);
    //LOGI("%s: obj=%s, fid=%s, value=%d", "SetIntField", object_name.c_str(), field_info.c_str(), value);
    trace_info->fun_call->args.push_back(fmt::format("obj={} {:#x}", object_name, (uintptr_t) obj));
    trace_info->fun_call->args.push_back(fmt::format("fid={} {:#x}", field_info, (uintptr_t) fid));
    trace_info->fun_call->args.push_back(fmt::format("value={}", value));
}

static void SetLongField(inst_trace_info_t *trace_info) {
    auto *env = reinterpret_cast<JNIEnv *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 0));
    auto obj = reinterpret_cast<jobject>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 1));
    auto fid = reinterpret_cast<jfieldID>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 2));
    auto value = (jlong) (DispatchBase::get_arg_register_value(&trace_info->pre_status, 3) & 0xffffffffffffffff);
    auto object_name = get_jni_jobject_name(env, obj);
    auto field_info = get_jni_field_signature(env, fid);
    //LOGI("%s: obj=%s, fid=%s, value=%ld", "SetLongField", object_name.c_str(), field_info.c_str(), value);
    trace_info->fun_call->args.push_back(fmt::format("obj={} {:#x}", object_name, (uintptr_t) obj));
    trace_info->fun_call->args.push_back(fmt::format("fid={} {:#x}", field_info, (uintptr_t) fid));
    trace_info->fun_call->args.push_back(fmt::format("value={}", value));
}

static void SetFloatField(inst_trace_info_t *trace_info) {
    auto *env = reinterpret_cast<JNIEnv *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 0));
    auto obj = reinterpret_cast<jobject>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 1));
    auto fid = reinterpret_cast<jfieldID>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 2));
    jfloat value = (jfloat) (DispatchBase::get_arg_register_value(&trace_info->pre_status, 3));
    auto object_name = get_jni_jobject_name(env, obj);
    auto field_info = get_jni_field_signature(env, fid);
    //LOGI("%s: obj=%s, fid=%s, value=%f", "SetFloatField", object_name.c_str(), field_info.c_str(), value);
    trace_info->fun_call->args.push_back(fmt::format("obj={} {:#x}", object_name, (uintptr_t) obj));
    trace_info->fun_call->args.push_back(fmt::format("fid={} {:#x}", field_info, (uintptr_t) fid));
    trace_info->fun_call->args.push_back(fmt::format("value={}", value));
}

static void SetDoubleField(inst_trace_info_t *trace_info) {
    auto *env = reinterpret_cast<JNIEnv *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 0));
    auto obj = reinterpret_cast<jobject>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 1));
    auto fid = reinterpret_cast<jfieldID>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 2));
    jdouble value = (jdouble) (DispatchBase::get_arg_register_value(&trace_info->pre_status, 3));
    auto object_name = get_jni_jobject_name(env, obj);
    auto field_info = get_jni_field_signature(env, fid);
    //LOGI("%s: obj=%s, fid=%s, value=%f", "SetDoubleField", object_name.c_str(), field_info.c_str(), value);
    trace_info->fun_call->args.push_back(fmt::format("obj={} {:#x}", object_name, (uintptr_t) obj));
    trace_info->fun_call->args.push_back(fmt::format("fid={} {:#x}", field_info, (uintptr_t) fid));
    trace_info->fun_call->args.push_back(fmt::format("value={}", value));
}

// static jmethodID GetStaticMethodID(JNIEnv *env, jclass java_class, const char *name,const char *sig)
static void GetStaticMethodID(inst_trace_info_t *trace_info) {
    auto *env = reinterpret_cast<JNIEnv *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 0));
    auto java_class = reinterpret_cast<jclass>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 1));
    auto name = reinterpret_cast<const char *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 2));
    auto sig = reinterpret_cast<const char *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 3));
    auto class_name = get_jni_class_or_java_class_name(env, java_class);
    //LOGI("%s: java_class=%s, name=%s, sig=%s", "GetStaticMethodID", class_name.c_str(), name, sig);
    trace_info->fun_call->args.push_back(fmt::format("java_class={} {:#x}", class_name, (uintptr_t) java_class));
    trace_info->fun_call->args.push_back(fmt::format("name={}", name));
    trace_info->fun_call->args.push_back(fmt::format("sig={}", sig));
}

//JNIEnv *env, jclass clazz, jmethodID mid,
#define PROXY_CALL_STATIC_TYPE_METHOD(_jname)                                                       \
    static void CallStatic##_jname##Method(inst_trace_info_t* trace_info)                                                                                                \
    {                                                                                                       \
    auto* env = reinterpret_cast<JNIEnv*>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 0));\
    auto clazz = reinterpret_cast<jclass>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 1));\
    auto mid = reinterpret_cast<jmethodID>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 2));\
    auto class_name = get_jni_class_or_java_class_name(env, clazz);\
    auto method_info = get_jni_method_signature(env, mid);\
    trace_info->fun_call->args.push_back(fmt::format("clazz={} {:#x}", class_name, (uintptr_t)clazz));\
    trace_info->fun_call->args.push_back(fmt::format("mid={} {:#x}", method_info, (uintptr_t)mid));\
    }

#define PROXY_CALL_STATIC_TYPE_METHODV(_jname)                                                      \
    static void CallStatic##_jname##MethodV(inst_trace_info_t* trace_info)                                                                                                \
    {                                                                                                       \
    auto* env = reinterpret_cast<JNIEnv*>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 0));\
    auto clazz = reinterpret_cast<jclass>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 1));\
    auto mid = reinterpret_cast<jmethodID>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 2));\
    auto class_name = get_jni_class_or_java_class_name(env, clazz);\
    auto method_info = get_jni_method_signature(env, mid);\
    trace_info->fun_call->args.push_back(fmt::format("clazz={} {:#x}", class_name, (uintptr_t)clazz));\
    trace_info->fun_call->args.push_back(fmt::format("mid={} {:#x}", method_info, (uintptr_t)mid));\
    }

#define PROXY_CALL_STATIC_TYPE_METHODA(_jname)                                                      \
    static void CallStatic##_jname##MethodA(inst_trace_info_t* trace_info)                                                                                                \
    {                                                                                                       \
    auto* env = reinterpret_cast<JNIEnv*>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 0));\
    auto clazz = reinterpret_cast<jclass>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 1));\
    auto mid = reinterpret_cast<jmethodID>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 2));\
    auto class_name = get_jni_class_or_java_class_name(env, clazz);\
    auto method_info = get_jni_method_signature(env, mid);\
    trace_info->fun_call->args.push_back(fmt::format("clazz={} {:#x}", class_name, (uintptr_t)clazz));\
    trace_info->fun_call->args.push_back(fmt::format("mid={} {:#x}", method_info, (uintptr_t)mid));\
    }
#define PROXY_CALL_STATIC_TYPE(_jname)                                    \
    PROXY_CALL_STATIC_TYPE_METHOD(_jname)                                 \
    PROXY_CALL_STATIC_TYPE_METHODV( _jname)                                \
    PROXY_CALL_STATIC_TYPE_METHODA(_jname)

PROXY_CALL_STATIC_TYPE(Object)

PROXY_CALL_STATIC_TYPE(Boolean)

PROXY_CALL_STATIC_TYPE(Byte)

PROXY_CALL_STATIC_TYPE(Char)

PROXY_CALL_STATIC_TYPE(Short)

PROXY_CALL_STATIC_TYPE(Int)

PROXY_CALL_STATIC_TYPE(Long)

PROXY_CALL_STATIC_TYPE(Float)

PROXY_CALL_STATIC_TYPE(Double)

PROXY_CALL_STATIC_TYPE(Void)

//GetStaticFieldID
static void GetStaticFieldID(inst_trace_info_t *trace_info) {
    auto *env = reinterpret_cast<JNIEnv *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 0));
    auto java_class = reinterpret_cast<jclass>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 1));
    auto name = reinterpret_cast<const char *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 2));
    auto sig = reinterpret_cast<const char *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 3));
    auto class_name = get_jni_class_or_java_class_name(env, java_class);
    trace_info->fun_call->args.push_back(fmt::format("java_class={} {:#x}", class_name, (uintptr_t) java_class));
    trace_info->fun_call->args.push_back(fmt::format("name={}", name));
    trace_info->fun_call->args.push_back(fmt::format("sig={}", sig));
}

#define PROXY_GET_STATIC_TYPE_Field(_jname)                                             \
    static void GetStatic##_jname##Field (inst_trace_info_t* trace_info)        \
    {                                                                             \
        auto* env = reinterpret_cast<JNIEnv*>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 0));\
        auto clazz = reinterpret_cast<jclass>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 1));\
        auto fid = reinterpret_cast<jfieldID>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 2));\
        auto class_name = get_jni_class_or_java_class_name(env, clazz);\
        auto field_info = get_jni_field_signature(env, fid);\
        trace_info->fun_call->args.push_back(fmt::format("clazz={} {:#x}", class_name, (uintptr_t)clazz));\
        trace_info->fun_call->args.push_back(fmt::format("fid={} {:#x}", field_info, (uintptr_t)fid));\
    }

PROXY_GET_STATIC_TYPE_Field(Object)

PROXY_GET_STATIC_TYPE_Field(Boolean)

PROXY_GET_STATIC_TYPE_Field(Byte)

PROXY_GET_STATIC_TYPE_Field(Char)

PROXY_GET_STATIC_TYPE_Field(Short)

PROXY_GET_STATIC_TYPE_Field(Int)

PROXY_GET_STATIC_TYPE_Field(Long)

PROXY_GET_STATIC_TYPE_Field(Float)

PROXY_GET_STATIC_TYPE_Field(Double)

//static void SetStaticObjectField(JNIEnv *env, jclass clazz, jfieldID fieldID,jobject value)
static void SetStaticObjectField(inst_trace_info_t *trace_info) {
    auto *env = reinterpret_cast<JNIEnv *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 0));
    auto clazz = reinterpret_cast<jclass>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 1));
    auto fid = reinterpret_cast<jfieldID>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 2));
    auto value = reinterpret_cast<jobject>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 3));
    auto class_name = get_jni_class_or_java_class_name(env, clazz);
    auto field_info = get_jni_field_signature(env, fid);
    auto value_info = get_jni_object_to_string(env, value);
    trace_info->fun_call->args.push_back(fmt::format("clazz={} {:#x}", class_name, (uintptr_t) clazz));
    trace_info->fun_call->args.push_back(fmt::format("fid={} {:#x}", field_info, (uintptr_t) fid));
    trace_info->fun_call->args.push_back(fmt::format("value={} {:#x}", value_info, (uintptr_t) value));
}

static void SetStaticBooleanField(inst_trace_info_t *trace_info) {
    auto *env = reinterpret_cast<JNIEnv *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 0));
    auto clazz = reinterpret_cast<jclass>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 1));
    auto fid = reinterpret_cast<jfieldID>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 2));
    auto value = (jboolean) (DispatchBase::get_arg_register_value(&trace_info->pre_status, 3));
    auto class_name = get_jni_class_or_java_class_name(env, clazz);
    auto field_info = get_jni_field_signature(env, fid);
    trace_info->fun_call->args.push_back(fmt::format("clazz={} {:#x}", class_name, (uintptr_t) clazz));
    trace_info->fun_call->args.push_back(fmt::format("fid={} {:#x}", field_info, (uintptr_t) fid));
    trace_info->fun_call->args.push_back(fmt::format(
            "value={}", value ? "true" : "false"));
}

static void SetStaticByteField(inst_trace_info_t *trace_info) {
    auto *env = reinterpret_cast<JNIEnv *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 0));
    auto clazz = reinterpret_cast<jclass>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 1));
    auto fid = reinterpret_cast<jfieldID>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 2));
    auto value = (jbyte) (DispatchBase::get_arg_register_value(&trace_info->pre_status, 3));
    auto class_name = get_jni_class_or_java_class_name(env, clazz);
    auto field_info = get_jni_field_signature(env, fid);
    trace_info->fun_call->args.push_back(fmt::format("clazz={} {:#x}", class_name, (uintptr_t) clazz));
    trace_info->fun_call->args.push_back(fmt::format("fid={} {:#x}", field_info, (uintptr_t) fid));
    trace_info->fun_call->args.push_back(fmt::format("value={}", value));
}

static void SetStaticCharField(inst_trace_info_t *trace_info) {
    auto *env = reinterpret_cast<JNIEnv *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 0));
    auto clazz = reinterpret_cast<jclass>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 1));
    auto fid = reinterpret_cast<jfieldID>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 2));
    auto value = (jchar) (DispatchBase::get_arg_register_value(&trace_info->pre_status, 3));
    auto class_name = get_jni_class_or_java_class_name(env, clazz);
    auto field_info = get_jni_field_signature(env, fid);
    trace_info->fun_call->args.push_back(fmt::format("clazz={} {:#x}", class_name, (uintptr_t) clazz));
    trace_info->fun_call->args.push_back(fmt::format("fid={} {:#x}", field_info, (uintptr_t) fid));
    trace_info->fun_call->args.push_back(fmt::format("value={}", value));
}

static void SetStaticShortField(inst_trace_info_t *trace_info) {
    auto *env = reinterpret_cast<JNIEnv *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 0));
    auto clazz = reinterpret_cast<jclass>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 1));
    auto fid = reinterpret_cast<jfieldID>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 2));
    auto value = (jshort) (DispatchBase::get_arg_register_value(&trace_info->pre_status, 3));
    auto class_name = get_jni_class_or_java_class_name(env, clazz);
    auto field_info = get_jni_field_signature(env, fid);
    trace_info->fun_call->args.push_back(fmt::format("clazz={} {:#x}", class_name, (uintptr_t) clazz));
    trace_info->fun_call->args.push_back(fmt::format("fid={} {:#x}", field_info, (uintptr_t) fid));
    trace_info->fun_call->args.push_back(fmt::format("value={}", value));
}

static void SetStaticIntField(inst_trace_info_t *trace_info) {
    auto *env = reinterpret_cast<JNIEnv *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 0));
    auto clazz = reinterpret_cast<jclass>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 1));
    auto fid = reinterpret_cast<jfieldID>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 2));
    auto value = (jint) (DispatchBase::get_arg_register_value(&trace_info->pre_status, 3));
    auto class_name = get_jni_class_or_java_class_name(env, clazz);
    auto field_info = get_jni_field_signature(env, fid);
    //LOGI("%s: clazz=%s, fid=%s, value=%d", "SetStaticIntField", class_name.c_str(), field_info.c_str(), value);
    trace_info->fun_call->args.push_back(fmt::format("clazz={} {:#x}", class_name, (uintptr_t) clazz));
    trace_info->fun_call->args.push_back(fmt::format("fid={} {:#x}", field_info, (uintptr_t) fid));
    trace_info->fun_call->args.push_back(fmt::format("value={}", value));
}

static void SetStaticLongField(inst_trace_info_t *trace_info) {
    auto *env = reinterpret_cast<JNIEnv *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 0));
    auto clazz = reinterpret_cast<jclass>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 1));
    auto fid = reinterpret_cast<jfieldID>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 2));
    auto value = (jlong) (DispatchBase::get_arg_register_value(&trace_info->pre_status, 3));
    auto class_name = get_jni_class_or_java_class_name(env, clazz);
    auto field_info = get_jni_field_signature(env, fid);
    //LOGI("%s: clazz=%s, fid=%s, value=%ld", "SetStaticLongField", class_name.c_str(), field_info.c_str(), value);
    trace_info->fun_call->args.push_back(fmt::format("clazz={} {:#x}", class_name, (uintptr_t) clazz));
    trace_info->fun_call->args.push_back(fmt::format("fid={} {:#x}", field_info, (uintptr_t) fid));
    trace_info->fun_call->args.push_back(fmt::format("value={}", value));
}

static void SetStaticFloatField(inst_trace_info_t *trace_info) {
    auto *env = reinterpret_cast<JNIEnv *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 0));
    auto clazz = reinterpret_cast<jclass>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 1));
    auto fid = reinterpret_cast<jfieldID>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 2));
    auto value = (jfloat) (DispatchBase::get_arg_register_value(&trace_info->pre_status, 3));
    auto class_name = get_jni_class_or_java_class_name(env, clazz);
    auto field_info = get_jni_field_signature(env, fid);
    //LOGI("%s: clazz=%s, fid=%s, value=%f", "SetStaticFloatField", class_name.c_str(), field_info.c_str(), value);
    trace_info->fun_call->args.push_back(fmt::format("clazz={} {:#x}", class_name, (uintptr_t) clazz));
    trace_info->fun_call->args.push_back(fmt::format("fid={} {:#x}", field_info, (uintptr_t) fid));
    trace_info->fun_call->args.push_back(fmt::format("value={}", value));
}


static void SetStaticDoubleField(inst_trace_info_t *trace_info) {
    auto *env = reinterpret_cast<JNIEnv *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 0));
    auto clazz = reinterpret_cast<jclass>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 1));
    auto fid = reinterpret_cast<jfieldID>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 2));
    auto value = (jdouble) (DispatchBase::get_arg_register_value(&trace_info->pre_status, 3));
    auto class_name = get_jni_class_or_java_class_name(env, clazz);
    auto field_info = get_jni_field_signature(env, fid);
    //LOGI("%s: clazz=%s, fid=%s, value=%f", "SetStaticDoubleField", class_name.c_str(), field_info.c_str(), value);
    trace_info->fun_call->args.push_back(fmt::format("clazz={} {:#x}", class_name, (uintptr_t) clazz));
    trace_info->fun_call->args.push_back(fmt::format("fid={} {:#x}", field_info, (uintptr_t) fid));
    trace_info->fun_call->args.push_back(fmt::format("value={}", value));
}

//NewString
static void NewString(inst_trace_info_t *trace_info) {
    auto *env = reinterpret_cast<JNIEnv *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 0));
    auto str = reinterpret_cast<jchar *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 1));
    auto str_len = (jsize) (DispatchBase::get_arg_register_value(&trace_info->pre_status, 2));
    std::wstring wstr = std::wstring(str, str + str_len);
    //LOGI("%s: str=%ls, str_len=%d", "NewString", wstr.c_str(), str_len);
    trace_info->fun_call->args.push_back(fmt::format("str={:#x}", static_cast<void *>(str)));
    trace_info->fun_call->args.push_back(fmt::format("str_len={}", str_len));
}

//GetStringLength
static void GetStringLength(inst_trace_info_t *trace_info) {
    auto *env = reinterpret_cast<JNIEnv *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 0));
    auto str = reinterpret_cast<jstring>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 1));
    auto len = env->GetStringLength(str);
    //LOGI("%s: str:%p len=%d", "GetStringLength", str, len);
    trace_info->fun_call->args.push_back(fmt::format("str={:#x}", (uintptr_t) str));
    trace_info->fun_call->args.push_back(fmt::format("len={}", len));
}

//GetStringChars
static void GetStringChars(inst_trace_info_t *trace_info) {
    auto *env = reinterpret_cast<JNIEnv *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 0));
    auto str = reinterpret_cast<jstring>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 1));
    auto copy = (jboolean) (DispatchBase::get_arg_register_value(&trace_info->pre_status, 2));
    auto chars = env->GetStringChars(str, nullptr);
    std::wstring wstr = std::wstring(chars, chars + env->GetStringLength(str));
    //LOGI("%s: str=%ls, copy=%d", "GetStringChars", wstr.c_str(), copy);
    trace_info->fun_call->args.push_back(fmt::format("str={:#x}", (uintptr_t) str));
    trace_info->fun_call->args.push_back(fmt::format("copy={}", copy));
    env->ReleaseStringChars(str, chars);
}

//ReleaseStringChars
static void ReleaseStringChars(inst_trace_info_t *trace_info) {
    auto *env = reinterpret_cast<JNIEnv *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 0));
    auto str = reinterpret_cast<jstring>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 1));
    auto chars = reinterpret_cast<const jchar *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 2));
    //LOGI("%s: str:%p chars=%p", "ReleaseStringChars", str, chars);
    trace_info->fun_call->args.push_back(fmt::format("str={:#x}", (uintptr_t) str));
    trace_info->fun_call->args.push_back(fmt::format("chars={}", (uintptr_t) chars));
}

//static jstring NewStringUTF(JNIEnv *env, const char *bytes)
static void NewStringUTF(inst_trace_info_t *trace_info) {
    auto *env = reinterpret_cast<JNIEnv *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 0));
    auto str = reinterpret_cast<const char *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 1));
    //LOGI("%s: str=%s", "NewStringUTF", str);
    trace_info->fun_call->args.push_back(fmt::format("str={}", str));
}

// static jsize GetStringUTFLength(JNIEnv *env, jstring string)
static void GetStringUTFLength(inst_trace_info_t *trace_info) {
    auto *env = reinterpret_cast<JNIEnv *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 0));
    auto str = reinterpret_cast<jstring>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 1));
    auto len = env->GetStringUTFLength(str);
    //LOGI("%s: str:%p len=%d", "GetStringUTFLength", str, len);
    trace_info->fun_call->args.push_back(fmt::format("str={:#x}", (uintptr_t) str));
    trace_info->fun_call->args.push_back(fmt::format("len={}", len));
}

//static const char *GetStringUTFChars(JNIEnv *env, jstring string, jboolean *isCopy)
static void GetStringUTFChars(inst_trace_info_t *trace_info) {
    auto *env = reinterpret_cast<JNIEnv *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 0));
    auto str = reinterpret_cast<jstring>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 1));
    auto copy = reinterpret_cast<jboolean *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 2));
    auto chars = env->GetStringUTFChars(str, copy);
    //LOGI("%s: str:%p chars=%p", "GetStringUTFChars", str, chars);
    trace_info->fun_call->args.push_back(fmt::format("str={:#x}", (uintptr_t) str));
    trace_info->fun_call->args.push_back(fmt::format("chars={}", chars));
    env->ReleaseStringUTFChars(str, chars);
}

//ReleaseStringUTFChars
static void ReleaseStringUTFChars(inst_trace_info_t *trace_info) {
    auto *env = reinterpret_cast<JNIEnv *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 0));
    auto str = reinterpret_cast<jstring>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 1));
    auto chars = reinterpret_cast<const char *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 2));
    //LOGI("%s: str:%p chars=%p", "ReleaseStringUTFChars", str, chars);
    trace_info->fun_call->args.push_back(fmt::format("str={:#x}", (uintptr_t) str));
    trace_info->fun_call->args.push_back(fmt::format("chars={}", chars));
}

//static jsize GetArrayLength(JNIEnv *env, jarray array) {
static void GetArrayLength(inst_trace_info_t *trace_info) {
    auto *env = reinterpret_cast<JNIEnv *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 0));
    auto array = reinterpret_cast<jarray>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 1));
    auto len = env->GetArrayLength(array);
    //LOGI("%s: array:%p len=%d", "GetArrayLength", array, len);
    trace_info->fun_call->args.push_back(fmt::format("array={:#x}", (uintptr_t) array));
    trace_info->fun_call->args.push_back(fmt::format("len={}", len));
}

//static jobjectArray NewObjectArray(JNIEnv *env, jsize length, jclass elementClass, jobject initialElement)
static void NewObjectArray(inst_trace_info_t *trace_info) {
    auto *env = reinterpret_cast<JNIEnv *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 0));
    auto len = (jsize) DispatchBase::get_arg_register_value(&trace_info->pre_status, 1);
    auto elementClass = reinterpret_cast<jclass>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 2));
    auto initialElement = reinterpret_cast<jobject>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 3));
    auto element_info = get_jni_class_or_java_class_name(env, elementClass);
    //LOGI("%s: len=%d elementClass=%s initialElement=%p", "NewObjectArray", len, element_info.c_str(), initialElement);
    trace_info->fun_call->args.push_back(fmt::format("len={}", len));
    trace_info->fun_call->args.push_back(fmt::format("elementClass={}", element_info));
    trace_info->fun_call->args.push_back(fmt::format("initialElement={:#x}", (uintptr_t) initialElement));
}

//static jobject GetObjectArrayElement(JNIEnv *env, jobjectArray array, jsize index)
static void GetObjectArrayElement(inst_trace_info_t *trace_info) {
    auto *env = reinterpret_cast<JNIEnv *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 0));
    auto array = reinterpret_cast<jobjectArray>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 1));
    auto index = (jsize) DispatchBase::get_arg_register_value(&trace_info->pre_status, 2);
    //LOGI("%s: array:%p index=%d ", "GetObjectArrayElement", array, index);
    trace_info->fun_call->args.push_back(fmt::format("array={:#x}", (uintptr_t) array));
    trace_info->fun_call->args.push_back(fmt::format("index={}", index));
}

// static void SetObjectArrayElement(JNIEnv *env, jobjectArray array, jsize index, jobject val)
static void SetObjectArrayElement(inst_trace_info_t *trace_info) {
    auto *env = reinterpret_cast<JNIEnv *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 0));
    auto array = reinterpret_cast<jobjectArray>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 1));
    auto index = (jsize) DispatchBase::get_arg_register_value(&trace_info->pre_status, 2);
    auto val = reinterpret_cast<jobject>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 3));
    auto val_info = get_jni_object_to_string(env, val);
    //LOGI("%s: array:%p index=%d val=%p", "SetObjectArrayElement", array, index, val_info.c_str());
    trace_info->fun_call->args.push_back(fmt::format("array={:#x}", (uintptr_t) array));
    trace_info->fun_call->args.push_back(fmt::format("index={}", index));
    trace_info->fun_call->args.push_back(fmt::format("val={}", val_info));
}


#define PROXY_New_Array(_jname) \
    static void New##_jname##Array(inst_trace_info_t* trace_info) \
    {\
        auto* env = reinterpret_cast<JNIEnv*>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 0));\
        auto len = (jsize)DispatchBase::get_arg_register_value(&trace_info->pre_status, 1);\
        trace_info->fun_call->args.push_back(fmt::format("len={}", len));\
    }


PROXY_New_Array(Boolean)

PROXY_New_Array(Byte)

PROXY_New_Array(Char)

PROXY_New_Array(Short)

PROXY_New_Array(Int)

PROXY_New_Array(Long)

PROXY_New_Array(Float)

PROXY_New_Array(Double)


#define PROXY_GET_Array_Elements(_jtype, _jname) \
    static void Get##_jname##ArrayElements(inst_trace_info_t* trace_info) \
    {\
        auto* env = reinterpret_cast<JNIEnv*>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 0));\
        auto array = reinterpret_cast<_jtype>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 1));\
        auto isCopy = reinterpret_cast<jboolean*>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 2));\
        trace_info->fun_call->args.push_back(fmt::format("array={:#x}", (uintptr_t)array));\
        trace_info->fun_call->args.push_back(fmt::format("isCopy={}", (bool)isCopy));\
    }

PROXY_GET_Array_Elements(jbooleanArray, Boolean)

PROXY_GET_Array_Elements(jbyteArray, Byte)

PROXY_GET_Array_Elements(jcharArray, Char)

PROXY_GET_Array_Elements(jshortArray, Short)

PROXY_GET_Array_Elements(jintArray, Int)

PROXY_GET_Array_Elements(jlongArray, Long)

PROXY_GET_Array_Elements(jfloatArray, Float)

PROXY_GET_Array_Elements(jdoubleArray, Double)

//static void ReleaseBooleanArrayElements(JNIEnv *env, jbooleanArray array, jboolean *elems, jint mode)
static void ReleaseBooleanArrayElements(inst_trace_info_t *trace_info) {
    auto *env = reinterpret_cast<JNIEnv *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 0));
    auto array = reinterpret_cast<jbooleanArray>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 1));
    auto elems = reinterpret_cast<jboolean *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 2));
    auto mode = (jint) DispatchBase::get_arg_register_value(&trace_info->pre_status, 3);
    trace_info->fun_call->args.push_back(fmt::format("array={:#x}", (uintptr_t) array));
    trace_info->fun_call->args.push_back(fmt::format("elems={:#x}", (uintptr_t) elems));
    trace_info->fun_call->args.push_back(fmt::format("mode={}", mode));
}

static void ReleaseByteArrayElements(inst_trace_info_t *trace_info) {
    auto *env = reinterpret_cast<JNIEnv *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 0));
    auto array = reinterpret_cast<jbyteArray>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 1));
    auto elems = reinterpret_cast<jbyte *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 2));
    auto mode = (jint) DispatchBase::get_arg_register_value(&trace_info->pre_status, 3);
    //LOGI("%s: array:%p elems=%p mode=%d", "ReleaseByteArrayElements", array, elems, mode);
    trace_info->fun_call->args.push_back(fmt::format("array={:#x}", (uintptr_t) array));
    trace_info->fun_call->args.push_back(fmt::format("elems={:#x}", (uintptr_t) elems));
    trace_info->fun_call->args.push_back(fmt::format("mode={}", mode));
}

static void ReleaseCharArrayElements(inst_trace_info_t *trace_info) {
    auto *env = reinterpret_cast<JNIEnv *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 0));
    auto array = reinterpret_cast<jcharArray>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 1));
    auto elems = reinterpret_cast<jchar *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 2));
    auto mode = (jint) DispatchBase::get_arg_register_value(&trace_info->pre_status, 3);
    //LOGI("%s: array:%p elems=%p mode=%d", "ReleaseCharArrayElements", array, elems, mode);
    trace_info->fun_call->args.push_back(fmt::format("array={:#x}", (uintptr_t) array));
    trace_info->fun_call->args.push_back(fmt::format("elems={:#x}", (uintptr_t) elems));
    trace_info->fun_call->args.push_back(fmt::format("mode={}", mode));
}

static void ReleaseShortArrayElements(inst_trace_info_t *trace_info) {
    auto *env = reinterpret_cast<JNIEnv *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 0));
    auto array = reinterpret_cast<jshortArray>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 1));
    auto elems = reinterpret_cast<jshort *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 2));
    auto mode = (jint) DispatchBase::get_arg_register_value(&trace_info->pre_status, 3);
    //LOGI("%s: array:%p elems=%p mode=%d", "ReleaseShortArrayElements", array, elems, mode);
    trace_info->fun_call->args.push_back(fmt::format("array={:#x}", (uintptr_t) array));
}

static void ReleaseIntArrayElements(inst_trace_info_t *trace_info) {
    auto *env = reinterpret_cast<JNIEnv *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 0));
    auto array = reinterpret_cast<jintArray>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 1));
    auto elems = reinterpret_cast<jint *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 2));
    auto mode = (jint) DispatchBase::get_arg_register_value(&trace_info->pre_status, 3);
    //LOGI("%s: array:%p elems=%p mode=%d", "ReleaseIntArrayElements", array, elems, mode);
    trace_info->fun_call->args.push_back(fmt::format("array={:#x}", (uintptr_t) array));
    trace_info->fun_call->args.push_back(fmt::format("elems={:#x}", (uintptr_t) elems));
    trace_info->fun_call->args.push_back(fmt::format("mode={}", mode));
}

static void ReleaseLongArrayElements(inst_trace_info_t *trace_info) {
    auto *env = reinterpret_cast<JNIEnv *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 0));
    auto array = reinterpret_cast<jlongArray>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 1));
    auto elems = reinterpret_cast<jlong *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 2));
    auto mode = (jint) DispatchBase::get_arg_register_value(&trace_info->pre_status, 3);
    //LOGI("%s: array:%p elems=%p mode=%d", "ReleaseLongArrayElements", array, elems, mode);
    trace_info->fun_call->args.push_back(fmt::format("array={:#x}", (uintptr_t) array));
    trace_info->fun_call->args.push_back(fmt::format("elems={:#x}", (uintptr_t) elems));
    trace_info->fun_call->args.push_back(fmt::format("mode={}", mode));
}

static void ReleaseFloatArrayElements(inst_trace_info_t *trace_info) {
    auto *env = reinterpret_cast<JNIEnv *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 0));
    auto array = reinterpret_cast<jfloatArray>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 1));
    auto elems = reinterpret_cast<jfloat *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 2));
    auto mode = (jint) DispatchBase::get_arg_register_value(&trace_info->pre_status, 3);
    //LOGI("%s: array:%p elems=%p mode=%d", "ReleaseFloatArrayElements", array, elems, mode);
    trace_info->fun_call->args.push_back(fmt::format("array={:#x}", (uintptr_t) array));
    trace_info->fun_call->args.push_back(fmt::format("elems={:#x}", (uintptr_t) elems));
    trace_info->fun_call->args.push_back(fmt::format("mode={}", mode));
}

static void ReleaseDoubleArrayElements(inst_trace_info_t *trace_info) {
    auto *env = reinterpret_cast<JNIEnv *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 0));
    auto array = reinterpret_cast<jdoubleArray>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 1));
    auto elems = reinterpret_cast<jdouble *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 2));
    auto mode = (jint) DispatchBase::get_arg_register_value(&trace_info->pre_status, 3);
    //LOGI("%s: array:%p elems=%p mode=%d", "ReleaseDoubleArrayElements", array, elems, mode);
    trace_info->fun_call->args.push_back(fmt::format("array={:#x}", (uintptr_t) array));
    trace_info->fun_call->args.push_back(fmt::format("elems={:#x}", (uintptr_t) elems));
    trace_info->fun_call->args.push_back(fmt::format("mode={}", mode));
}

//void GetBooleanArrayRegion(jbooleanArray array, jsize start, jsize len,
//        jboolean* buf)
static void GetByteArrayRegion(inst_trace_info_t *trace_info) {
    auto env = reinterpret_cast<JNIEnv *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 0));
    auto array = reinterpret_cast<jbyteArray>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 1));
    auto start = (jsize) DispatchBase::get_arg_register_value(&trace_info->pre_status, 2);
    auto len = (jsize) DispatchBase::get_arg_register_value(&trace_info->pre_status, 3);
    auto buf = reinterpret_cast<jbyte *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 4));
    //LOGI("%s: array:%p start=%d len=%d buf=%p", "GetByteArrayRegion", array, start, len, buf);
    trace_info->fun_call->args.push_back(fmt::format("array={:#x}", (uintptr_t) array));
    trace_info->fun_call->args.push_back(fmt::format("start={}", start));
    trace_info->fun_call->args.push_back(fmt::format("len={}", len));
    trace_info->fun_call->args.push_back(fmt::format("buf={:#x}", (uintptr_t) buf));
}


static void GetBooleanArrayRegion(inst_trace_info_t *trace_info) {
    auto env = reinterpret_cast<JNIEnv *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 0));
    auto array = reinterpret_cast<jbooleanArray>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 1));
    auto start = (jsize) DispatchBase::get_arg_register_value(&trace_info->pre_status, 2);
    auto len = (jsize) DispatchBase::get_arg_register_value(&trace_info->pre_status, 3);
    auto buf = reinterpret_cast<jboolean *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 4));
    //LOGI("%s: array:%p start=%d len=%d buf=%p", "GetBooleanArrayRegion", array, start, len, buf);
    trace_info->fun_call->args.push_back(fmt::format("array={:#x}", (uintptr_t) array));
    trace_info->fun_call->args.push_back(fmt::format("start={}", start));
    trace_info->fun_call->args.push_back(fmt::format("len={}", len));
    trace_info->fun_call->args.push_back(fmt::format("buf={:#x}", (uintptr_t) buf));
}


static void GetCharArrayRegion(inst_trace_info_t *trace_info) {
    auto env = reinterpret_cast<JNIEnv *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 0));
    auto array = reinterpret_cast<jcharArray>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 1));
    auto start = (jsize) DispatchBase::get_arg_register_value(&trace_info->pre_status, 2);
    auto len = (jsize) DispatchBase::get_arg_register_value(&trace_info->pre_status, 3);
    auto buf = reinterpret_cast<jchar *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 4));
    //LOGI("%s: array:%p start=%d len=%d buf=%p", "GetCharArrayRegion", array, start, len, buf);
    trace_info->fun_call->args.push_back(fmt::format("array={:#x}", (uintptr_t) array));
    trace_info->fun_call->args.push_back(fmt::format("start={}", start));
    trace_info->fun_call->args.push_back(fmt::format("len={}", len));
    trace_info->fun_call->args.push_back(fmt::format("buf={:#x}", (uintptr_t) buf));
}

static void GetShortArrayRegion(inst_trace_info_t *trace_info) {
    auto env = reinterpret_cast<JNIEnv *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 0));
    auto array = reinterpret_cast<jshortArray>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 1));
    auto start = (jsize) DispatchBase::get_arg_register_value(&trace_info->pre_status, 2);
    auto len = (jsize) DispatchBase::get_arg_register_value(&trace_info->pre_status, 3);
    auto buf = reinterpret_cast<jshort *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 4));
    //LOGI("%s: array:%p start=%d len=%d buf=%p", "GetShortArrayRegion", array, start, len, buf);
    trace_info->fun_call->args.push_back(fmt::format("array={:#x}", (uintptr_t) array));
    trace_info->fun_call->args.push_back(fmt::format("start={}", start));
    trace_info->fun_call->args.push_back(fmt::format("len={}", len));
    trace_info->fun_call->args.push_back(fmt::format("buf={:#x}", (uintptr_t) buf));
}

static void GetIntArrayRegion(inst_trace_info_t *trace_info) {
    auto env = reinterpret_cast<JNIEnv *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 0));
    auto array = reinterpret_cast<jintArray>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 1));
    auto start = (jsize) DispatchBase::get_arg_register_value(&trace_info->pre_status, 2);
    auto len = (jsize) DispatchBase::get_arg_register_value(&trace_info->pre_status, 3);
    auto buf = reinterpret_cast<jint *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 4));
    //LOGI("%s: array:%p start=%d len=%d buf=%p", "GetIntArrayRegion", array, start, len, buf);
    trace_info->fun_call->args.push_back(fmt::format("array={:#x}", (uintptr_t) array));
    trace_info->fun_call->args.push_back(fmt::format("start={}", start));
    trace_info->fun_call->args.push_back(fmt::format("len={}", len));
    trace_info->fun_call->args.push_back(fmt::format("buf={:#x}", (uintptr_t) buf));
}

static void GetLongArrayRegion(inst_trace_info_t *trace_info) {
    auto env = reinterpret_cast<JNIEnv *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 0));
    auto array = reinterpret_cast<jlongArray>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 1));
    auto start = (jsize) DispatchBase::get_arg_register_value(&trace_info->pre_status, 2);
    auto len = (jsize) DispatchBase::get_arg_register_value(&trace_info->pre_status, 3);
    auto buf = reinterpret_cast<jlong *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 4));
    //LOGI("%s: array:%p start=%d len=%d buf=%p", "GetLongArrayRegion", array, start, len, buf);
    trace_info->fun_call->args.push_back(fmt::format("array={:#x}", (uintptr_t) array));
    trace_info->fun_call->args.push_back(fmt::format("start={}", start));
    trace_info->fun_call->args.push_back(fmt::format("len={}", len));
    trace_info->fun_call->args.push_back(fmt::format("buf={:#x}", (uintptr_t) buf));
}

static void GetFloatArrayRegion(inst_trace_info_t *trace_info) {
    auto env = reinterpret_cast<JNIEnv *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 0));
    auto array = reinterpret_cast<jfloatArray>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 1));
    auto start = (jsize) DispatchBase::get_arg_register_value(&trace_info->pre_status, 2);
    auto len = (jsize) DispatchBase::get_arg_register_value(&trace_info->pre_status, 3);
    auto buf = reinterpret_cast<jfloat *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 4));
    trace_info->fun_call->args.push_back(fmt::format("array={:#x}", (uintptr_t) array));
    trace_info->fun_call->args.push_back(fmt::format("start={}", start));
    trace_info->fun_call->args.push_back(fmt::format("len={}", len));
    trace_info->fun_call->args.push_back(fmt::format("buf={:#x}", (uintptr_t) buf));
}

static void GetDoubleArrayRegion(inst_trace_info_t *trace_info) {
    auto env = reinterpret_cast<JNIEnv *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 0));
    auto array = reinterpret_cast<jdoubleArray>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 1));
    auto start = (jsize) DispatchBase::get_arg_register_value(&trace_info->pre_status, 2);
    auto len = (jsize) DispatchBase::get_arg_register_value(&trace_info->pre_status, 3);
    auto buf = reinterpret_cast<jdouble *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 4));
    //LOGI("%s: array:%p start=%d len=%d buf=%p", "GetDoubleArrayRegion", array, start, len, buf);
    trace_info->fun_call->args.push_back(fmt::format("array={:#x}", (uintptr_t) array));
    trace_info->fun_call->args.push_back(fmt::format("start={}", start));
    trace_info->fun_call->args.push_back(fmt::format("len={}", len));
    trace_info->fun_call->args.push_back(fmt::format("buf={:#x}", (uintptr_t) buf));
}

/*
*static void SetBooleanArrayRegion(JNIEnv *env, jbooleanArray array, jsize start, jsize len,
                                      const jboolean *buf)
 *
 */
static void SetBooleanArrayRegion(inst_trace_info_t *trace_info) {
    auto env = reinterpret_cast<JNIEnv *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 0));
    auto array = reinterpret_cast<jbooleanArray>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 1));
    auto start = (jsize) DispatchBase::get_arg_register_value(&trace_info->pre_status, 2);
    auto len = (jsize) DispatchBase::get_arg_register_value(&trace_info->pre_status, 3);
    auto buf = reinterpret_cast<jboolean *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 4));
    //LOGI("%s: array:%p start=%d len=%d buf=%p", "SetBooleanArrayRegion", array, start, len, buf);
    trace_info->fun_call->args.push_back(fmt::format("array={:#x}", (uintptr_t) array));
    trace_info->fun_call->args.push_back(fmt::format("start={}", start));
    trace_info->fun_call->args.push_back(fmt::format("len={}", len));
    trace_info->fun_call->args.push_back(fmt::format("buf={:#x}", (uintptr_t) buf));
}

static void SetByteArrayRegion(inst_trace_info_t *trace_info) {
    auto env = reinterpret_cast<JNIEnv *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 0));
    auto array = reinterpret_cast<jbyteArray>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 1));
    auto start = (jsize) DispatchBase::get_arg_register_value(&trace_info->pre_status, 2);
    auto len = (jsize) DispatchBase::get_arg_register_value(&trace_info->pre_status, 3);
    auto buf = reinterpret_cast<jbyte *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 4));
    //LOGI("%s: array:%p start=%d len=%d buf=%p", "SetByteArrayRegion", array, start, len, buf);
    trace_info->fun_call->args.push_back(fmt::format("array={:#x}", (uintptr_t) array));
    trace_info->fun_call->args.push_back(fmt::format("start={}", start));
    trace_info->fun_call->args.push_back(fmt::format("len={}", len));
    trace_info->fun_call->args.push_back(fmt::format("buf={:#x}", (uintptr_t) buf));
}

static void SetCharArrayRegion(inst_trace_info_t *trace_info) {
    auto env = reinterpret_cast<JNIEnv *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 0));
    auto array = reinterpret_cast<jcharArray>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 1));
    auto start = (jsize) DispatchBase::get_arg_register_value(&trace_info->pre_status, 2);
    auto len = (jsize) DispatchBase::get_arg_register_value(&trace_info->pre_status, 3);
    auto buf = reinterpret_cast<jchar *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 4));
    //LOGI("%s: array:%p start=%d len=%d buf=%p", "SetCharArrayRegion", array, start, len, buf);
    trace_info->fun_call->args.push_back(fmt::format("array={:#x}", (uintptr_t) array));
    trace_info->fun_call->args.push_back(fmt::format("start={}", start));
    trace_info->fun_call->args.push_back(fmt::format("len={}", len));
    trace_info->fun_call->args.push_back(fmt::format("buf={:#x}", (uintptr_t) buf));
}

static void SetShortArrayRegion(inst_trace_info_t *trace_info) {
    auto env = reinterpret_cast<JNIEnv *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 0));
    auto array = reinterpret_cast<jshortArray>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 1));
    auto start = (jsize) DispatchBase::get_arg_register_value(&trace_info->pre_status, 2);
    auto len = (jsize) DispatchBase::get_arg_register_value(&trace_info->pre_status, 3);
    auto buf = reinterpret_cast<jshort *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 4));
    //LOGI("%s: array:%p start=%d len=%d buf=%p", "SetShortArrayRegion", array, start, len, buf);
    trace_info->fun_call->args.push_back(fmt::format("array={:#x}", (uintptr_t) array));
    trace_info->fun_call->args.push_back(fmt::format("start={}", start));
    trace_info->fun_call->args.push_back(fmt::format("len={}", len));
    trace_info->fun_call->args.push_back(fmt::format("buf={:#x}", (uintptr_t) buf));
}

static void SetIntArrayRegion(inst_trace_info_t *trace_info) {
    auto env = reinterpret_cast<JNIEnv *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 0));
    auto array = reinterpret_cast<jintArray>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 1));
    auto start = (jsize) DispatchBase::get_arg_register_value(&trace_info->pre_status, 2);
    auto len = (jsize) DispatchBase::get_arg_register_value(&trace_info->pre_status, 3);
    auto buf = reinterpret_cast<jint *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 4));
    //LOGI("%s: array:%p start=%d len=%d buf=%p", "SetIntArrayRegion", array, start, len, buf);
    trace_info->fun_call->args.push_back(fmt::format("array={:#x}", (uintptr_t) array));
    trace_info->fun_call->args.push_back(fmt::format("start={}", start));
    trace_info->fun_call->args.push_back(fmt::format("len={}", len));
    trace_info->fun_call->args.push_back(fmt::format("buf={:#x}", (uintptr_t) buf));
}

static void SetLongArrayRegion(inst_trace_info_t *trace_info) {
    auto env = reinterpret_cast<JNIEnv *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 0));
    auto array = reinterpret_cast<jlongArray>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 1));
    auto start = (jsize) DispatchBase::get_arg_register_value(&trace_info->pre_status, 2);
    auto len = (jsize) DispatchBase::get_arg_register_value(&trace_info->pre_status, 3);
    auto buf = reinterpret_cast<jlong *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 4));
    //LOGI("%s: array:%p start=%d len=%d buf=%p", "SetLongArrayRegion", array, start, len, buf);
    trace_info->fun_call->args.push_back(fmt::format("array={:#x}", (uintptr_t) array));
    trace_info->fun_call->args.push_back(fmt::format("start={}", start));
    trace_info->fun_call->args.push_back(fmt::format("len={}", len));
    trace_info->fun_call->args.push_back(fmt::format("buf={:#x}", (uintptr_t) buf));
}

static void SetFloatArrayRegion(inst_trace_info_t *trace_info) {
    auto env = reinterpret_cast<JNIEnv *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 0));
    auto array = reinterpret_cast<jfloatArray>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 1));
    auto start = (jsize) DispatchBase::get_arg_register_value(&trace_info->pre_status, 2);
    auto len = (jsize) DispatchBase::get_arg_register_value(&trace_info->pre_status, 3);
    auto buf = reinterpret_cast<jfloat *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 4));
    //LOGI("%s: array:%p start=%d len=%d buf=%p", "SetFloatArrayRegion", array, start, len, buf);
    trace_info->fun_call->args.push_back(fmt::format("array={:#x}", (uintptr_t) array));
    trace_info->fun_call->args.push_back(fmt::format("start={}", start));
    trace_info->fun_call->args.push_back(fmt::format("len={}", len));
    trace_info->fun_call->args.push_back(fmt::format("buf={:#x}", (uintptr_t) buf));
}

static void SetDoubleArrayRegion(inst_trace_info_t *trace_info) {
    auto env = reinterpret_cast<JNIEnv *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 0));
    auto array = reinterpret_cast<jdoubleArray>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 1));
    auto start = (jsize) DispatchBase::get_arg_register_value(&trace_info->pre_status, 2);
    auto len = (jsize) DispatchBase::get_arg_register_value(&trace_info->pre_status, 3);
    auto buf = reinterpret_cast<jdouble *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 4));
    //LOGI("%s: array:%p start=%d len=%d buf=%p", "SetDoubleArrayRegion", array, start, len, buf);
    trace_info->fun_call->args.push_back(fmt::format("array={:#x}", (uintptr_t) array));
    trace_info->fun_call->args.push_back(fmt::format("start={}", start));
    trace_info->fun_call->args.push_back(fmt::format("len={}", len));
    trace_info->fun_call->args.push_back(fmt::format("buf={:#x}", (uintptr_t) buf));
}

//    static jint RegisterNatives(JNIEnv *env, jclass clazz, const JNINativeMethod *methods,
//jint nMethods
static void RegisterNatives(inst_trace_info_t *trace_info) {
    auto env = reinterpret_cast<JNIEnv *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 0));
    auto clazz = reinterpret_cast<jclass>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 1));
    auto methods = reinterpret_cast<const JNINativeMethod *>(DispatchBase::get_arg_register_value(
            &trace_info->pre_status, 2));
    auto nMethods = (jint) DispatchBase::get_arg_register_value(&trace_info->pre_status, 3);
    auto clazz_info = get_jni_class_or_java_class_name(env, clazz);
    for (int i = 0; i < nMethods; ++i) {
        LOGI("RegisterNatives: clazz=%s method=%s signature=%s fnPtr=%p",
             clazz_info.c_str(),
             methods[i].name, methods[i].signature, methods[i].fnPtr);
        auto method = fmt::format("method: {} {}{} {:#x}", clazz_info, methods[i].name, methods[i].signature,
                                  methods[i].fnPtr);
        trace_info->fun_call->args.push_back(method);
    }
}

static void UnregisterNatives(inst_trace_info_t *trace_info) {
    auto env = reinterpret_cast<JNIEnv *>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 0));
    auto clazz = reinterpret_cast<jclass>(DispatchBase::get_arg_register_value(&trace_info->pre_status, 1));
    auto clazz_info = get_jni_class_or_java_class_name(env, clazz);
    LOGI("UnregisterNatives: clazz=%s", clazz_info.c_str());
    trace_info->fun_call->args.push_back(fmt::format("clazz={}", clazz_info));
}


static const std::unordered_map<std::string, std::function<void(inst_trace_info_t *)>> arg_handlers_ = {
        {"FindClass",                    FindClass},
        {"FromReflectedMethod",          FromReflectedMethod},
        {"FromReflectedField",           FromReflectedField},
        {"ToReflectedField",             ToReflectedField},
        {"ToReflectedMethod",            ToReflectedMethod},
        {"GetObjectClass",               GetObjectClass},
        {"GetSuperclass",                GetSuperclass},
        {"IsAssignableFrom",             IsAssignableFrom},
        {"Throw",                        Throw},
        {"ThrowNew",                     ThrowNew},
        {"PushLocalFrame",               PushLocalFrame},
        {"PopLocalFrame",                PopLocalFrame},
        {"NewGlobalRef",                 NewGlobalRef},
        {"DeleteGlobalRef",              DeleteGlobalRef},
        {"DeleteLocalRef",               DeleteLocalRef},
        {"IsSameObject",                 IsSameObject},
        {"NewLocalRef",                  NewLocalRef},
        {"EnsureLocalCapacity",          EnsureLocalCapacity},
        {"AllocObject",                  AllocObject},
        {"NewObject",                    NewObject},
        {"NewObjectV",                   NewObjectV},
        {"NewObjectA",                   NewObjectA},
        {"IsInstanceOf",                 IsInstanceOf},
        {"GetMethodID",                  GetMethodID},
        {"CallObjectMethod",             CallObjectMethod},
        {"CallObjectMethodV",            CallObjectMethodV},
        {"CallObjectMethodA",            CallObjectMethodA},
        {"CallBooleanMethod",            CallBooleanMethod},
        {"CallBooleanMethodV",           CallBooleanMethodV},
        {"CallBooleanMethodA",           CallBooleanMethodA},
        {"CallByteMethod",               CallByteMethod},
        {"CallByteMethodV",              CallByteMethodV},
        {"CallByteMethodA",              CallByteMethodA},
        {"CallCharMethod",               CallCharMethod},
        {"CallCharMethodV",              CallCharMethodV},
        {"CallCharMethodA",              CallCharMethodA},
        {"CallShortMethod",              CallShortMethod},
        {"CallShortMethodV",             CallShortMethodV},
        {"CallShortMethodA",             CallShortMethodA},
        {"CallIntMethod",                CallIntMethod},
        {"CallIntMethodV",               CallIntMethodV},
        {"CallIntMethodA",               CallIntMethodA},
        {"CallLongMethod",               CallLongMethod},
        {"CallLongMethodV",              CallLongMethodV},
        {"CallLongMethodA",              CallLongMethodA},
        {"CallFloatMethod",              CallFloatMethod},
        {"CallFloatMethodV",             CallFloatMethodV},
        {"CallFloatMethodA",             CallFloatMethodA},
        {"CallDoubleMethod",             CallDoubleMethod},
        {"CallDoubleMethodV",            CallDoubleMethodV},
        {"CallDoubleMethodA",            CallDoubleMethodA},
        {"CallVoidMethod",               CallVoidMethod},
        {"CallVoidMethodV",              CallVoidMethodV},
        {"CallVoidMethodA",              CallVoidMethodA},
        {"CallNonvirtualObjectMethod",   CallNonvirtualObjectMethod},
        {"CallNonvirtualObjectMethodV",  CallNonvirtualObjectMethodV},
        {"CallNonvirtualObjectMethodA",  CallNonvirtualObjectMethodA},
        {"CallNonvirtualBooleanMethod",  CallNonvirtualBooleanMethod},
        {"CallNonvirtualBooleanMethodV", CallNonvirtualBooleanMethodV},
        {"CallNonvirtualBooleanMethodA", CallNonvirtualBooleanMethodA},
        {"CallNonvirtualByteMethod",     CallNonvirtualByteMethod},
        {"CallNonvirtualByteMethodV",    CallNonvirtualByteMethodV},
        {"CallNonvirtualByteMethodA",    CallNonvirtualByteMethodA},
        {"CallNonvirtualCharMethod",     CallNonvirtualCharMethod},
        {"CallNonvirtualCharMethodV",    CallNonvirtualCharMethodV},
        {"CallNonvirtualCharMethodA",    CallNonvirtualCharMethodA},
        {"CallNonvirtualShortMethod",    CallNonvirtualShortMethod},
        {"CallNonvirtualShortMethodV",   CallNonvirtualShortMethodV},
        {"CallNonvirtualShortMethodA",   CallNonvirtualShortMethodA},
        {"CallNonvirtualIntMethod",      CallNonvirtualIntMethod},
        {"CallNonvirtualIntMethodV",     CallNonvirtualIntMethodV},
        {"CallNonvirtualIntMethodA",     CallNonvirtualIntMethodA},
        {"CallNonvirtualLongMethod",     CallNonvirtualLongMethod},
        {"CallNonvirtualLongMethodV",    CallNonvirtualLongMethodV},
        {"CallNonvirtualLongMethodA",    CallNonvirtualLongMethodA},
        {"CallNonvirtualFloatMethod",    CallNonvirtualFloatMethod},
        {"CallNonvirtualFloatMethodV",   CallNonvirtualFloatMethodV},
        {"CallNonvirtualFloatMethodA",   CallNonvirtualFloatMethodA},
        {"CallNonvirtualDoubleMethod",   CallNonvirtualDoubleMethod},
        {"CallNonvirtualDoubleMethodV",  CallNonvirtualDoubleMethodV},
        {"CallNonvirtualDoubleMethodA",  CallNonvirtualDoubleMethodA},
        {"CallNonvirtualVoidMethod",     CallNonvirtualVoidMethod},
        {"CallNonvirtualVoidMethodV",    CallNonvirtualVoidMethodV},
        {"CallNonvirtualVoidMethodA",    CallNonvirtualVoidMethodA},
        {"GetFieldID",                   GetFieldID},
        {"GetObjectField",               GetObjectField},
        {"GetBooleanField",              GetBooleanField},
        {"GetByteField",                 GetByteField},
        {"GetCharField",                 GetCharField},
        {"GetShortField",                GetShortField},
        {"GetIntField",                  GetIntField},
        {"GetLongField",                 GetLongField},
        {"GetFloatField",                GetFloatField},
        {"GetDoubleField",               GetDoubleField},
        {"SetObjectField",               SetObjectField},
        {"SetBooleanField",              SetBooleanField},
        {"SetByteField",                 SetByteField},
        {"SetCharField",                 SetCharField},
        {"SetShortField",                SetShortField},
        {"SetIntField",                  SetIntField},
        {"SetLongField",                 SetLongField},
        {"SetFloatField",                SetFloatField},
        {"SetDoubleField",               SetDoubleField},
        {"GetStaticMethodID",            GetStaticMethodID},
        {"CallStaticObjectMethod",       CallStaticObjectMethod},
        {"CallStaticObjectMethodV",      CallStaticObjectMethodV},
        {"CallStaticObjectMethodA",      CallStaticObjectMethodA},
        {"CallStaticBooleanMethod",      CallStaticBooleanMethod},
        {"CallStaticBooleanMethodV",     CallStaticBooleanMethodV},
        {"CallStaticBooleanMethodA",     CallStaticBooleanMethodA},
        {"CallStaticByteMethod",         CallStaticByteMethod},
        {"CallStaticByteMethodV",        CallStaticByteMethodV},
        {"CallStaticByteMethodA",        CallStaticByteMethodA},
        {"CallStaticCharMethod",         CallStaticCharMethod},
        {"CallStaticCharMethodV",        CallStaticCharMethodV},
        {"CallStaticCharMethodA",        CallStaticCharMethodA},
        {"CallStaticShortMethod",        CallStaticShortMethod},
        {"CallStaticShortMethodV",       CallStaticShortMethodV},
        {"CallStaticShortMethodA",       CallStaticShortMethodA},
        {"CallStaticIntMethod",          CallStaticIntMethod},
        {"CallStaticIntMethodV",         CallStaticIntMethodV},
        {"CallStaticIntMethodA",         CallStaticIntMethodA},
        {"CallStaticLongMethod",         CallStaticLongMethod},
        {"CallStaticLongMethodV",        CallStaticLongMethodV},
        {"CallStaticLongMethodA",        CallStaticLongMethodA},
        {"CallStaticFloatMethod",        CallStaticFloatMethod},
        {"CallStaticFloatMethodV",       CallStaticFloatMethodV},
        {"CallStaticFloatMethodA",       CallStaticFloatMethodA},
        {"CallStaticDoubleMethod",       CallStaticDoubleMethod},
        {"CallStaticDoubleMethodV",      CallStaticDoubleMethodV},
        {"CallStaticDoubleMethodA",      CallStaticDoubleMethodA},
        {"CallStaticVoidMethod",         CallStaticVoidMethod},
        {"CallStaticVoidMethodV",        CallStaticVoidMethodV},
        {"CallStaticVoidMethodA",        CallStaticVoidMethodA},
        {"GetStaticFieldID",             GetStaticFieldID},
        {"GetStaticObjectField",         GetStaticObjectField},
        {"GetStaticBooleanField",        GetStaticBooleanField},
        {"GetStaticByteField",           GetStaticByteField},
        {"GetStaticCharField",           GetStaticCharField},
        {"GetStaticShortField",          GetStaticShortField},
        {"GetStaticIntField",            GetStaticIntField},
        {"GetStaticLongField",           GetStaticLongField},
        {"GetStaticFloatField",          GetStaticFloatField},
        {"GetStaticDoubleField",         GetStaticDoubleField},
        {"SetStaticObjectField",         SetStaticObjectField},
        {"SetStaticBooleanField",        SetStaticBooleanField},
        {"SetStaticByteField",           SetStaticByteField},
        {"SetStaticCharField",           SetStaticCharField},
        {"SetStaticShortField",          SetStaticShortField},
        {"SetStaticIntField",            SetStaticIntField},
        {"SetStaticLongField",           SetStaticLongField},
        {"SetStaticFloatField",          SetStaticFloatField},
        {"SetStaticDoubleField",         SetStaticDoubleField},
        {"NewString",                    NewString},
        {"GetStringLength",              GetStringLength},
        {"GetStringChars",               GetStringChars},
        {"ReleaseStringChars",           ReleaseStringChars},
        {"NewStringUTF",                 NewStringUTF},
        {"GetStringUTFLength",           GetStringUTFLength},
        {"ReleaseStringUTFChars",        ReleaseStringUTFChars},
        {"GetArrayLength",               GetArrayLength},
        {"NewObjectArray",               NewObjectArray},
        {"GetObjectArrayElement",        GetObjectArrayElement},
        {"SetObjectArrayElement",        SetObjectArrayElement},
        {"NewBooleanArray",              NewBooleanArray},
        {"NewByteArray",                 NewByteArray},
        {"NewCharArray",                 NewCharArray},
        {"NewShortArray",                NewShortArray},
        {"NewIntArray",                  NewIntArray},
        {"NewLongArray",                 NewLongArray},
        {"NewFloatArray",                NewFloatArray},
        {"NewDoubleArray",               NewDoubleArray},
        {"GetBooleanArrayElements",      GetBooleanArrayElements},
        {"GetByteArrayElements",         GetByteArrayElements},
        {"GetCharArrayElements",         GetCharArrayElements},
        {"GetShortArrayElements",        GetShortArrayElements},
        {"GetIntArrayElements",          GetIntArrayElements},
        {"GetLongArrayElements",         GetLongArrayElements},
        {"GetFloatArrayElements",        GetFloatArrayElements},
        {"GetDoubleArrayElements",       GetDoubleArrayElements},
        {"ReleaseBooleanArrayElements",  ReleaseBooleanArrayElements},
        {"ReleaseByteArrayElements",     ReleaseByteArrayElements},
        {"ReleaseCharArrayElements",     ReleaseCharArrayElements},
        {"ReleaseShortArrayElements",    ReleaseShortArrayElements},
        {"ReleaseIntArrayElements",      ReleaseIntArrayElements},
        {"ReleaseLongArrayElements",     ReleaseLongArrayElements},
        {"ReleaseFloatArrayElements",    ReleaseFloatArrayElements},
        {"ReleaseDoubleArrayElements",   ReleaseDoubleArrayElements},
        {"GetBooleanArrayRegion",        GetBooleanArrayRegion},
        {"GetByteArrayRegion",           GetByteArrayRegion},
        {"GetCharArrayRegion",           GetCharArrayRegion},
        {"GetShortArrayRegion",          GetShortArrayRegion},
        {"GetIntArrayRegion",            GetIntArrayRegion},
        {"GetLongArrayRegion",           GetLongArrayRegion},
        {"GetFloatArrayRegion",          GetFloatArrayRegion},
        {"GetDoubleArrayRegion",         GetDoubleArrayRegion},
        {"SetBooleanArrayRegion",        SetBooleanArrayRegion},
        {"SetByteArrayRegion",           SetByteArrayRegion},
        {"SetCharArrayRegion",           SetCharArrayRegion},
        {"SetShortArrayRegion",          SetShortArrayRegion},
        {"SetIntArrayRegion",            SetIntArrayRegion},
        {"SetLongArrayRegion",           SetLongArrayRegion},
        {"SetFloatArrayRegion",          SetFloatArrayRegion},
        {"SetDoubleArrayRegion",         SetDoubleArrayRegion},
        {"RegisterNatives",              RegisterNatives},
        {"UnregisterNatives",            UnregisterNatives},
};


void DispatchJNIEnv::dispatch_env(std::string name, inst_trace_info_t *trace_info) {
    auto env_ptr = get_arg_register_value(&trace_info->pre_status, 0);
    trace_info->fun_call->args.push_back(fmt::format("env={:#x}", env_ptr));
    auto it = arg_handlers_.find(name);
    if (it != arg_handlers_.end()) {
        it->second(trace_info);
    }
}


#define JNI_TABLE_FUN(name)     env_fun_table.emplace(fun_table[(offsetof(JNINativeInterface, name))/sizeof(void *)],#name)

void DispatchJNIEnv::init_env_fun_table() {
    auto env = smjni::jni_provider::get_jni();
    const auto *fun_table = reinterpret_cast<const uintptr_t *>(env->functions);
    JNI_TABLE_FUN(GetVersion);
    JNI_TABLE_FUN(DefineClass);
    JNI_TABLE_FUN(FindClass);
    JNI_TABLE_FUN(FromReflectedMethod);
    JNI_TABLE_FUN(FromReflectedField);
    JNI_TABLE_FUN(ToReflectedMethod);
    JNI_TABLE_FUN(GetSuperclass);
    JNI_TABLE_FUN(IsAssignableFrom);
    JNI_TABLE_FUN(ToReflectedField);
    JNI_TABLE_FUN(Throw);
    JNI_TABLE_FUN(ThrowNew);
    JNI_TABLE_FUN(ExceptionOccurred);
    JNI_TABLE_FUN(ExceptionDescribe);
    JNI_TABLE_FUN(ExceptionClear);
    JNI_TABLE_FUN(FatalError);
    JNI_TABLE_FUN(PushLocalFrame);
    JNI_TABLE_FUN(PopLocalFrame);
    JNI_TABLE_FUN(NewGlobalRef);
    JNI_TABLE_FUN(DeleteGlobalRef);
    JNI_TABLE_FUN(DeleteLocalRef);
    JNI_TABLE_FUN(IsSameObject);
    JNI_TABLE_FUN(NewLocalRef);
    JNI_TABLE_FUN(EnsureLocalCapacity);
    JNI_TABLE_FUN(AllocObject);
    JNI_TABLE_FUN(NewObject);
    JNI_TABLE_FUN(NewObjectV);
    JNI_TABLE_FUN(NewObjectA);
    JNI_TABLE_FUN(GetObjectClass);
    JNI_TABLE_FUN(IsInstanceOf);
    JNI_TABLE_FUN(GetMethodID);
    JNI_TABLE_FUN(CallObjectMethod);
    JNI_TABLE_FUN(CallObjectMethodV);
    JNI_TABLE_FUN(CallObjectMethodA);
    JNI_TABLE_FUN(CallBooleanMethod);
    JNI_TABLE_FUN(CallBooleanMethodV);
    JNI_TABLE_FUN(CallBooleanMethodA);
    JNI_TABLE_FUN(CallByteMethod);
    JNI_TABLE_FUN(CallByteMethodV);
    JNI_TABLE_FUN(CallByteMethodA);
    JNI_TABLE_FUN(CallCharMethod);
    JNI_TABLE_FUN(CallCharMethodV);
    JNI_TABLE_FUN(CallCharMethodA);
    JNI_TABLE_FUN(CallShortMethod);
    JNI_TABLE_FUN(CallShortMethodV);
    JNI_TABLE_FUN(CallShortMethodA);
    JNI_TABLE_FUN(CallIntMethod);
    JNI_TABLE_FUN(CallIntMethodV);
    JNI_TABLE_FUN(CallIntMethodA);
    JNI_TABLE_FUN(CallLongMethod);
    JNI_TABLE_FUN(CallLongMethodV);
    JNI_TABLE_FUN(CallLongMethodA);
    JNI_TABLE_FUN(CallFloatMethod);
    JNI_TABLE_FUN(CallFloatMethodV);
    JNI_TABLE_FUN(CallFloatMethodA);
    JNI_TABLE_FUN(CallDoubleMethod);
    JNI_TABLE_FUN(CallDoubleMethodV);
    JNI_TABLE_FUN(CallDoubleMethodA);
    JNI_TABLE_FUN(CallVoidMethod);
    JNI_TABLE_FUN(CallVoidMethodV);
    JNI_TABLE_FUN(CallVoidMethodA);
    JNI_TABLE_FUN(CallNonvirtualObjectMethod);
    JNI_TABLE_FUN(CallNonvirtualObjectMethodV);
    JNI_TABLE_FUN(CallNonvirtualObjectMethodA);
    JNI_TABLE_FUN(CallNonvirtualBooleanMethod);
    JNI_TABLE_FUN(CallNonvirtualBooleanMethodV);
    JNI_TABLE_FUN(CallNonvirtualBooleanMethodA);
    JNI_TABLE_FUN(CallNonvirtualByteMethod);
    JNI_TABLE_FUN(CallNonvirtualByteMethodV);
    JNI_TABLE_FUN(CallNonvirtualByteMethodA);
    JNI_TABLE_FUN(CallNonvirtualCharMethod);
    JNI_TABLE_FUN(CallNonvirtualCharMethodV);
    JNI_TABLE_FUN(CallNonvirtualCharMethodA);
    JNI_TABLE_FUN(CallNonvirtualShortMethod);
    JNI_TABLE_FUN(CallNonvirtualShortMethodV);
    JNI_TABLE_FUN(CallNonvirtualShortMethodA);
    JNI_TABLE_FUN(CallNonvirtualIntMethod);
    JNI_TABLE_FUN(CallNonvirtualIntMethodV);
    JNI_TABLE_FUN(CallNonvirtualIntMethodA);
    JNI_TABLE_FUN(CallNonvirtualLongMethod);
    JNI_TABLE_FUN(CallNonvirtualLongMethodV);
    JNI_TABLE_FUN(CallNonvirtualLongMethodA);
    JNI_TABLE_FUN(CallNonvirtualFloatMethod);
    JNI_TABLE_FUN(CallNonvirtualFloatMethodV);
    JNI_TABLE_FUN(CallNonvirtualFloatMethodA);
    JNI_TABLE_FUN(CallNonvirtualDoubleMethod);
    JNI_TABLE_FUN(CallNonvirtualDoubleMethodV);
    JNI_TABLE_FUN(CallNonvirtualDoubleMethodA);
    JNI_TABLE_FUN(CallNonvirtualVoidMethod);
    JNI_TABLE_FUN(CallNonvirtualVoidMethodV);
    JNI_TABLE_FUN(CallNonvirtualVoidMethodA);
    JNI_TABLE_FUN(GetFieldID);
    JNI_TABLE_FUN(GetObjectField);
    JNI_TABLE_FUN(GetBooleanField);
    JNI_TABLE_FUN(GetByteField);
    JNI_TABLE_FUN(GetCharField);
    JNI_TABLE_FUN(GetShortField);
    JNI_TABLE_FUN(GetIntField);
    JNI_TABLE_FUN(GetLongField);
    JNI_TABLE_FUN(GetFloatField);
    JNI_TABLE_FUN(GetDoubleField);
    JNI_TABLE_FUN(SetObjectField);
    JNI_TABLE_FUN(SetBooleanField);
    JNI_TABLE_FUN(SetByteField);
    JNI_TABLE_FUN(SetCharField);
    JNI_TABLE_FUN(SetShortField);
    JNI_TABLE_FUN(SetIntField);
    JNI_TABLE_FUN(SetLongField);
    JNI_TABLE_FUN(SetFloatField);
    JNI_TABLE_FUN(SetDoubleField);
    JNI_TABLE_FUN(GetStaticMethodID);
    JNI_TABLE_FUN(CallStaticObjectMethod);
    JNI_TABLE_FUN(CallStaticObjectMethodV);
    JNI_TABLE_FUN(CallStaticObjectMethodA);
    JNI_TABLE_FUN(CallStaticBooleanMethod);
    JNI_TABLE_FUN(CallStaticBooleanMethodV);
    JNI_TABLE_FUN(CallStaticBooleanMethodA);
    JNI_TABLE_FUN(CallStaticByteMethod);
    JNI_TABLE_FUN(CallStaticByteMethodV);
    JNI_TABLE_FUN(CallStaticByteMethodA);
    JNI_TABLE_FUN(CallStaticCharMethod);
    JNI_TABLE_FUN(CallStaticCharMethodV);
    JNI_TABLE_FUN(CallStaticCharMethodA);
    JNI_TABLE_FUN(CallStaticShortMethod);
    JNI_TABLE_FUN(CallStaticShortMethodV);
    JNI_TABLE_FUN(CallStaticShortMethodA);
    JNI_TABLE_FUN(CallStaticIntMethod);
    JNI_TABLE_FUN(CallStaticIntMethodV);
    JNI_TABLE_FUN(CallStaticIntMethodA);
    JNI_TABLE_FUN(CallStaticLongMethod);
    JNI_TABLE_FUN(CallStaticLongMethodV);
    JNI_TABLE_FUN(CallStaticLongMethodA);
    JNI_TABLE_FUN(CallStaticFloatMethod);
    JNI_TABLE_FUN(CallStaticFloatMethodV);
    JNI_TABLE_FUN(CallStaticFloatMethodA);
    JNI_TABLE_FUN(CallStaticDoubleMethod);
    JNI_TABLE_FUN(CallStaticDoubleMethodV);
    JNI_TABLE_FUN(CallStaticDoubleMethodA);
    JNI_TABLE_FUN(CallStaticVoidMethod);
    JNI_TABLE_FUN(CallStaticVoidMethodV);
    JNI_TABLE_FUN(CallStaticVoidMethodA);
    JNI_TABLE_FUN(GetStaticFieldID);
    JNI_TABLE_FUN(GetStaticObjectField);
    JNI_TABLE_FUN(GetStaticBooleanField);
    JNI_TABLE_FUN(GetStaticByteField);
    JNI_TABLE_FUN(GetStaticCharField);
    JNI_TABLE_FUN(GetStaticShortField);
    JNI_TABLE_FUN(GetStaticIntField);
    JNI_TABLE_FUN(GetStaticLongField);
    JNI_TABLE_FUN(GetStaticFloatField);
    JNI_TABLE_FUN(GetStaticDoubleField);
    JNI_TABLE_FUN(SetStaticObjectField);
    JNI_TABLE_FUN(SetStaticBooleanField);
    JNI_TABLE_FUN(SetStaticByteField);
    JNI_TABLE_FUN(SetStaticCharField);
    JNI_TABLE_FUN(SetStaticShortField);
    JNI_TABLE_FUN(SetStaticIntField);
    JNI_TABLE_FUN(SetStaticLongField);
    JNI_TABLE_FUN(SetStaticFloatField);
    JNI_TABLE_FUN(SetStaticDoubleField);
    JNI_TABLE_FUN(NewString);
    JNI_TABLE_FUN(GetStringLength);
    JNI_TABLE_FUN(GetStringChars);
    JNI_TABLE_FUN(ReleaseStringChars);
    JNI_TABLE_FUN(NewStringUTF);
    JNI_TABLE_FUN(GetStringUTFLength);
    JNI_TABLE_FUN(GetStringUTFChars);
    JNI_TABLE_FUN(ReleaseStringUTFChars);
    JNI_TABLE_FUN(GetArrayLength);
    JNI_TABLE_FUN(NewObjectArray);
    JNI_TABLE_FUN(GetObjectArrayElement);
    JNI_TABLE_FUN(SetObjectArrayElement);
    JNI_TABLE_FUN(NewBooleanArray);
    JNI_TABLE_FUN(NewByteArray);
    JNI_TABLE_FUN(NewCharArray);
    JNI_TABLE_FUN(NewShortArray);
    JNI_TABLE_FUN(NewIntArray);
    JNI_TABLE_FUN(NewLongArray);
    JNI_TABLE_FUN(NewFloatArray);
    JNI_TABLE_FUN(NewDoubleArray);
    JNI_TABLE_FUN(GetBooleanArrayElements);
    JNI_TABLE_FUN(ReleaseBooleanArrayElements);
    JNI_TABLE_FUN(GetByteArrayElements);
    JNI_TABLE_FUN(ReleaseByteArrayElements);
    JNI_TABLE_FUN(GetCharArrayElements);
    JNI_TABLE_FUN(ReleaseCharArrayElements);
    JNI_TABLE_FUN(GetShortArrayElements);
    JNI_TABLE_FUN(ReleaseShortArrayElements);
    JNI_TABLE_FUN(GetIntArrayElements);
    JNI_TABLE_FUN(ReleaseIntArrayElements);
    JNI_TABLE_FUN(GetLongArrayElements);
    JNI_TABLE_FUN(ReleaseLongArrayElements);
    JNI_TABLE_FUN(GetFloatArrayElements);
    JNI_TABLE_FUN(ReleaseFloatArrayElements);
    JNI_TABLE_FUN(GetDoubleArrayElements);
    JNI_TABLE_FUN(ReleaseDoubleArrayElements);
    JNI_TABLE_FUN(GetBooleanArrayRegion);
    JNI_TABLE_FUN(SetBooleanArrayRegion);
    JNI_TABLE_FUN(GetByteArrayRegion);
    JNI_TABLE_FUN(SetByteArrayRegion);
    JNI_TABLE_FUN(GetCharArrayRegion);
    JNI_TABLE_FUN(SetCharArrayRegion);
    JNI_TABLE_FUN(GetShortArrayRegion);
    JNI_TABLE_FUN(SetShortArrayRegion);
    JNI_TABLE_FUN(GetIntArrayRegion);
    JNI_TABLE_FUN(SetIntArrayRegion);
    JNI_TABLE_FUN(GetLongArrayRegion);
    JNI_TABLE_FUN(SetLongArrayRegion);
    JNI_TABLE_FUN(GetFloatArrayRegion);
    JNI_TABLE_FUN(SetFloatArrayRegion);
    JNI_TABLE_FUN(GetDoubleArrayRegion);
    JNI_TABLE_FUN(SetDoubleArrayRegion);
    JNI_TABLE_FUN(RegisterNatives);
    JNI_TABLE_FUN(UnregisterNatives);
    JNI_TABLE_FUN(MonitorEnter);
    JNI_TABLE_FUN(MonitorExit);
    JNI_TABLE_FUN(GetJavaVM);
    JNI_TABLE_FUN(GetStringRegion);
    JNI_TABLE_FUN(GetStringUTFRegion);
    JNI_TABLE_FUN(GetPrimitiveArrayCritical);
    JNI_TABLE_FUN(ReleasePrimitiveArrayCritical);
    JNI_TABLE_FUN(GetStringCritical);
    JNI_TABLE_FUN(ReleaseStringCritical);
    JNI_TABLE_FUN(NewWeakGlobalRef);
    JNI_TABLE_FUN(DeleteWeakGlobalRef);
    JNI_TABLE_FUN(ExceptionCheck);
    JNI_TABLE_FUN(NewDirectByteBuffer);
    JNI_TABLE_FUN(GetDirectBufferAddress);
    JNI_TABLE_FUN(GetDirectBufferCapacity);
    JNI_TABLE_FUN(GetObjectRefType);
}

bool DispatchJNIEnv::dispatch_args(inst_trace_info_t *info) {
    if (!is_module_address(info->fun_call->fun_address)) {
        return false;
    }
    info->fun_call->call_module_name = "libart.so";
    auto env_fun_find = env_fun_table.find(info->fun_call->fun_address);
    if (env_fun_find != env_fun_table.end()) {
        auto name = env_fun_find->second;
        info->fun_call->fun_name = name;
        dispatch_env(name, info);
        return true;
    }
    return true;
}


static const std::unordered_map<std::string, std::function<void(inst_trace_info_t *)>> ret_handlers_ = {


};


bool DispatchJNIEnv::dispatch_ret(inst_trace_info_t *info, const QBDI::GPRState *ret_status) {
    if (!is_module_address(info->fun_call->fun_address)) {
        return false;
    }
    auto ret_value = get_ret_register_value(ret_status, 0);


    if (info->fun_call->fun_name == "GetSuperclass") {
        auto env = smjni::jni_provider::get_jni();
        info->fun_call->ret_value = (fmt::format("ret class={}:{:#x}",
                                                 get_jni_class_or_java_class_name(env,
                                                                                  (jclass) ret_value),
                                                 ret_value));
    } else {
        add_common_return_value(info, ret_status);
    }
    return true;
}