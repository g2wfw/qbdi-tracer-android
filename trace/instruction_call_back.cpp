#include <android/log.h>
#include <dlfcn.h>
#include <jni_provider.h>
#include <dobby.h>
#include "instruction_tracer_manager.h"
#include "instruction_call_back.h"
#include "instruction_register_utils.h"

#define LOG_TAG "QBDI"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

QBDI::VMAction pre_instruction_call(QBDI::VM *vm, QBDI::GPRState *gprState,
                                    QBDI::FPRState *fprState, void *data) {
    const QBDI::InstAnalysis *inst = vm->getInstAnalysis(
            QBDI::AnalysisType::ANALYSIS_INSTRUCTION | QBDI::AnalysisType::ANALYSIS_DISASSEMBLY
            | QBDI::AnalysisType::ANALYSIS_OPERANDS);
    if (inst == nullptr) {
        return QBDI::VMAction::CONTINUE;
    }
    auto self = (InstructionTracerManager *) data;
    if (self == nullptr) {
        LOGE("callback data is nullptr in pre call");
        return QBDI::VMAction::STOP;
    }
    if (!self->is_need_record(gprState->pc)) {
        return QBDI::VMAction::CONTINUE;
    }


    auto &info_manger = self->get_info_manager();
    if (info_manger == nullptr) {
        LOGE("info_manger is nullptr in pre call");
        return QBDI::VMAction::STOP;
    }
    auto info = info_manger->alloc_inst_trace_info(gprState->pc);
    uint64_t pc = info->pc;
    auto offset = pc - self->getModuleRange().base;
    self->trace_callback_pre(offset, gprState, fprState);

    memcpy(&info->pre_status.gpr_state, gprState, sizeof(QBDI::GPRState));
    if (fprState != nullptr) {
        memcpy(&info->pre_status.fpr_state, fprState, sizeof(QBDI::FPRState));
    }
    //check fun call
    if ((inst->isBranch || inst->isCall) && inst->affectControlFlow) {
        info_manger->alloc_fun_call(gprState->pc);
        info->inst_analysis = inst;
    }

    return QBDI::VMAction::CONTINUE;
}


QBDI::VMAction post_instruction_call(QBDI::VM *vm, QBDI::GPRState *gprState,
                                     QBDI::FPRState *fprState, void *data) {
    auto self = (InstructionTracerManager *) data;
    if (self == nullptr) {
        LOGE("callback data is nullptr in post call");
        return QBDI::VMAction::STOP;
    }
    const QBDI::InstAnalysis *inst = vm->getInstAnalysis(
            QBDI::AnalysisType::ANALYSIS_INSTRUCTION | QBDI::AnalysisType::ANALYSIS_DISASSEMBLY
            | QBDI::AnalysisType::ANALYSIS_OPERANDS);
    if (inst == nullptr) {
        return QBDI::VMAction::CONTINUE;
    }
    if (!self->is_need_record(inst->address)) {
        return QBDI::VMAction::CONTINUE;
    }
    auto &info_manger = self->get_info_manager();
    if (info_manger == nullptr) {
        LOGE("info_manger is nullptr in post call");
        return QBDI::VMAction::STOP;
    }
    auto current_info = info_manger->get_current_inst_trace_info();
    //check address
    if (current_info == nullptr) {
        LOGE("info is nullptr in post call %p", (void *) inst->address);
        return QBDI::VMAction::STOP;
    }
    if (current_info->pc != inst->address) {
        LOGE("info pc != inst->address in post call %p", (void *) inst->address);
        return QBDI::VMAction::STOP;
    }
    uint64_t pc = current_info->pc;
    auto offset = pc - self->getModuleRange().base;
    self->trace_callback_post(offset, gprState, fprState);
    memcpy(&current_info->post_status.gpr_state, gprState, sizeof(QBDI::GPRState));
    if (fprState != nullptr) {
        memcpy(&current_info->post_status.fpr_state, fprState, sizeof(QBDI::FPRState));
    }
    if (current_info->fun_call != nullptr) {
        if (self->is_address_in_module_range(gprState->pc) && !current_info->fun_call->is_svc) {
            info_manger->dispatch_fun_call_common_args(gprState->pc);
        } else {
            info_manger->dispatch_fun_call_args(gprState->pc);
        }
    }
    //check pre fun call is nullptr
    auto prev = info_manger->get_previous_inst_trace_info();
    //set ret value
    if (prev != nullptr && prev->fun_call != nullptr) {
        if (self->is_address_in_module_range(prev->post_status.gpr_state.pc) &&
            !prev->fun_call->is_svc) {
            info_manger->dispatch_fun_call_common_return(gprState);
        } else {
            info_manger->dispatch_fun_call_return(gprState);
        }
        std::vector<QBDI::MemoryAccess> empty(0);
        info_manger->write_trace_info(prev->inst_analysis, empty);
    }
    //check cur_inst is call
    if (current_info->fun_call == nullptr) {
        //write trace info
        auto access_list = vm->getInstMemoryAccess();
        if (!access_list.empty()) {
            access_list.erase(std::remove_if(access_list.begin(), access_list.end(),
                                             [&](const QBDI::MemoryAccess &ma) {
                                                 return self->is_address_in_stack_range(
                                                         ma.accessAddress);
                                             }), access_list.end());
        }
        info_manger->write_trace_info(inst, access_list);
    }
    auto access_list = vm->getInstMemoryAccess();
    if (access_list.empty()) {
        return QBDI::CONTINUE;
    }
    return QBDI::CONTINUE;
}

