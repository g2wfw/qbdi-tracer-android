
include_directories(./include ./src)
set(break_trace_src
        BacktraceImpl.cpp
        BacktraceMap.cpp
        BacktraceThread.cpp
        thread_utils.c
        UnwindCurrent.cpp
        UnwindMap.cpp
        UnwindPtrace.cpp
)


set(libunwind_src_files

        src/mi/init.c
        src/mi/flush_cache.c
        src/mi/mempool.c
        src/mi/strerror.c
        src/mi/backtrace.c
        src/mi/dyn-cancel.c
        src/mi/dyn-info-list.c
        src/mi/dyn-register.c
        src/mi/map.c
        src/mi/Lmap.c
        src/mi/Ldyn-extract.c
        src/mi/Lfind_dynamic_proc_info.c
        src/mi/Lget_proc_info_by_ip.c
        src/mi/Lget_proc_name.c
        src/mi/Lput_dynamic_unwind_info.c
        src/mi/Ldestroy_addr_space.c
        src/mi/Lget_reg.c
        src/mi/Lset_reg.c
        src/mi/Lget_fpreg.c
        src/mi/Lset_fpreg.c
        src/mi/Lset_caching_policy.c
        src/mi/Gdyn-extract.c
        src/mi/Gdyn-remote.c
        src/mi/Gfind_dynamic_proc_info.c
        src/mi/Gget_accessors.c
        src/mi/Gget_proc_info_by_ip.c
        src/mi/Gget_proc_name.c
        src/mi/Gput_dynamic_unwind_info.c
        src/mi/Gdestroy_addr_space.c
        src/mi/Gget_reg.c
        src/mi/Gset_reg.c
        src/mi/Gget_fpreg.c
        src/mi/Gset_fpreg.c
        src/mi/Gset_caching_policy.c
        src/dwarf/Lexpr.c
        src/dwarf/Lfde.c
        src/dwarf/Lparser.c
        src/dwarf/Lpe.c
        src/dwarf/Lstep_dwarf.c
        src/dwarf/Lfind_proc_info-lsb.c
        src/dwarf/Lfind_unwind_table.c
        src/dwarf/Gexpr.c
        src/dwarf/Gfde.c
        src/dwarf/Gfind_proc_info-lsb.c
        src/dwarf/Gfind_unwind_table.c
        src/dwarf/Gparser.c
        src/dwarf/Gpe.c
        src/dwarf/Gstep_dwarf.c
        src/dwarf/global.c
        src/os-common.c
        src/os-linux.c
        src/Los-common.c
)


add_definitions(-DHAVE_CONFIG_H
        -DDEBUG
        -D_GNU_SOURCE)

if (${CMAKE_SYSTEM_PROCESSOR} MATCHES i686)
    include_directories(./include/tdep-x86)
    file(GLOB UNWIND_SOURCES_ASM
            src/x86/getcontext-linux.S
            src/x86/Gos-linux.c
            src/x86/Los-linux.c
            debuggerd/x86/machine.cpp
            debuggerd/x86/crashglue.S
    )

    set(libunwind_src_files_${CMAKE_SYSTEM_PROCESSOR}
            src/x86/is_fpreg.c
            src/x86/regname.c
            src/x86/Gcreate_addr_space.c
            src/x86/Gget_proc_info.c
            src/x86/Gget_save_loc.c
            src/x86/Gglobal.c
            src/x86/Ginit.c
            src/x86/Ginit_local.c
            src/x86/Ginit_remote.c
            src/x86/Gregs.c
            src/x86/Gresume.c
            src/x86/Gstep.c
            src/x86/Lcreate_addr_space.c
            src/x86/Lget_proc_info.c
            src/x86/Lget_save_loc.c
            src/x86/Lglobal.c
            src/x86/Linit.c
            src/x86/Linit_local.c
            src/x86/Linit_remote.c
            src/x86/Lregs.c
            src/x86/Lresume.c
            src/x86/Lstep.c
    )
elseif (${CMAKE_SYSTEM_PROCESSOR} MATCHES x86_64)
    include_directories(./include/tdep-x86_64)
    file(GLOB UNWIND_SOURCES_ASM
            src/x86_64/getcontext.S
            src/x86_64/Gstash_frame.c
            src/x86_64/Gtrace.c
            src/x86_64/Gos-linux.c
            src/x86_64/Lstash_frame.c
            src/x86_64/Ltrace.c
            src/x86_64/Los-linux.c
            src/x86_64/setcontext.S
            debuggerd/x86_64/machine.cpp
            debuggerd/x86_64/crashglue.S
    )


    set(libunwind_src_files_${CMAKE_SYSTEM_PROCESSOR}
            src/x86_64/is_fpreg.c
            src/x86_64/regname.c
            src/x86_64/Gcreate_addr_space.c
            src/x86_64/Gget_proc_info.c
            src/x86_64/Gget_save_loc.c
            src/x86_64/Gglobal.c
            src/x86_64/Ginit.c
            src/x86_64/Ginit_local.c
            src/x86_64/Ginit_remote.c
            src/x86_64/Gregs.c
            src/x86_64/Gresume.c
            src/x86_64/Gstep.c
            src/x86_64/Lcreate_addr_space.c
            src/x86_64/Lget_proc_info.c
            src/x86_64/Lget_save_loc.c
            src/x86_64/Lglobal.c
            src/x86_64/Linit.c
            src/x86_64/Linit_local.c
            src/x86_64/Linit_remote.c
            src/x86_64/Lregs.c
            src/x86_64/Lresume.c
            src/x86_64/Lstep.c
    )
