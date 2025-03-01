include(${CMAKE_CURRENT_LIST_DIR}/git_utils.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/common.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/vcpkg_utils.cmake)

set(vcpkg_path ${CMAKE_CURRENT_LIST_DIR}/vcpkg)
set(root_path ${CMAKE_CURRENT_LIST_DIR})
set(target_abi_list armeabi-v7a arm64-v8a x86 x86_64 "armeabi-v7a with NEON")
set(toolchain_name_list gcc clang)
set(stl_type_list c++_static c++_shared none system)

# 设置ndk目录信息
function(_set_ndk_home _ndk_home)
    if (IS_DIRECTORY ${_ndk_home})
        if (EXISTS ${_ndk_home}/ndk-build.cmd OR EXISTS ${ndk_home}/ndk-build)
            set(ANDROID_NDK_HOME ${_ndk_home} CACHE STRING "for ndk home path")
            set_property(GLOBAL PROPERTY ANDROID_NDK_HOME ${_ndk_home})
            message("-- set NDK_HOME to: ${ANDROID_NDK_HOME}")
        endif ()
    else ()
        message(FATAL_ERROR "-- NDK_HOME:${_ndk_home} not a directory")
    endif ()
endfunction()


function(_set_vcpkg_android_toolchain_file toolchain_file)
    if (IS_DIRECTORY ${toolchain_file})
        if (EXISTS ${toolchain_file}/build/cmake/android.toolchain.cmake)
            set(VCPKG_CHAINLOAD_TOOLCHAIN_FILE ${toolchain_file}/build/cmake/android.toolchain.cmake CACHE STRING "for android.toolchain.cmake path")
            message("-- set CMAKE_TOOLCHAIN_FILE to:${CMAKE_TOOLCHAIN_FILE}")
            return()
        endif ()
    endif ()
    if (EXISTS ${toolchain_file})
        get_filename_component(toolchain_file_name ${toolchain_file} NAME)
        if (${toolchain_file_name} STREQUAL "android.toolchain.cmake")
            set(VCPKG_CHAINLOAD_TOOLCHAIN_FILE ${toolchain_file}/build/cmake/android.toolchain.cmake CACHE STRING "for android.toolchain.cmake path")
            message("-- set CMAKE_TOOLCHAIN_FILE to:${CMAKE_TOOLCHAIN_FILE}")
            return()
        endif ()
    endif ()
    message(WARNING "toolchain_file:${toolchain_file} is not android toolchain file,try find ndk home by env")
    find_ndk_path(android_ndk_home)
    if (EXISTS ${android_ndk_home}/build/cmake/android.toolchain.cmake)
        set(VCPKG_CHAINLOAD_TOOLCHAIN_FILE ${toolchain_file}/build/cmake/android.toolchain.cmake CACHE STRING "for android.toolchain.cmake path")
        message("-- set CMAKE_TOOLCHAIN_FILE to:${CMAKE_TOOLCHAIN_FILE}")
        return()
    endif ()
    message(FATAL_ERROR "-- can not find android.toolchain.cmake file,please set [toolchain_file] to NDK home or android.toolchain.cmake")
endfunction()

function(_set_vcpkg_toolchain_file ndk_home)
    if (NOT EXISTS ${vcpkg_path}/scripts/buildsystems/vcpkg.cmake)
        message(FATAL_ERROR "-- ${vcpkg_path}/scripts/buildsystems/vcpkg.cmake not exists")
    endif ()
    set(CMAKE_TOOLCHAIN_FILE ${vcpkg_path}/scripts/buildsystems/vcpkg.cmake CACHE STRING "for vcpkg.cmake path")
    #for ndk
    _set_ndk_home(${ndk_home})
    _set_vcpkg_android_toolchain_file(${ndk_home})
endfunction()

#[[
初始化NDK_HOME
]]
function(init_vcpkg_android_ndk ndk_home)
    check_or_download_vcpkg()
    _set_vcpkg_toolchain_file(${ndk_home})
    set(ENV{VCPKG_ROOT} ${vcpkg_path})
    message(STATUS "set VCPKG_ROOT to ${vcpkg_path}")

endfunction()

