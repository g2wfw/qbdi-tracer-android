#[[
用于自动安装各种模块
]]
include(${CMAKE_CURRENT_LIST_DIR}/git_utils.cmake)
set(target_abi_list armeabi-v7a arm64-v8a x86 x86_64 "armeabi-v7a with NEON")
set(vcpkg_path ${CMAKE_CURRENT_LIST_DIR}/vcpkg)
set(root_path ${CMAKE_CURRENT_LIST_DIR})
# 安装vcpkg android 模块
function(install_android_package package_name target_abi)
    if (NOT ${target_abi} IN_LIST target_abi_list)
        message(FATAL_ERROR "-- unsupported target abi:${target_abi},target_abi must in ${target_abi_list}")
    endif ()
    if (${target_abi} STREQUAL "armeabi-v7a with NEON" OR ${target_abi} STREQUAL armeabi-v7a)
        set(vcpkg_target_triplet arm-android)
    elseif (${target_abi} STREQUAL arm64-v8a)
        set(vcpkg_target_triplet arm64-android)
    elseif (${target_abi} STREQUAL x86)
        set(vcpkg_target_triplet x86-android)
    elseif (${target_abi} STREQUAL x86_64)
        set(vcpkg_target_triplet x64-android)
    else ()
        message(FATAL_ERROR "Invalid Android ABI: ${target_abi}.")
    endif ()
    message(STATUS "set vcpkg_target_triplet to: ${vcpkg_target_triplet}")
    get_property(ndk_home GLOBAL PROPERTY ANDROID_NDK_HOME)
    message(STATUS "get NDK_HOME: ${ndk_home}")
    if (NOT EXISTS ${ndk_home})
        message(FATAL_ERROR "Invalid NDK HOME: ${ndk_home}.")
    endif ()
    message(STATUS "NDK HOME: ${ndk_home}")
    set(ENV{ANDROID_NDK_HOME} ${ndk_home})
    #检测vcpkg 是否安装
    check_or_download_vcpkg()
    if (CMAKE_HOST_WIN32)
        set(vcpkg_exe_path, ${vcpkg_path}/vcpkg.exe)
    else ()
        set(vcpkg_exe_path, ${vcpkg_path}/vcpkg)
    endif ()

    if (NOT EXISTS ${vcpkg_exe_path})
        message(FATAL_ERROR "vcpkg execute not exists:${vcpkg_exe_path}")
    endif ()
    #执行安装命令
    execute_process(
            COMMAND "${vcpkg_exe_path} install ${package_name}:${vcpkg_target_triplet}"
            WORKING_DIRECTORY ${vcpkg_path}
            RESULT_VARIABLE install_result
            OUTPUT_VARIABLE install_output
    )
    if (NOT install_result EQUAL "0")
        message(FATAL_ERROR "-- running ${vcpkg_exe_path} install ${package_name}:${vcpkg_target_triplet} fail with:${install_output}")
    endif ()

endfunction()





#[[
安装host架构版本的库
]]
function(install_package package_name triplet)
    #检测vcpkg 是否安装
    check_or_download_vcpkg()
    if (CMAKE_HOST_WIN32)
        set(vcpkg_exe_path ${vcpkg_path}/vcpkg.exe)
    else ()
        set(vcpkg_exe_path ${vcpkg_path}/vcpkg)
    endif ()
    message(${vcpkg_exe_path})
    if (NOT EXISTS ${vcpkg_exe_path})
        message(FATAL_ERROR "vcpkg execute not exists:${vcpkg_exe_path}")
    endif ()
    #执行安装命令
    execute_process(
            COMMAND "${vcpkg_exe_path} install ${package_name}:${triplet}"
            WORKING_DIRECTORY ${vcpkg_path}
            RESULT_VARIABLE install_result
            OUTPUT_VARIABLE install_output
    )
    if (NOT install_result EQUAL "0")
        message(FATAL_ERROR "-- running ${vcpkg_exe_path} install ${package_name} fail with:${install_output}")
    endif ()
endfunction()


#[[
为当前环境上下文设置代理
]]
function(set_proxy_env host port)
    set(ENV{HTTP_PROXY} "http://${host}:${port}")
    set(ENV{HTTPS_PROXY} "http://${host}:${port}")
    message(STATUS "set proxy to : http://${host}:${port}")
endfunction()


function(check_or_download_vcpkg)

    if (NOT EXISTS ${vcpkg_path})
        message(${vcpkg_path})
        git_clone(
                PROJECT_NAME vcpkg
                GIT_URL https://github.com/microsoft/vcpkg.git
                GIT_TAG "2021.05.12"
                DIRECTORY ${root_path}
                CLONE_RESULT_VARIABLE clone_result
        )
        if (NOT ${clone_result})
            message(FATAL_ERROR "-- running git clone https://github.com/microsoft/vcpkg.git fail")
        endif ()
    endif ()
    # for vcpkg executable file
    # download
    if (CMAKE_HOST_WIN32)
        if (NOT EXISTS ${vcpkg_path}/vcpkg.exe)
            message(STATUS "start running ${vcpkg_path}/bootstrap-vcpkg.bat")
            execute_process(
                    COMMAND ${vcpkg_path}/bootstrap-vcpkg.bat
                    WORKING_DIRECTORY ${vcpkg_path}
                    RESULT_VARIABLE bootstrap_result
                    OUTPUT_VARIABLE bootstrap_out
            )
            if (NOT bootstrap_result EQUAL "0")
                message(FATAL_ERROR "-- running ${vcpkg_path}/bootstrap-vcpkg.bat fail with:${bootstrap_out}")
            endif ()
        endif ()
    else ()
        if (NOT EXISTS ${vcpkg_path}/vcpkg)
            message(STATUS "start running ${vcpkg_path}/bootstrap-vcpkg")
            execute_process(
                    COMMAND "chmod +x ${vcpkg_path}/bootstrap-vcpkg.sh"
                    WORKING_DIRECTORY ${vcpkg_path}
                    RESULT_VARIABLE bootstrap_result
                    OUTPUT_VARIABLE bootstrap_out
            )
            execute_process(
                    COMMAND ${vcpkg_path}/bootstrap-vcpkg.sh
                    WORKING_DIRECTORY ${vcpkg_path}
                    RESULT_VARIABLE bootstrap_result
                    OUTPUT_VARIABLE bootstrap_out
            )
            if (NOT bootstrap_result EQUAL "0")
                message(FATAL_ERROR "-- running ${vcpkg_path}/bootstrap-vcpkg.sh fail with:${bootstrap_out}")
            endif ()
        endif ()
    endif ()
endfunction()

