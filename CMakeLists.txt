cmake_minimum_required(VERSION 3.22)
cmake_policy(SET CMP0135 NEW)
set(CMAKE_BUILD_TYPE Debug)
if (NOT DEFINED CMAKE_ANDROID_NDK)
    if (EXISTS ${CMAKE_CURRENT_LIST_DIR}/cmake)
        include(cmake/android.cmake)
        if (EXISTS /mnt/d/ndk/android-ndk-r25c)
            init_android_ndk(/mnt/d/ndk/android-ndk-r25c)
        elseif (EXISTS /mnt/d/linux-ndk/android-ndk-r25c)
            init_android_ndk(/mnt/d/linux-ndk/android-ndk-r25c)
        endif ()
    else ()
        message(FATAL_ERROR "can not find cmake tools dir,please clone cmake tools")
    endif ()
    # target_abi must in [armeabi-v7a、arm64-v8a、x86、x86_64]
    # toolchain_name must in [gcc、clang]
    # stl_type must in [c++_static、c++_shared、none、system]
    if (DEFINED target_abi)
        init_android_build(${target_abi} 24 "clang" "c++_static")
    else ()
        init_android_build("armeabi-v7a" 24 "clang" "c++_static")
    endif ()
endif ()
project(reverse_tools)
if (NOT DEFINED CMAKE_ANDROID_NDK)
    config_toolchain_file()
endif ()
include(FetchContent)
if (ANDROID_ABI STREQUAL "arm64-v8a")
    FetchContent_Declare(
            qdbi
            URL "file://${CMAKE_CURRENT_LIST_DIR}/prebuild/arm64/QBDI-0.11.1.arm64.zip"  #
    )
elseif (ANDROID_ABI STREQUAL "armeabi-v7a")
    FetchContent_Declare(
            qdbi
            URL "file://${CMAKE_CURRENT_LIST_DIR}/prebuild/arm/QBDI-0.11.1.arm.zip"
    )
endif ()
FetchContent_MakeAvailable(qdbi)

set(QBDI_LIB_PATH "${qdbi_SOURCE_DIR}/lib/libQBDI.a")
set(QBDI_INCLUDE_DIR "${qdbi_SOURCE_DIR}/include")


include_directories(${QBDI_INCLUDE_DIR} ./)


set(CMAKE_CXX_STANDARD 17)
add_subdirectory(external)
include_directories(./)
set(core_source
        core/memory.cpp
        core/library.cpp
        core/linker.cpp
        core/linker.h
        core/hook_manager.cpp
        core/back_trace_manager.cpp
        core/back_trace_manager.h
        core/memory_scan.cpp
        core/memory_scan.h
        core/java_helper.cpp
        core/java_helper.h
)

set(trace_src
        trace/instruction_tracer_manager.cpp
        trace/instruction_tracer_manager.h
        trace/instruction_call_back.cpp
        trace/common.h
        trace/instruction_register_utils.cpp
        trace/instruction_register_utils.h
        trace/instruction_info_manager.cpp
        trace/instruction_info_manager.h
        trace/instruction_dispatch_manager.cpp
        trace/instruction_dispatch_manager.h
        trace/dispatch/dispatch_base.cpp
        trace/dispatch/dispatch_jni_env.cpp
        trace/dispatch/dispatch_libc.cpp
        trace/dispatch/jni_internal.cpp
        trace/logger_manager.cpp
        trace/logger_manager.h
        trace/dispatch/dispatch_syscall.cpp
        trace/dispatch/dispatch_libz.cpp
        trace/dispatch/dispatch_libz.h
        trace/memory_manager.cpp
        trace/memory_manager.h
        trace/hex_dump.cpp
        trace/hex_dump.h
)

add_library(qbdi-tracer SHARED ${core_source} ${hook_src} ${trace_src})
target_link_libraries(qbdi-tracer PUBLIC ${QBDI_LIB_PATH} stl::core smjni dobby_static xdl xHook log breaktrace)


add_library(qbdi-tracer-static STATIC ${core_source} ${hook_src} ${trace_src})
target_link_libraries(qbdi-tracer-static PUBLIC ${QBDI_LIB_PATH} stl::core smjni dobby_static xdl xHook log breaktrace)


add_subdirectory(examples)