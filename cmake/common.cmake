
macro(dump_all_variables)
    message(STATUS "print_all_variables------------------------------------------{")
    get_cmake_property(_variableNames VARIABLES)
    foreach (_variableName ${_variableNames})
        message(STATUS "${_variableName}=${${_variableName}}")
    endforeach ()
    message(STATUS "print_all_variables------------------------------------------}")
endmacro()

macro(find_ndk_path android_ndk_home)
    #dump_all_variables()
    if (CMAKE_HOST_WIN32)
        find_program(ANDROID_NDK_BUILD_PROGRAM ndk-build.cmd)
    else ()
        find_program(ANDROID_NDK_BUILD_PROGRAM ndk-build)
    endif ()
    if (ANDROID_NDK_BUILD_PROGRAM)
        get_filename_component(android_ndk_home "${ANDROID_NDK_BUILD_PROGRAM}" DIRECTORY)
    elseif (DEFINED ENV{NDK_HOME})
        set(android_ndk_home ENV{NDK_HOME})
    else ()
        message(WARNING "Cannot find 'ndk home', make sure you installed the NDK and added it to your PATH")
    endif ()
endmacro()

# returns true if only a single one of its arguments is true
function(xor result)
    set(true_args_count 0)

    foreach (foo ${ARGN})
        if (foo)
            math(EXPR true_args_count "${true_args_count}+1")
        endif ()
    endforeach ()

    if (NOT (${true_args_count} EQUAL 1))
        set(${result} FALSE PARENT_SCOPE)
    else ()
        set(${result} TRUE PARENT_SCOPE)
    endif ()
endfunction()

function(at_most_one result)
    set(true_args_count 0)

    foreach (foo ${ARGN})
        if (foo)
            math(EXPR true_args_count "${true_args_count}+1")
        endif ()
    endforeach ()

    if (${true_args_count} GREATER 1)
        set(${result} FALSE PARENT_SCOPE)
    else ()
        set(${result} TRUE PARENT_SCOPE)
    endif ()
endfunction()

macro(enable_release_build)
    string(TOLOWER ${CMAKE_BUILD_TYPE} build_type)
    if (NOT ${build_type} STREQUAL "minsizerel")
        message(WARNING "CMAKE_BUILD_TYPE is : ${CMAKE_BUILD_TYPE}, Will set to Release")
        set(CMAKE_BUILD_TYPE "Release")
    endif ()
    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
        # using Clang
        # using GCC
        message(STATUS "Compiler is GCC or Clang")
        cmake_policy(SET CMP0065 NEW)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ffunction-sections -fdata-sections -s")
        set(CMAKE_CXX_LINK_FLAGS "${CMAKE_CXX_LINK_FLAGS} -Wl,--gc-sections")
    elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
        # using Intel C++
        message(FATAL_ERROR "Compiler is Intel not Support")
    elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
        # using Visual Studio C++
        message(STATUS "Compiler is MSVC")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  /Gy")
        set(CMAKE_CXX_LINK_FLAGS "${CMAKE_CXX_LINK_FLAGS} /OPT:REF")
    endif ()
endmacro()


macro(enforce_enable_release_build)
    string(TOLOWER ${CMAKE_BUILD_TYPE} build_type)
    if (NOT ${build_type} STREQUAL "minsizerel")
        message(WARNING "CMAKE_BUILD_TYPE is : ${CMAKE_BUILD_TYPE}, Will set to Release")
        set(CMAKE_BUILD_TYPE "Release")
    endif ()
    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
        # using Clang
        # using GCC
        message(STATUS "Compiler is GCC or Clang")
        cmake_policy(SET CMP0065 NEW)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ffunction-sections -fdata-sections -s")
        set(CMAKE_CXX_LINK_FLAGS "${CMAKE_CXX_LINK_FLAGS} -Wl,--gc-sections")
    elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
        # using Intel C++
        message(FATAL_ERROR "Compiler is Intel not Support")
    elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
        message(STATUS "Compiler is MSVC")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  /Gy")
        set(CMAKE_CXX_LINK_FLAGS "${CMAKE_CXX_LINK_FLAGS} /OPT:REF")
    endif ()
endmacro()






