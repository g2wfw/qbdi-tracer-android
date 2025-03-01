cmake_minimum_required(VERSION 3.22)
if (NOT DEFINED CMAKE_ANDROID_NDK)
    if (EXISTS ${CMAKE_CURRENT_LIST_DIR}/cmake)
        include(cmake/android.cmake)
        if (WIN32)
            init_android_ndk("D:/Android/sdk/ndk/27.0.12077973")
        else ()
            init_android_ndk("/mnt/d/ndk-r16b")
        endif ()
    else ()
        message(FATAL_ERROR "can not find cmake tools dir,please clone cmake tools")
    endif ()
    # target_abi must in [armeabi-v7a、arm64-v8a、x86、x86_64]
    # toolchain_name must in [gcc、clang]
    # stl_type must in [c++_static、c++_shared、none、system]
    if (DEFINED target_abi)
        init_android_build(${target_abi} 21 "clang" "c++_static")
    else ()
        init_android_build("armeabi-v7a" 21 "clang" "c++_static")
    endif ()
endif ()

project(qbdi_tracer)
if (NOT DEFINED CMAKE_ANDROID_NDK)
    config_toolchain_file()
endif ()
set(CMAKE_CXX_STANDARD 17)


include(FetchContent)
if (ANDROID_ABI STREQUAL "arm64-v8a")
    FetchContent_Declare(
            qdbi
            URL "file://${CMAKE_CURRENT_LIST_DIR}/prebuild/arm64/QBDI-0.11.1.arm64.zip"  #
    )
    FetchContent_Declare(
            gum
            URL "file://${CMAKE_CURRENT_LIST_DIR}/prebuild/arm64/frida-gum-devkit-16.6.6-android-arm64.tar.xz"
    )

elseif (ANDROID_ABI STREQUAL "armeabi-v7a")
    FetchContent_Declare(
            qdbi
            URL "file://${CMAKE_CURRENT_LIST_DIR}/prebuild/arm/QBDI-0.11.1.arm.zip"
    )
    FetchContent_Declare(
            gum
            URL "file://${CMAKE_CURRENT_LIST_DIR}/prebuild/arm/frida-gum-devkit-16.6.6-android-arm.tar.xz"
    )
endif ()
FetchContent_MakeAvailable(qdbi)
FetchContent_MakeAvailable(gum)
set(QBDI_LIB_PATH "${qdbi_SOURCE_DIR}/lib/libQBDI.a")
set(QBDI_INCLUDE_DIR "${qdbi_SOURCE_DIR}/include")

set(FRIDA_GUM_LIB_PATH "${gum_SOURCE_DIR}/libfrida-gum.a")
set(FRIDA_GUM_INCLUDE_DIR "${gum_SOURCE_DIR}/")
include_directories(${QBDI_INCLUDE_DIR} ${FRIDA_GUM_INCLUDE_DIR} ./)
add_subdirectory(external)
add_library(qbdi_tracer SHARED
        core/instruction_tracer_manager.cpp
        core/instruction_tracer_manager.h
        core/instruction_call_back.cpp
        core/common.h
        core/instruction_register_utils.cpp
        core/instruction_register_utils.h
        core/instruction_info_manager.cpp
        core/instruction_info_manager.h
        core/instruction_dispatch_manager.cpp
        core/instruction_dispatch_manager.h
        core/dispatch/dispatch_base.cpp
        core/dispatch/dispatch_jni_env.cpp
        core/dispatch/dispatch_libc.cpp
        core/dispatch/jni_internal.cpp
        core/logger_manager.cpp
        core/logger_manager.h
        test/trace_test.cpp
        core/dispatch/dispatch_syscall.cpp
        core/dispatch/dispatch_libz.cpp
        core/dispatch/dispatch_libz.h
)

target_link_libraries(qbdi_tracer PRIVATE ${QBDI_LIB_PATH} ${FRIDA_GUM_LIB_PATH} smjni log)