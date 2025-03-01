//
// Created by xiaob on 2020/4/10.
//


#include <dlfcn.h>
#include <mutex>
#include <numeric>
#include <iostream>
#include "smjvm.h"

#ifndef __ANDROID__

namespace smjni {
    static inline std::mutex jvm_mu;

    std::vector<std::string> get_potential_libjvm_paths() {
        std::vector<std::string> libjvm_potential_paths;

        std::vector<std::string> search_prefixes;
        std::vector<std::string> search_suffixes;
        std::string file_name;

// From heuristics
#ifdef __WIN32
        search_prefixes = {""};
        search_suffixes = {"/jre/bin/server", "/bin/server"};
        file_name = "smjni.dll";
#elif __APPLE__
        search_prefixes = {""};
  search_suffixes = {"", "/jre/lib/server", "/lib/server"};
  file_name = "libjvm.dylib";

// SFrame uses /usr/libexec/java_home to find JAVA_HOME; for now we are
// expecting users to set an environment variable
#else
        search_prefixes = {
                "/usr/lib/jvm/default-java",
                "/usr/lib/jvm/java",
                "/usr/lib/jvm",
                "/usr/lib64/jvm",

                "/usr/local/lib/jvm/default-java",
                "/usr/local/lib/jvm/java",
                "/usr/local/lib/jvm",
                "/usr/local/lib64/jvm",

                "/usr/local/lib/jvm/java-7-openjdk-amd64",
                "/usr/lib/jvm/java-7-openjdk-amd64",

                "/usr/local/lib/jvm/java-6-openjdk-amd64",
                "/usr/lib/jvm/java-6-openjdk-amd64",

                "/usr/lib/jvm/java-8-openjdk-amd64",
                "/usr/local/lib/jvm/java-8-openjdk-amd64",

                "/usr/lib/jvm/java-10-openjdk-amd64",
                "/usr/local/lib/jvm/java-10-openjdk-amd64",

                "/usr/lib/jvm/java-11-openjdk-amd64",
                "/usr/local/lib/jvm/java-11-openjdk-amd64",

                "/usr/lib/jvm/java-6-oracle",
                "/usr/lib/jvm/java-7-oracle",
                "/usr/lib/jvm/java-8-oracle",

                "/usr/local/lib/jvm/java-6-oracle",
                "/usr/local/lib/jvm/java-7-oracle",
                "/usr/local/lib/jvm/java-8-oracle",

                "/usr/lib/jvm/java-9-jdk",
                "/usr/local/lib/jvm/java-9-jdk",

                "/usr/lib/jvm/java-10-jdk",
                "/usr/local/lib/jvm/java-10-jdk",

                "/usr/lib/jvm/java-11-jdk",
                "/usr/local/lib/jvm/java-11-jdk",

                "/usr/lib/jvm/default",
                "/usr/lib/jvm/default-runtime",

                "/usr/java/latest",
        };
        search_suffixes = {"", "/jre/lib/amd64/server", "jre/lib/amd64", "lib/server", "/lib/amd64/server/"};
        file_name = "libjvm.so";
#endif
        // From direct environment variable
        char *env_value = NULL;
        if ((env_value = getenv("JAVA_HOME")) != NULL) {
            search_prefixes.insert(search_prefixes.begin(), env_value);
        }

        // Generate cross product between search_prefixes, search_suffixes, and file_name
        for (const std::string &prefix : search_prefixes) {
            for (const std::string &suffix : search_suffixes) {
                std::string path = prefix + "/" + suffix + "/" + file_name;
                libjvm_potential_paths.push_back(path);

            }
        }
        /*std::vector<std::string> path;
        path.push_back(
                "/mnt/d/openjdk-build/workspace/build/src/build/linux-x86_64-normal-server-slowdebug/jdk/lib/amd64/server/libjvm.so");
        return path;*/
        return libjvm_potential_paths;
    }