elseif (${CMAKE_SYSTEM_PROCESSOR} MATCHES arm)
    include_directories(./include/tdep-arm)
    file(GLOB UNWIND_SOURCES_ASM
            src/arm/getcontext.S
            src/arm/Gis_signal_frame.c
            src/arm/Gex_tables.c
            src/arm/Lis_signal_frame.c
            src/arm/Lex_tables.c
            debuggerd/arm/machine.cpp
            debuggerd/arm/crashglue.S
    )

    set(libunwind_src_files_${CMAKE_SYSTEM_PROCESSOR}
            src/arm/is_fpreg.c
            src/arm/regname.c
            src/arm/Gcreate_addr_space.c
            src/arm/Gget_proc_info.c
            src/arm/Gget_save_loc.c
            src/arm/Gglobal.c
            src/arm/Ginit.c
            src/arm/Ginit_local.c
            src/arm/Ginit_remote.c
            src/arm/Gregs.c
            src/arm/Gresume.c
            src/arm/Gstep.c
            src/arm/Lcreate_addr_space.c
            src/arm/Lget_proc_info.c
            src/arm/Lget_save_loc.c
            src/arm/Lglobal.c
            src/arm/Linit.c
            src/arm/Linit_local.c
            src/arm/Linit_remote.c
            src/arm/Lregs.c
            src/arm/Lresume.c
            src/arm/Lstep.c
    )
elseif (${CMAKE_SYSTEM_PROCESSOR} MATCHES aarch64)
    include_directories(./include/tdep-aarch64)

    file(GLOB UNWIND_SOURCES_ASM
            src/aarch64/siglongjmp.S
            src/aarch64/Gis_signal_frame.c
            src/aarch64/Lis_signal_frame.c
            debuggerd/arm64/machine.cpp
            debuggerd/arm64/crashglue.S
    )

    set(libunwind_src_files_${CMAKE_SYSTEM_PROCESSOR}
            src/aarch64/is_fpreg.c
            src/aarch64/regname.c
            src/aarch64/Gcreate_addr_space.c
            src/aarch64/Gget_proc_info.c
            src/aarch64/Gget_save_loc.c
            src/aarch64/Gglobal.c
            src/aarch64/Ginit.c
            src/aarch64/Ginit_local.c
            src/aarch64/Ginit_remote.c
            src/aarch64/Gregs.c
            src/aarch64/Gresume.c
            src/aarch64/Gstep.c
            src/aarch64/Lcreate_addr_space.c
            src/aarch64/Lget_proc_info.c
            src/aarch64/Lget_save_loc.c
            src/aarch64/Lglobal.c
            src/aarch64/Linit.c
            src/aarch64/Linit_local.c
            src/aarch64/Linit_remote.c
            src/aarch64/Lregs.c
            src/aarch64/Lresume.c
            src/aarch64/Lstep.c
    )
else ()
    message(FATAL_ERROR "Unsupported architecture: ${CMAKE_SYSTEM_PROCESSOR}")
endif ()


set(libunwind-ptrace_src_files
        src/ptrace/_UPT_elf.c
        src/ptrace/_UPT_accessors.c
        src/ptrace/_UPT_access_fpreg.c
        src/ptrace/_UPT_access_mem.c
        src/ptrace/_UPT_access_reg.c
        src/ptrace/_UPT_create.c
        src/ptrace/_UPT_destroy.c
        src/ptrace/_UPT_find_proc_info.c
        src/ptrace/_UPT_get_dyn_info_list_addr.c
        src/ptrace/_UPT_put_unwind_info.c
        src/ptrace/_UPT_get_proc_name.c
        src/ptrace/_UPT_reg_offset.c
        src/ptrace/_UPT_resume.c

)

set(debugggerd_src
        debuggerd/backtrace.cpp
        debuggerd/utility.cpp
        debuggerd/debugger.cpp
)
enable_language(C ASM)
add_library(breaktrace STATIC
        ${break_trace_src}
        ${UNWIND_SOURCES_ASM}
        ${libunwind_src_files}
        ${libunwind_src_files_${CMAKE_SYSTEM_PROCESSOR}}
        ${libunwind-ptrace_src_files}
        ${debugggerd_src}
)
target_link_libraries(breaktrace log)
target_include_directories(breaktrace PUBLIC include ./)