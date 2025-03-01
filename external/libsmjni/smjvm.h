//
// Created by xiaob on 2020/4/10.
//

#ifndef CPPJVM_SMJVM_H
#define CPPJVM_SMJVM_H

#include <vector>
#include <string>
#include <jni.h>
#ifndef __ANDROID__
namespace smjni {
#if defined(JVM_STATIC_LINK)
    static constexpr bool jvm_static_link = true;
#else
    static constexpr bool jvm_static_link = false;
#endif
    using JNI_CreateJavaVM_t = std::add_pointer_t<decltype(JNI_CreateJavaVM)>;
    using JNI_GetCreatedJavaVMs_t = std::add_pointer_t<decltype(JNI_GetCreatedJavaVMs)>;

    std::vector<std::string> get_potential_libjvm_paths();

    int try_dlopen(std::vector<std::string> potential_paths, void *&out_handle);


    class JVM {
    public:
        void add_classpath(std::string classpath_option);

        void add_classpath(std::vector<std::string> classpath_option);

        void apply_classpath_option();

        JNIEnv *&env() {
            return (this->env_);
        }

        JavaVM *vm() {
            return this->jvm_;
        }
        void relase_javavm();

        JVM();


    public:
        static JVM &instance();


    private:
        static void destroy(void);


        inline static thread_local JVM *instance_ = nullptr;
    private:
        std::vector<std::string> classpath_options;

        JavaVM *jvm_{nullptr};
        JNIEnv *env_{nullptr};
    };


}

#endif
#endif //CPPJVM_SMJVM_H
