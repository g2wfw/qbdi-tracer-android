//
//
//

#include <jni.h>
#include <spdlog/fmt/fmt.h>
#include <smjni.h>
#include "dispatch_jni_env.h"
#include "jni_internal.h"

static std::unordered_map<uintptr_t, std::string> env_fun_table;

DispatchJNIEnv *DispatchJNIEnv::get_instance() {
    static DispatchJNIEnv dispatchJniEnv;
    return &dispatchJniEnv;
}


DispatchJNIEnv::DispatchJNIEnv() {
    init_env_fun_table();
    auto module = gum_process_find_module_by_name("libart.so");
    auto range = gum_module_get_range(module);
    module_range.base = range->base_address;
    module_range.end = range->size + range->base_address;
    gum_module_enumerate_exports(module, dispatch_export_func, this);
    g_object_unref(module);
}


inline bool starts_with(const std::string &str, const std::string &prefix) {
    return str.size() >= prefix.size() && str.compare(0, prefix.size(), prefix) == 0;
}

inline bool ends_with(const std::string &str, const std::string &suffix) {
    if (suffix.size() > str.size()) return false;
    return std::equal(suffix.rbegin(), suffix.rend(), str.rbegin());
}

void DispatchJNIEnv::dispatch_env(std::string name, inst_trace_info_t *trace_info) {
    auto env = smjni::jni_provider::get_jni();
    auto env_ptr = get_arg_register_value(&trace_info->pre_status, 0);
    auto arg0 = get_arg_register_value(&trace_info->pre_status, 1);
    auto arg1 = get_arg_register_value(&trace_info->pre_status, 2);
    auto arg2 = get_arg_register_value(&trace_info->pre_status, 3);
    auto arg3 = get_arg_register_value(&trace_info->pre_status, 4);
    trace_info->fun_call->args.push_back(fmt::format("env={:#x}", env_ptr));

    static const std::unordered_map<std::string, std::function<void()>> handlers = {
            {"FindClass",               [&]() {
                trace_info->fun_call->args.push_back(
                        fmt::format("name={}", read_string_from_address(arg0)));
            }},
            {"FromReflectedMethod",     [&]() {
                trace_info->fun_call->args.push_back(
                        fmt::format("method={}", get_reflected_method_info(env, (jobject) arg0)));
            }},
            {"FromReflectedField",      [&]() {
                trace_info->fun_call->args.push_back(
                        fmt::format("field={}", get_reflected_field_info(env, (jobject) arg0)));
            }},
            {"ToReflectedMethod",       [&]() {
                trace_info->fun_call->args.push_back(fmt::format("methodID={}",
                                                                 get_jni_method_signature(env,
                                                                                          (jmethodID) arg1)));
            }},
            {"ToReflectedField",        [&]() {
                trace_info->fun_call->args.push_back(
                        fmt::format("fieldID={}", get_jni_field_signature(env, (jfieldID) arg1)));
            }},
            {"GetSuperclass",           [&]() {
                trace_info->fun_call->args.push_back(fmt::format("clazz={}",
                                                                 get_jni_class_or_java_class_name(
                                                                         env, (jclass) arg0)));
            }},
            {"IsAssignableFrom",        [&]() {
                trace_info->fun_call->args.push_back(fmt::format("clazz1={}",
                                                                 get_jni_class_or_java_class_name(
                                                                         env, (jclass) arg0)));
                trace_info->fun_call->args.push_back(fmt::format("clazz2={}",
                                                                 get_jni_class_or_java_class_name(
                                                                         env, (jclass) arg1)));
            }},
            {"ThrowNew",                [&]() {
                trace_info->fun_call->args.push_back(fmt::format("clazz={}",
                                                                 get_jni_class_or_java_class_name(
                                                                         env, (jclass) arg0)));
                trace_info->fun_call->args.push_back(
                        fmt::format("message={}", read_string_from_address(arg1)));
            }},
            {"FatalError",              [&]() {
                trace_info->fun_call->args.push_back(
                        fmt::format("message={}", read_string_from_address(arg0)));
            }},
            {"PushLocalFrame",          [&]() {
                trace_info->fun_call->args.push_back(fmt::format("capacity={:#x}", arg0));
            }},
            {"PopLocalFrame",           [&]() {
                trace_info->fun_call->args.push_back(
                        fmt::format("object={}", get_jni_jobject_name(env, (jobject) arg0)));
            }},
            {"NewGlobalRef",            [&]() {
                trace_info->fun_call->args.push_back(
                        fmt::format("object={}", get_jni_jobject_name(env, (jobject) arg0)));
            }},
            {"DeleteGlobalRef",         [&]() {
                trace_info->fun_call->args.push_back(
                        fmt::format("object={}", get_jni_jobject_name(env, (jobject) arg0)));
            }},
            {"DeleteLocalRef",          [&]() {
                trace_info->fun_call->args.push_back(
                        fmt::format("object={}", get_jni_jobject_name(env, (jclass) arg0)));
            }},
            {"IsSameObject",            [&]() {
                trace_info->fun_call->args.push_back(
                        fmt::format("ref1={}", get_jni_jobject_name(env, (jobject) arg0)));
                trace_info->fun_call->args.push_back(
                        fmt::format("ref2={}", get_jni_jobject_name(env, (jobject) arg1)));
            }},
            {"NewLocalRef",             [&]() {
                trace_info->fun_call->args.push_back(
                        fmt::format("object={}", get_jni_jobject_name(env, (jobject) arg0)));
            }},
            {"EnsureLocalCapacity",     [&]() {
                trace_info->fun_call->args.push_back(fmt::format("capacity={:#x}", arg0));
            }},
            {"AllocObject",             [&]() {
                trace_info->fun_call->args.push_back(fmt::format("clazz={}",
                                                                 get_jni_class_or_java_class_name(
                                                                         env, (jclass) arg0)));
            }},
            {"NewObject",               [&]() {
                trace_info->fun_call->args.push_back(fmt::format("clazz={}",
                                                                 get_jni_class_or_java_class_name(
                                                                         env, (jclass) arg0)));
                trace_info->fun_call->args.push_back(
                        fmt::format("method={}", get_jni_method_signature(env, (jmethodID) arg1)));
            }},
            {"NewObjectV",              [&]() {
                if (env->ExceptionCheck()) {
                    env->ExceptionDescribe();
                }
                trace_info->fun_call->args.push_back(fmt::format("clazz={}",
                                                                 get_jni_class_or_java_class_name(
                                                                         env, (jclass) arg0)));
                trace_info->fun_call->args.push_back(
                        fmt::format("method={}", get_jni_method_signature(env, (jmethodID) arg1)));
                if (env->ExceptionCheck()) {
                    env->ExceptionDescribe();
                }
            }},
            {"NewObjectA",              [&]() {
                trace_info->fun_call->args.push_back(fmt::format("clazz={}",
                                                                 get_jni_class_or_java_class_name(
                                                                         env, (jclass) arg0)));
                trace_info->fun_call->args.push_back(
                        fmt::format("method={}", get_jni_method_signature(env, (jmethodID) arg1)));
            }},
            {"GetObjectClass",          [&]() {
                trace_info->fun_call->args.push_back(
                        fmt::format("object={}", get_jni_jobject_name(env, (jobject) arg0)));
            }},
            {"IsInstanceOf",            [&]() {
                trace_info->fun_call->args.push_back(
                        fmt::format("object={}", get_jni_jobject_name(env, (jobject) arg0)));
                trace_info->fun_call->args.push_back(fmt::format("clazz={}",
                                                                 get_jni_class_or_java_class_name(
                                                                         env, (jclass) arg1)));
            }},
            {"GetMethodID",             [&]() {
                trace_info->fun_call->args.push_back(
                        fmt::format("object={}", get_jni_jobject_name(env, (jobject) arg0)));
                trace_info->fun_call->args.push_back(
                        fmt::format("name={}", read_string_from_address(arg1)));
                trace_info->fun_call->args.push_back(
                        fmt::format("sig={}", read_string_from_address(arg2)));
            }},
            {"Call",                    [&]() {
                if (starts_with(name, "CallStatic")) {
                    trace_info->fun_call->args.push_back(fmt::format("object={}",
                                                                     get_jni_class_or_java_class_name(
                                                                             env, (jclass) arg0)));
                    trace_info->fun_call->args.push_back(fmt::format("method={}",
                                                                     get_jni_method_signature(env,
                                                                                              (jmethodID) arg1)));
                } else {
                    trace_info->fun_call->args.push_back(
                            fmt::format("object={}", get_jni_jobject_name(env, (jobject) arg0)));
                    trace_info->fun_call->args.push_back(fmt::format("method={}",
                                                                     get_jni_method_signature(env,
                                                                                              (jmethodID) arg1)));
                }
            }},
            {"GetField",                [&]() {
                trace_info->fun_call->args.push_back(
                        fmt::format("object={}", get_jni_jobject_name(env, (jobject) arg0)));
                trace_info->fun_call->args.push_back(
                        fmt::format("field={}", get_jni_field_signature(env, (jfieldID) arg1)));
            }},
            {"GetFieldID",              [&]() {
                trace_info->fun_call->args.push_back(
                        fmt::format("name={}", read_string_from_address(arg1)));
                trace_info->fun_call->args.push_back(
                        fmt::format("sig={}", read_string_from_address(arg2)));
            }},
            {"SetField",                [&]() {
                trace_info->fun_call->args.push_back(
                        fmt::format("object={}", get_jni_jobject_name(env, (jobject) arg0)));
                trace_info->fun_call->args.push_back(
                        fmt::format("field={}", get_jni_field_signature(env, (jfieldID) arg1)));
                if (name == "SetStaticObjectField" || name == "SetObjectField") {
                    trace_info->fun_call->args.push_back(
                            fmt::format("value={}", get_jni_object_to_string(env, (jobject) arg2)));
                } else {
                    trace_info->fun_call->args.push_back(fmt::format("value={:#x}", arg2));
                }
            }},
            {"NewStringUTF",            [&]() {
                trace_info->fun_call->args.push_back(
                        fmt::format("string={}", read_string_from_address(arg0)));
            }},
            {"GetStringUTFLength",      [&]() {
                trace_info->fun_call->args.push_back(
                        fmt::format("string={}", jstring_to_string(env, (jstring) arg0)));
            }},
            {"GetStringUTFChars",       [&]() {
                trace_info->fun_call->args.push_back(
                        fmt::format("string={}", jstring_to_string(env, (jstring) arg0)));
            }},
            {"ReleaseStringUTFChars",   [&]() {
                trace_info->fun_call->args.push_back(
                        fmt::format("string={}", jstring_to_string(env, (jstring) arg0)));
            }},
            {"GetArrayLength",          [&]() {
                trace_info->fun_call->args.push_back(
                        fmt::format("array={}:{:#x}", get_jni_jobject_name(env, (jobject) arg0),
                                    arg0));
            }},
            {"NewObjectArray",          [&]() {
                trace_info->fun_call->args.push_back(fmt::format("length={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("elementClass={}",
                                                                 get_jni_class_or_java_class_name(
                                                                         env, (jclass) arg1)));
            }},
            {"NewArray",                [&]() {
                trace_info->fun_call->args.push_back(fmt::format("length={:#x}", arg0));
            }},
            {"GetObjectArrayElement",   [&]() {
                trace_info->fun_call->args.push_back(
                        fmt::format("array={}:{:#x}", get_jni_jobject_name(env, (jobject) arg0),
                                    arg0));
                trace_info->fun_call->args.push_back(fmt::format("index={:#x}", arg1));
            }},
            {"SetObjectArrayElement",   [&]() {
                trace_info->fun_call->args.push_back(
                        fmt::format("array={}:{:#x}", get_jni_jobject_name(env, (jobject) arg0),
                                    arg0));
                trace_info->fun_call->args.push_back(fmt::format("index={:#x}", arg1));
                trace_info->fun_call->args.push_back(
                        fmt::format("value={}", get_jni_object_to_string(env, (jobject) arg2)));
            }},
            {"GetArrayElements",        [&]() {
                trace_info->fun_call->args.push_back(fmt::format("array={:#x}", arg0));
            }},
            {"ReleaseArrayElements",    [&]() {
                trace_info->fun_call->args.push_back(fmt::format("array={:#x}", arg0));
            }},
            {"GetArrayRegion",          [&]() {
                trace_info->fun_call->args.push_back(fmt::format("array={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("start={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("len={:#x}", arg2));
                trace_info->fun_call->args.push_back(fmt::format("buf={:#x}", arg3));
            }},
            {"SetRegion",               [&]() {
                trace_info->fun_call->args.push_back(fmt::format("array={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("start={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("len={:#x}", arg2));
                trace_info->fun_call->args.push_back(fmt::format("buf={:#x}", arg3));
            }},
            {"RegisterNatives",         [&]() {
                trace_info->fun_call->args.push_back(fmt::format("class={}",
                                                                 get_jni_class_or_java_class_name(
                                                                         env, (jclass) arg0)));
                JNINativeMethod *method = (JNINativeMethod *) arg1;
                auto size = arg2;
                std::vector<std::string> methods;
                for (int i = 0; i < size; ++i) {
                    methods.emplace_back(fmt::format("{}{}{}", method[i].name, method[i].signature,
                                                     method[i].fnPtr));
                }
                auto str = join(methods, ",");
                trace_info->fun_call->args.push_back(fmt::format("methods={}", str));
                trace_info->fun_call->args.push_back(fmt::format("nMethods={:#x}", arg2));
            }},
            {"UnregisterNatives",       [&]() {
                trace_info->fun_call->args.push_back(fmt::format("class={}",
                                                                 get_jni_class_or_java_class_name(
                                                                         env, (jclass) arg0)));
            }},
            {"Monitor",                 [&]() {
                trace_info->fun_call->args.push_back(
                        fmt::format("object={}", get_jni_jobject_name(env, (jobject) arg0)));
            }},
            {"NewWeakGlobalRef",        [&]() {
                trace_info->fun_call->args.push_back(
                        fmt::format("object={}", get_jni_jobject_name(env, (jobject) arg0)));
            }},
            {"DeleteWeakGlobalRef",     [&]() {
                trace_info->fun_call->args.push_back(
                        fmt::format("object={}", get_jni_jobject_name(env, (jobject) arg0)));
            }},
            {"NewDirectByteBuffer",     [&]() {
                trace_info->fun_call->args.push_back(fmt::format("address={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("capacity={:#x}", arg1));
            }},
            {"GetDirectBufferAddress",  [&]() {
                trace_info->fun_call->args.push_back(fmt::format("buf={:#x}", arg0));
            }},
            {"GetDirectBufferCapacity", [&]() {
                trace_info->fun_call->args.push_back(fmt::format("buf={:#x}", arg0));
            }},
    };
    auto it = handlers.find(name);
    if (it != handlers.end()) {
        it->second();
    } else if (starts_with(name, "Call")) {
        handlers.at("Call")();
    } else if (starts_with(name, "Get") && ends_with(name, "Field")) {
        handlers.at("GetField")();
    } else if (starts_with(name, "Set") && ends_with(name, "Field")) {
        handlers.at("SetField")();
    } else if (starts_with(name, "New") && ends_with(name, "Array")) {
        handlers.at("NewArray")();
    } else if (starts_with(name, "Get") && ends_with(name, "ArrayElements")) {
        handlers.at("GetArrayElements")();
    } else if (starts_with(name, "Release") && ends_with(name, "ArrayElements")) {
        handlers.at("ReleaseArrayElements")();
    } else if (starts_with(name, "Get") && ends_with(name, "ArrayRegion")) {
        handlers.at("GetArrayRegion")();
    } else if (starts_with(name, "Set") && ends_with(name, "Region")) {
        handlers.at("SetRegion")();
    } else if (starts_with(name, "Monitor")) {
        handlers.at("Monitor")();
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
    auto env_fun_find = env_fun_table.find(info->fun_call->fun_address);
    if (env_fun_find != env_fun_table.end()) {
        auto name = env_fun_find->second;
        info->fun_call->fun_name = name;
        dispatch_env(name, info);
        return true;
    }
    return true;
}

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
        add_common_return_value(info,ret_status);
    }
    return true;
}