#初始化需要构建的目标信息，
#[[
target_abi: 查看target_abi_list 值信息
api_level：android api等级
toolchain_name:编译器名称 gcc or clang
stl_type: 标准库名称
]]
function(init_vcpkg_android_build target_abi api_level toolchain_name stl_type)
    #设置构建目标系统为Android
    set(CMAKE_SYSTEM_NAME "Android" CACHE STRING "for CMAKE_SYSTEM_NAME value")
    message(STATUS "set CMAKE_SYSTEM_NAME to Android")
    #判断目标架构是否在符合支持架构
    if (NOT ${target_abi} IN_LIST target_abi_list)
        message(FATAL_ERROR "-- unsupported target abi:${target_abi},target_abi must in ${target_abi_list}")
    endif ()
    #设置目标架构
    set(ANDROID_ABI ${target_abi} CACHE STRING "for ANDROID_ABI value")

    #编译器名称 正常情况下是clang,gcc 在某些版本已经废弃
    if (NOT ${toolchain_name} IN_LIST toolchain_name_list)
        message(FATAL_ERROR "-- unsupported toolchain name:${toolchain_name},toolchain name must in ${toolchain_name_list}")
    endif ()
    #设置编译器名称
    set(CMAKE_ANDROID_NDK_TOOLCHAIN_VERSION ${toolchain_name} CACHE STRING "for CMAKE_ANDROID_NDK_TOOLCHAIN_VERSION value")
    # 判断标准库的名称
    if (NOT ${stl_type} IN_LIST stl_type_list)
        message(FATAL_ERROR "-- unsupported stl type :${stl_type},stl type must in ${stl_type_list}")
    endif ()
    #设置标准库
    set(CMAKE_ANDROID_STL_TYPE ${stl_type} CACHE STRING "for CMAKE_ANDROID_STL_TYPE value")
    #设置编译等级
    set(ANDROID_NATIVE_API_LEVEL ${api_level} CACHE STRING "for ANDROID_NATIVE_API_LEVEL value")

    #设置VCPKG_TARGET_TRIPLET为android 相关数据，否则将默认为系统架构，导致无法查找目标信息
    if (ANDROID_ABI STREQUAL "armeabi-v7a with NEON" OR ${target_abi} STREQUAL armeabi-v7a)
        set(VCPKG_TARGET_TRIPLET arm-android CACHE STRING "for VCPKG_TARGET_TRIPLET value")
    elseif (${target_abi} STREQUAL arm64-v8a)
        set(VCPKG_TARGET_TRIPLET arm64-android CACHE STRING "for VCPKG_TARGET_TRIPLET value")
    elseif (${target_abi} STREQUAL x86)
        set(VCPKG_TARGET_TRIPLET x86-android CACHE STRING "for VCPKG_TARGET_TRIPLET value")
    elseif (${target_abi} STREQUAL x86_64)
        set(VCPKG_TARGET_TRIPLET x64-android CACHE STRING "for VCPKG_TARGET_TRIPLET value")
    else ()
        message(FATAL_ERROR "Invalid Android ABI: ${target_abi}.")
    endif ()
    message(STATUS "set VCPKG_TARGET_TRIPLET to:${VCPKG_TARGET_TRIPLET}")

endfunction()


#[[
此处使用macro 而不是使用function 因为function变量设置存在问题
]]
macro(config_vcpkg_toolchain_file)
    if (ANDROID_ABI STREQUAL armeabi-v7a OR ANDROID_ABI STREQUAL "armeabi-v7a with NEON")
        set(ANDROID_COMPILER_PREFIX armv7a-linux-androideabi${ANDROID_NATIVE_API_LEVEL})
    elseif (ANDROID_ABI STREQUAL arm64-v8a)
        set(ANDROID_COMPILER_PREFIX aarch64-linux-android${ANDROID_NATIVE_API_LEVEL})
    elseif (ANDROID_ABI STREQUAL x86)
        #i686-linux-android23
        set(ANDROID_COMPILER_PREFIX i686-linux-android${ANDROID_NATIVE_API_LEVEL})
    elseif (ANDROID_ABI STREQUAL x86_64)
        #x86_64-linux-android22
        set(ANDROID_COMPILER_PREFIX x86_64-linux-android${ANDROID_NATIVE_API_LEVEL})
    else ()
        message(FATAL_ERROR "Invalid Android ABI: ${ANDROID_ABI}")
    endif ()
    message(STATUS "ANDROID_COMPILER_PREFIX:${ANDROID_COMPILER_PREFIX}")
    #对于 ndk 版本小于19的  直接使用clang 导致编译参数存在问题，如果小于默认加上相关参数
    if (ANDROID_NDK_MAJOR VERSION_LESS 19)
        set(CMAKE_CXX_FLAGS "--target=${ANDROID_COMPILER_PREFIX} ${CMAKE_CXX_FLAGS}")
    else ()
        #不同系统上的 android ndk 存在形式
        if (${CMAKE_HOST_SYSTEM} MATCHES Windows)
            set(CMAKE_C_COMPILER ${ANDROID_TOOLCHAIN_ROOT}/bin/${ANDROID_COMPILER_PREFIX}-clang.cmd)
            message(STATUS "set CMAKE_C_COMPILER:${CMAKE_C_COMPILER}")
            set(CMAKE_CXX_COMPILER ${ANDROID_TOOLCHAIN_ROOT}/bin/${ANDROID_COMPILER_PREFIX}-clang++.cmd)
            message(STATUS "set CMAKE_CXX_COMPILER:${CMAKE_CXX_COMPILER}")
        else ()
            set(CMAKE_C_COMPILER ${ANDROID_TOOLCHAIN_ROOT}/bin/${ANDROID_COMPILER_PREFIX}-clang)
            message(STATUS "set CMAKE_C_COMPILER:${CMAKE_C_COMPILER}")
            set(CMAKE_CXX_COMPILER ${ANDROID_TOOLCHAIN_ROOT}/bin/${ANDROID_COMPILER_PREFIX}-clang++)
            message(STATUS "set CMAKE_CXX_COMPILER:${CMAKE_CXX_COMPILER}")
        endif ()
    endif ()
endmacro()