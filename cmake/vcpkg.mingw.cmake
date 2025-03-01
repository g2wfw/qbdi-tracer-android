#[[
此cmake 文件主要是针对，mingw的vcpkg 相关设置
]]
include(${CMAKE_CURRENT_LIST_DIR}/git_utils.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/common.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/vcpkg_utils.cmake)
function(_set_vcpkg_toolchain_file)
    if (NOT EXISTS ${vcpkg_path}/scripts/buildsystems/vcpkg.cmake)
        message(FATAL_ERROR "-- ${vcpkg_path}/scripts/buildsystems/vcpkg.cmake not exists")
    endif ()
    set(VCPKG_TARGET_TRIPLET x64-mingw-static CACHE STRING "for VCPKG_TARGET_TRIPLET value")
    set(CMAKE_TOOLCHAIN_FILE ${vcpkg_path}/scripts/buildsystems/vcpkg.cmake CACHE STRING "for vcpkg.cmake path")

endfunction()


function(init_vcpkg_mingw)
    check_or_download_vcpkg()
    _set_vcpkg_toolchain_file()
    set(ENV{VCPKG_ROOT} ${vcpkg_path})
    message(STATUS "set VCPKG_ROOT to ${vcpkg_path}")
endfunction()

