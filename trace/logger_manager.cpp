/*
 * MIT License
 * 
 * Copyright (c) 2024 g2wfw
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */


#include "logger_manager.h"
#include "jni_provider.h"
#include "common.h"
#include "memory_manager.h"
#include <spdlog/sinks/android_sink.h>
#include <spdlog/sinks/sink.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/async.h>
#include <libgen.h>
#include <jni.h>
#include <cstdint>


#define LOG_TAG "QBDI"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

static uint64_t get_timestamp_ms() {
    struct timeval tv;
    gettimeofday(&tv, nullptr);

    return (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
}

template<typename Container>
static inline std::string join(const Container &v, const char *delim) {
    if (v.empty()) {
        return "[]";
    }
    std::ostringstream os;
    os << "[";
    std::copy(v.begin(), std::prev(v.end()),
              std::ostream_iterator<typename Container::value_type>(os, delim));
    os << *(v.rbegin());
    os << "]";
    return os.str();
}

static std::string get_files_dir(JNIEnv *env) {
    jclass activityThreadClass = env->FindClass("android/app/ActivityThread");
    jmethodID currentActivityThreadMethod = env->GetStaticMethodID(activityThreadClass,
                                                                   "currentActivityThread",
                                                                   "()Landroid/app/ActivityThread;");
    jobject activityThread = env->CallStaticObjectMethod(activityThreadClass,
                                                         currentActivityThreadMethod);

    jmethodID getApplicationMethod = env->GetMethodID(activityThreadClass,
                                                      "getApplication",
                                                      "()Landroid/app/Application;");
    jobject context = env->CallObjectMethod(activityThread, getApplicationMethod);


    jclass contextClass = env->FindClass("android/content/Context");
    jmethodID getFilesDirMethod = env->GetMethodID(contextClass, "getFilesDir", "()Ljava/io/File;");
    jobject filesDir = env->CallObjectMethod(context, getFilesDirMethod);

    jclass fileClass = env->FindClass("java/io/File");
    jmethodID getAbsolutePathMethod = env->GetMethodID(fileClass, "getAbsolutePath",
                                                       "()Ljava/lang/String;");
    jstring absPath = (jstring) env->CallObjectMethod(filesDir, getAbsolutePathMethod);

    const char *absPathChars = env->GetStringUTFChars(absPath, 0);
    std::string result(absPathChars);
    env->ReleaseStringUTFChars(absPath, absPathChars);

    return result;
}

void LoggerManager::set_enable_to_logcat(bool enable) {
    if (enable && this->logcat != nullptr) {
        return;
    }
    if (!enable && this->logcat != nullptr) {
        this->logcat.reset();
        return;
    }
    if (enable) {
        this->logcat = spdlog::android_logger_mt("logcat_itrace", "qbdi");
        logcat->set_pattern("[%H:%M:%S.%e] %v");
    }
}

void LoggerManager::set_enable_to_file(bool enable) {
    if (enable) {
        auto env = smjni::jni_provider::get_jni();
        auto file_dir = get_files_dir(env);
        std::string trace_log_dir = file_dir + "/itrace/";
        if (!check_and_mkdir(trace_log_dir)) {
            LOGE("mkdir failed %s", trace_log_dir.c_str());
            return;
        }
        if (trace_log_base.empty()) {
            trace_log_base = fmt::format("{}{}_{:x}_{:x}/", trace_log_dir,
                                         basename(this->module_name.c_str()), module_range.base,
                                         get_timestamp_ms());
            if (!check_and_mkdir(trace_log_base)) {
                LOGE("mkdir failed %s", trace_log_base.c_str());
            }
        }
        if (this->file_log == nullptr) {
            this->file_log = spdlog::basic_logger_mt("itracer", trace_log_base + "itrace.txt",
                                                     false);
            file_log->set_pattern("[%H:%M:%S.%e] %v");
        }
    } else {
        if (this->file_log != nullptr) {
            this->file_log.reset();
        }
    }
}

void LoggerManager::set_memory_dump_to_file(bool dump) {
    if (dump) {
        auto env = smjni::jni_provider::get_jni();
        auto file_dir = get_files_dir(env);
        std::string trace_log_dir = file_dir + "/itrace/";
        if (!check_and_mkdir(trace_log_dir)) {
            LOGE("mkdir failed %s", trace_log_dir.c_str());
            return;
        }
        if (trace_log_base.empty()) {
            trace_log_base = fmt::format("{}{}_{:x}_{:x}/", trace_log_dir,
                                         basename(this->module_name.c_str()), module_range.base,
                                         get_timestamp_ms());
            if (!check_and_mkdir(trace_log_base)) {
                LOGE("mkdir failed %s", trace_log_base.c_str());
            }
        }
        if (memory_manager == nullptr) {
            memory_manager = std::make_unique<MemoryManager>();
        }
        memory_manager->set_dump_path(trace_log_base + "/memory_dump.txt");
    } else {
        if (this->memory_manager != nullptr) {
            this->memory_manager.reset();
        }
    }
}

void LoggerManager::flush() {
    if (this->memory_manager != nullptr) {
        this->memory_manager->clear();
    }
}

bool LoggerManager::check_and_mkdir(std::string &path) {
    struct stat info;
    if (stat(path.c_str(), &info) == 0) {
        if (info.st_mode & S_IFDIR) {
            return true;
        } else {
            return false;
        }
    }
    if (mkdir(path.c_str(), 0755) == 0) {
        return true;
    } else {
        return false;
    }
    return true;
}

static inline std::string ltrim(const std::string &str) {
    size_t start = str.find_first_not_of(" \t\n\r\f\v");
    return (start == std::string::npos) ? "" : str.substr(start);
}

static inline std::string rtrim(const std::string &str) {
    size_t end = str.find_last_not_of(" \t\n\r\f\v");
    return (end == std::string::npos) ? "" : str.substr(0, end + 1);
}

static inline std::string trim(const std::string &str) {
    return rtrim(ltrim(str));
}

void LoggerManager::write_trace_info(const inst_trace_info_t *info,
                                     const QBDI::InstAnalysis *instAnalysis,
                                     std::vector<QBDI::MemoryAccess> &memoryAccesses) const {
    if (this->logcat == nullptr && this->file_log == nullptr) {
        return;
    }
    if (info->fun_call != nullptr && !info->fun_call->fun_name.empty()) {
        if (info->fun_call->memory_free_address != 0) {
            memory_manager->remove_memory(info->fun_call->memory_free_address);
        }
        if (info->fun_call->memory_alloc_size != 0) {
            memory_manager->add_memory(info->fun_call->memory_alloc_address,
                                       info->fun_call->memory_alloc_size);
        }
    }
    std::string line = (fmt::format("|{:#x}", info->pc));
    //[00:31:57.995]|0x76a5af6488|0x13214c| lsl w15, w15, #3|[W15= 0x8 ==> 0x40]
    line.append(fmt::format("|{:#x}|", info->pc - module_range.base));
    std::string dis_str = instAnalysis->disassembly;

    dis_str = trim(dis_str);
    line.append(dis_str);
    std::string reg_info;
    format_register_info(reg_info, info, instAnalysis);
    line.append("|");
    if (!reg_info.empty()) {
        line.append(reg_info);
    }
    line.append("|");
    std::string memory_access_info;
    format_access_info(memory_access_info, memoryAccesses);;
    if (!memory_access_info.empty()) {
        line.append(memory_access_info);
    }
    std::string call_info;
    format_call_info(call_info, info, instAnalysis);
    line.append("|");
    if (!call_info.empty()) {
        line.append(call_info);
    }

    write_info(line);
}


void LoggerManager::format_access_info(std::string &result,
                                       std::vector<QBDI::MemoryAccess> &memoryAccesses) const {
    if (memoryAccesses.empty()) {
        return;
    }
    std::vector<std::string> ma_info;
    for (const auto &ma: memoryAccesses) {
        if (is_address_in_module_range(ma.accessAddress)) {
            if (ma.type == QBDI::MemoryAccessType::MEMORY_READ) {
                ma_info.push_back(
                        fmt::format("read module offset:{:#x} size:{:#x} => {:#x}",
                                    ma.accessAddress - module_range.base,
                                    ma.size, ma.value));
            } else {
                ma_info.push_back(
                        fmt::format("write module offset:{:#x} size:{:#x} => {:#x} ",
                                    ma.accessAddress - module_range.base,
                                    ma.size, ma.value));
            }
        } else {
            auto [offset, memory_index] = this->memory_manager->get_memory_offset(ma.accessAddress);
            if (ma.type == QBDI::MemoryAccessType::MEMORY_READ) {
                ma_info.push_back(
                        fmt::format(
                                "read memory:{:#x}=>{:#x} memory block index:{:#x} size:{:#x} offset:{:#x}",
                                ma.accessAddress, ma.value, memory_index,
                                ma.size, offset));
            } else {
                ma_info.push_back(
                        fmt::format(
                                "write memory:{:#x}=>{:#x} memory block index:{:#x} size:{:#x} offset:{:#x}",
                                ma.accessAddress, ma.value, memory_index,
                                ma.size, offset));

            }
        }
    }

    if (ma_info.empty()) {
        return;
    }
    result = join(ma_info, ",");
}

void LoggerManager::write_info(std::string &line) const {
    if (this->logcat != nullptr) {
        this->logcat->info(line);
    }
    if (this->file_log != nullptr) {
        this->file_log->info(line);
    }
}

void LoggerManager::format_register_info(std::string &result, const inst_trace_info_t *info,
                                         const QBDI::InstAnalysis *inst) {
    //[],read:[]
    std::vector<std::string> cur_regs_vector;
    std::vector<std::string> read_regs_vector;
    auto &post_fpr_state = info->post_status.fpr_state;
    auto &pre_fpr_state = info->pre_status.fpr_state;
    auto &post_gpr_state = info->post_status.gpr_state;
    auto &pre_gpr_state = info->pre_status.gpr_state;
    for (int i = 0; i < inst->numOperands; ++i) {
        auto operand = inst->operands[i];
        if (operand.regAccess == QBDI::RegisterAccessType::REGISTER_READ
            || operand.regAccess == QBDI::RegisterAccessType::REGISTER_READ_WRITE
            || operand.regAccess == QBDI::RegisterAccessType::REGISTER_WRITE
                ) {
            if (operand.regName == nullptr) {
                continue;
            }
            if (operand.regCtxIdx < 0) {
                continue;
            }
#ifdef __arm__

            if (operand.type == QBDI::OPERAND_FPR) {
                switch (operand.size) {
                    case 4: {
                        if (operand.regAccess == QBDI::REGISTER_READ ||
                            operand.regAccess == QBDI::REGISTER_READ_WRITE) {
                            read_regs_vector.emplace_back(
                                fmt::format("{}= {:.2a}", operand.regName,
                                            pre_fpr_state.vreg.s[operand.regCtxIdx]));
                        }
                        cur_regs_vector.emplace_back(
                            fmt::format("{}= {:.2a}", operand.regName,
                                        post_fpr_state.vreg.s[operand.regCtxIdx]));
                    }
                    break;
                    case 8: {
                        if (operand.regAccess == QBDI::REGISTER_READ ||
                            operand.regAccess == QBDI::REGISTER_READ_WRITE) {
                            read_regs_vector.emplace_back(
                                fmt::format("{}= {:.2a}", operand.regName,
                                            pre_fpr_state.vreg.d[operand.regCtxIdx]));
                        }
                        cur_regs_vector.emplace_back(
                            fmt::format("{}= {:.2a}", operand.regName,
                                        post_fpr_state.vreg.d[operand.regCtxIdx]));
                    }
                    break;
                    case 16: {
                        //todo 128bit num read on arm32
                        if (operand.regAccess == QBDI::REGISTER_READ ||
                            operand.regAccess == QBDI::REGISTER_READ_WRITE) {
                            cur_regs_vector.emplace_back(
                                fmt::format("{}= {:#x}", operand.regName,
                                            (uint64_t)pre_fpr_state.vreg.q[operand.regCtxIdx]));
                        }
                        cur_regs_vector.emplace_back(
                            fmt::format("{}= {:#x}", operand.regName,
                                        (uint64_t)post_fpr_state.vreg.q[operand.regCtxIdx]));
                    }
                    break;
                    default:
                        LOGE("fail to read %s %hx", operand.regName, operand.regCtxIdx);
                        break;
                }
            } else if (operand.type == QBDI::OPERAND_GPR) {
                cur_regs_vector.emplace_back(
                    fmt::format("{}= {:#x}", operand.regName,
                                QBDI_GPR_GET(&post_gpr_state, operand.regCtxIdx)));
            }
#else
#define QBDI_FPR_GET_B(state, i) (reinterpret_cast<const uint8_t *>(state)[i])
#define QBDI_FPR_GET_H(state, i) (reinterpret_cast<const uint16_t *>(state)[i])
#define QBDI_FPR_GET_S(state, i) (reinterpret_cast<const uint32_t *>(state)[i])
#define QBDI_FPR_GET_D(state, i) (reinterpret_cast<const uint64_t *>(state)[i])
#define QBDI_FPR_GET_V(state, i) (reinterpret_cast<const __uint128_t *>(state)[i])
            if (operand.type == QBDI::OPERAND_FPR) {
                if (operand.regName[0] == 'B') {
                    if (operand.regAccess == QBDI::REGISTER_READ ||
                        operand.regAccess == QBDI::REGISTER_READ_WRITE) {
                        uint8_t value = QBDI_FPR_GET_B(&pre_fpr_state, operand.regCtxIdx);
                        read_regs_vector.emplace_back(
                                fmt::format("{}= {:#x}", operand.regName, value));
                    }
                    //LOGI("arm reg b:%s 0x%x", operand.regName, operand.regCtxIdx);
                    uint8_t value = QBDI_FPR_GET_B(&post_fpr_state, operand.regCtxIdx);
                    cur_regs_vector.emplace_back(
                            fmt::format("{}= {:#x}", operand.regName, value));

                } else if (operand.regName[0] == 'H') {
                    if (operand.regAccess == QBDI::REGISTER_READ ||
                        operand.regAccess == QBDI::REGISTER_READ_WRITE) {
                        uint16_t value = QBDI_FPR_GET_H(&pre_fpr_state, operand.regCtxIdx / 2);
                        read_regs_vector.emplace_back(
                                fmt::format("{}= {:#x}", operand.regName, value));
                    }
                    //LOGI("arm reg h:%s 0x%x", operand.regName, operand.regCtxIdx / 2);
                    uint16_t value = QBDI_FPR_GET_H(&post_fpr_state, operand.regCtxIdx / 2);
                    cur_regs_vector.emplace_back(
                            fmt::format("{}= {:#x}", operand.regName, value));
                } else if (operand.regName[0] == 'S') {
                    //LOGI("arm reg s:%s 0x%x", operand.regName, operand.regCtxIdx / 4);
                    if (operand.regAccess == QBDI::REGISTER_READ ||
                        operand.regAccess == QBDI::REGISTER_READ_WRITE) {
                        auto value = QBDI_FPR_GET_S(&pre_fpr_state, operand.regCtxIdx / 4);
                        read_regs_vector.emplace_back(
                                fmt::format("{}= {:#x}", operand.regName, value));
                    }
                    uint32_t value = QBDI_FPR_GET_S(&post_fpr_state, operand.regCtxIdx / 4);
                    cur_regs_vector.emplace_back(
                            fmt::format("{}= {:#x}", operand.regName, value));
                } else if (operand.regName[0] == 'D') {
                    //LOGI("arm reg d:%s 0x%x", operand.regName, operand.regCtxIdx / 8);
                    if (operand.regAccess == QBDI::REGISTER_READ ||
                        operand.regAccess == QBDI::REGISTER_READ_WRITE) {
                        uint64_t value = QBDI_FPR_GET_D(&pre_fpr_state, operand.regCtxIdx / 8);
                        read_regs_vector.emplace_back(
                                fmt::format("{}= {:#x}", operand.regName, value));
                    }
                    uint64_t value = QBDI_FPR_GET_D(&post_fpr_state, operand.regCtxIdx / 8);
                    cur_regs_vector.emplace_back(
                            fmt::format("{}= {:#x}", operand.regName, value));
                } else {
                    if (operand.regAccess == QBDI::REGISTER_READ ||
                        operand.regAccess == QBDI::REGISTER_READ_WRITE) {
                        read_regs_vector.emplace_back(
                                fmt::format("{}= {:#x}", operand.regName,
                                            QBDI_FPR_GET_V(&pre_fpr_state,
                                                           operand.regCtxIdx / 16)));
                    }
                    cur_regs_vector.emplace_back(
                            fmt::format("{}= {:#x}", operand.regName,
                                        QBDI_FPR_GET_V(&post_fpr_state, operand.regCtxIdx / 16)));
                }
            } else if (operand.type == QBDI::OPERAND_GPR) {
                if (operand.regAccess == QBDI::REGISTER_READ ||
                    operand.regAccess == QBDI::REGISTER_READ_WRITE) {
                    read_regs_vector.emplace_back(
                            fmt::format("{}= {:#x}", operand.regName,
                                        QBDI_GPR_GET(&pre_gpr_state, operand.regCtxIdx)));
                }
                cur_regs_vector.emplace_back(
                        fmt::format("{}= {:#x}", operand.regName,
                                    QBDI_GPR_GET(&post_gpr_state, operand.regCtxIdx)));
            }
#endif
        }
    }
    if (!cur_regs_vector.empty()) {
        result.append(join(cur_regs_vector, ","));
    }
    if (!read_regs_vector.empty()) {
        result.append(",");
        result.append(join(read_regs_vector, ","));
    }
    if (result.empty()) {
        result = "";
    }
}

void LoggerManager::format_call_info(std::string &result, const inst_trace_info_t *info,
                                     const QBDI::InstAnalysis *instAnalysis) {
    if (info->fun_call == nullptr) {
        result = " ";
        return;
    }
    auto call = info->fun_call;
    // lib_name:fun_name args ret
    result.append(call->call_module_name);
    result.append(":");
    result.append(call->fun_name);
    result.append(" args:");
    result.append(join(call->args, ","));
    result.append(" ");
    result.append(call->ret_value);
}

LoggerManager::~LoggerManager() {
    if (this->memory_manager != nullptr) {
        this->memory_manager->clear();
    }
}