    int try_dlopen(std::vector<std::string> potential_paths, void *&out_handle) {
        for (const std::string &i : potential_paths) {
            out_handle = dlopen(i.c_str(), RTLD_NOW | RTLD_LOCAL);

            if (out_handle != nullptr) {
                break;
            }
        }

        if (out_handle == nullptr) {
            return 0;
        }
        return 1;
    }

    bool resolve(JNI_CreateJavaVM_t &hdl, JNI_GetCreatedJavaVMs_t &hdl2) {
        if (jvm_static_link) {
            hdl = &JNI_CreateJavaVM;
            hdl2 = &JNI_GetCreatedJavaVMs;
            return true;
        } else {
            std::vector<std::string> paths = get_potential_libjvm_paths();
            void *handler = nullptr;
            if (try_dlopen(paths, handler)) {
                auto &&fptr = reinterpret_cast<JNI_CreateJavaVM_t>(dlsym(handler, "JNI_CreateJavaVM"));
                if (fptr != nullptr) {
                    hdl = fptr;
                } else {
                    return false;
                }

                auto &&fptr_get_created = reinterpret_cast<JNI_GetCreatedJavaVMs_t>(dlsym(handler,
                                                                                          "JNI_GetCreatedJavaVMs"));
                if (fptr_get_created != nullptr) {
                    hdl2 = fptr_get_created;
                } else {
                    return false;
                }
                return true;
            }
            return false;
        }

    }

    JVM &JVM::instance() {
        const std::lock_guard<std::mutex> lock(jvm_mu);
        if (instance_ == nullptr) {
            instance_ = new JVM{};
            std::atexit(destroy);
        }
        return *instance_;
    }

    void JVM::destroy(void) {
        const std::lock_guard<std::mutex> lock(jvm_mu);
        delete instance_;
    }

    JVM::JVM() {


    }


    void JVM::add_classpath(std::vector<std::string> classpath_option) {
        this->classpath_options.insert(classpath_options.end(), classpath_option.begin(), classpath_option.end());
    }

    void JVM::apply_classpath_option() {
        if (jvm_ != nullptr) {
            this->relase_javavm();
        }
        JNI_CreateJavaVM_t jvm_fnc = nullptr;
        JNI_GetCreatedJavaVMs_t jvm_get_created = nullptr;
        if (not resolve(jvm_fnc, jvm_get_created)) {
            std::cerr << "[-] Can't resolve JVM symbols" << std::endl;
            return;
        }
        // 1. Get the number of JVM created
        int nJVMs;
        jvm_get_created(nullptr, 0, &nJVMs);
        if (nJVMs > 0) {
            jvm_get_created(&this->jvm_, 1, &nJVMs);
            this->jvm_->AttachCurrentThread(reinterpret_cast<void **>(&this->env_), nullptr);
            return;
        }
        std::string classpath = "";
        for (const auto &option : classpath_options) {
            classpath.append(option);
            classpath.append(":");
        }
        JavaVMOption opt[2];

        std::string classpath_option = "-Djava.class.path=" + classpath;
        opt[0].optionString = const_cast<char *>(classpath_option.c_str());
        opt[1].optionString = const_cast<char *>("-Djava.awt.headless=true");


        JavaVMInitArgs args;
        args.version = JNI_VERSION_1_6;
        args.options = opt;
        args.nOptions = 2;
        args.ignoreUnrecognized = JNI_TRUE;

        int created = jvm_fnc(&(this->jvm_), reinterpret_cast<void **>(&this->env_), &args);

        // It mays occurs if a JVM is already created
        if (created != JNI_OK or this->jvm_ == nullptr) {
            std::cerr << "[-] Error while creating the JVM" << std::endl;
            std::abort();
        }

    }

    void JVM::add_classpath(std::string classpath_option) {
        this->classpath_options.push_back(classpath_option);
    }

    void JVM::relase_javavm() {
        if (this->jvm_ != nullptr) {
            this->jvm_->DestroyJavaVM();
            //delete this->jvm_;
        }
    }
}
#endif