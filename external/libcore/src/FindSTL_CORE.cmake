# FindSTLCore
# --------
#
# Find the stl libraries
#
# Result Variables
# ^^^^^^^^^^^^^^^^
#
# The following variables will be defined:
#
# ``STL_CORE_FOUND`` True if Logger found on the local system
#
# ``STL_CORE_INCLUDE_DIRS`` Location of Logger header files
#
# ``STL_CORE_LIBRARY_DIRS`` Location of Logger libraries
#
# ``STL_CORE_LIBRARIES`` List of the Logger libraries found
#

include(${CMAKE_ROOT}/Modules/FindPackageHandleStandardArgs.cmake)
include(${CMAKE_ROOT}/Modules/SelectLibraryConfigurations.cmake)
include(${CMAKE_ROOT}/Modules/CMakeFindDependencyMacro.cmake)


# 获取当前FindCore.cmake文件所在的路径
get_filename_component(CORE_MODULE_DIR "${CMAKE_CURRENT_LIST_DIR}" DIRECTORY)
set(STL_CORE_INCLUDE_DIRS ${CORE_MODULE_DIR}/include/ CACHE STRING "")
if (WIN32)
    set(__CMAKE_CORE_LIB_PATH "${CORE_MODULE_DIR}/lib" CACHE INTERNAL "Local variable" FORCE)
    set(__CMAKE_CORE_LIB_SUFFIX .dll CACHE INTERNAL "Local variable" FORCE)
elseif (APPLE)
    set(__CMAKE_CORE_LIB_PATH "${CORE_MODULE_DIR}/lib" CACHE INTERNAL "Local variable" FORCE)
    set(__CMAKE_CORE_LIB_SUFFIX .dylib CACHE INTERNAL "Local variable" FORCE)
elseif (UNIX)
    set(__CMAKE_CORE_LIB_PATH "${CORE_MODULE_DIR}/lib" CACHE INTERNAL "Local variable" FORCE)
    set(__CMAKE_CORE_LIB_SUFFIX .so CACHE INTERNAL "Local variable" FORCE)
else ()
    message(FATAL_ERROR "Unsupported operating system")
endif ()
if (NOT DEFINED BUILD_SHARED_LIBS OR NOT BUILD_SHARED_LIBS)
    set(__CMAKE_CORE_LIB_SUFFIX .a CACHE INTERNAL "Local variable" FORCE)
endif ()

if (NOT EXISTS ${__CMAKE_CORE_LIB_PATH}/libcore${__CMAKE_CORE_LIB_SUFFIX})
    message(WARNING "core static library file not exists")
    set(STL_CORE_FOUND OFF CACHE BOOL "")
else ()
    set(STL_CORE_FOUND ON CACHE BOOL "")
endif ()
if (WIN32)
    set(STL_CORE_LIBRARIES ${__CMAKE_CORE_LIB_PATH}/libcore${__CMAKE_CORE_LIB_SUFFIX} shlwapi version winmm CACHE STRING "")
elseif (APPLE)
    set(STL_CORE_LIBRARIES ${__CMAKE_CORE_LIB_PATH}/libcore${__CMAKE_CORE_LIB_SUFFIX} "-framework Foundation -framework AppKit -framework IOKit -framework Security" CACHE STRING "")
elseif (UNIX)
    set(STL_CORE_LIBRARIES ${__CMAKE_CORE_LIB_PATH}/libcore${__CMAKE_CORE_LIB_SUFFIX} libstdc++${__CMAKE_CORE_LIB_SUFFIX} pthread dl CACHE STRING "")
else ()
    message(FATAL_ERROR "Unsupported operating system")
endif ()
find_package_handle_standard_args(STL_CORE REQUIRED_VARS STL_CORE_INCLUDE_DIRS STL_CORE_LIBRARIES STL_CORE_FOUND)
mark_as_advanced(STL_CORE_INCLUDE_DIRS STL_CORE_LIBRARIES STL_CORE_FOUND)