QBDI::VMAction
on_fun_call(QBDI::VMInstanceRef vm, const QBDI::VMState *state, QBDI::GPRState *gprState,
            QBDI::FPRState *fprState, void *data) {
    /*  auto self = (InstructionTracerManager *) data;
      if (self == nullptr) {
          LOGE("callback data is nullptr in pre call");
          return QBDI::VMAction::STOP;
      }
      if (!self->is_need_record(gprState->pc)) {
          return QBDI::VMAction::CONTINUE;
      }
      auto &info_manger = self->get_info_manager();
      if (info_manger == nullptr) {
          LOGE("info_manger is nullptr in on_fun_call");
          return QBDI::VMAction::STOP;
      }*/
    return QBDI::CONTINUE;
}

QBDI::VMAction
on_memory_read_or_write(QBDI::VMInstanceRef vm, QBDI::GPRState *gprState,
                        QBDI::FPRState *fprState,
                        void *data) {
    return QBDI::CONTINUE;
}

static long long get_timestamp() {
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    return (long long) tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

static void dump_time_diff(long long start, long long end) {
    long long diff = end - start; //
    long long minutes = diff / (60 * 1000);
    long long seconds = (diff % (60 * 1000)) / 1000;
    long long milliseconds = diff % 1000;
    LOGI("%02lld:%02lld:%03lld", minutes, seconds, milliseconds);
}

void inst_at_enter(void *address, void *x, void *user_data) {
    DobbyRegisterContext *ctx = static_cast<DobbyRegisterContext *>(x);
    auto *self = (InstructionTracerManager *) user_data;
#ifdef __arm__
    if (!self->check_attach_cond((uintptr_t) address, 8, ctx->general.r, nullptr)) {
        return;
    }
#else
    if (!self->check_attach_cond((uintptr_t) address, 8, ctx->general.x,
                                 (uintptr_t *) ctx->floating.q)) {
        return;
    }
#endif
    auto vm = self->get_qbdi_vm();
    auto status = vm->getGPRState();
    auto fstatus = vm->getFPRState();
    {
        //DobbyDestroy(address);
        InstructionRegisterUtils::doby_to_qbdi(ctx, status);
        InstructionRegisterUtils::doby_to_qbdi(ctx, fstatus);
        vm->setGPRState(status);
        QBDI::rword ret_value;
        vm->addCodeCB(QBDI::InstPosition::PREINST, pre_instruction_call, user_data);
        vm->addCodeCB(QBDI::InstPosition::POSTINST, post_instruction_call, user_data);
        vm->addVMEventCB(QBDI::VMEvent::EXEC_TRANSFER_CALL, on_fun_call, user_data);
        vm->recordMemoryAccess(QBDI::MEMORY_READ_WRITE);
        auto orig = DobbyGetOrigFunc(address);
        vm->addInstrumentedModuleFromAddr(reinterpret_cast<QBDI::rword>(address));
        vm->addInstrumentedRange((QBDI::rword) orig, (QBDI::rword) orig + 0x256);
        auto start = get_timestamp();
        auto result = vm->call(&ret_value, (QBDI::rword) orig, {});
        self->get_info_manager()->flush();
        if (!result) {
            LOGE("run fail");
        }
        auto end = get_timestamp();
        dump_time_diff(start, end);
    }
}

QBDI::VMAction
pre_svc_instruction_call(QBDI::VM *vm, QBDI::GPRState *gprState, QBDI::FPRState *fprState,
                         void *data) {
    const auto self = static_cast<InstructionTracerManager *>(data);
    if (self == nullptr) {
        LOGE("callback data is nullptr in pre svc");
        return QBDI::VMAction::STOP;
    }
    auto &info_manger = self->get_info_manager();
    if (info_manger == nullptr) {
        LOGE("info_manger is nullptr in post call");
        return QBDI::VMAction::STOP;
    }
    const QBDI::InstAnalysis *inst = vm->getInstAnalysis(
            QBDI::AnalysisType::ANALYSIS_INSTRUCTION | QBDI::AnalysisType::ANALYSIS_DISASSEMBLY
            | QBDI::AnalysisType::ANALYSIS_OPERANDS);

    auto current_info = info_manger->get_current_inst_trace_info();
    if (current_info->fun_call == nullptr) {
        info_manger->alloc_fun_call(gprState->pc);
        current_info->inst_analysis = inst;
        current_info->fun_call->is_svc = true;
    }
    return QBDI::CONTINUE;
}