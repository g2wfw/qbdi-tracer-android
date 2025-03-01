include(ExternalData)

macro(config_output_build)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/lib/)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/lib/)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/bin/)
endmacro()


# 封装为 CMake 函数，通过 install(CODE) 调用
function(install_target_with_deps TARGET)
    # 注意 CMake 3.x 以上版本支持的 install(CODE) 需要正确写法
    if (NOT CMAKE_SYSTEM_NAME STREQUAL "Windows")
        set_target_properties(${TARGET} PROPERTIES INSTALL_RPATH "$ORIGIN/lib")
    elseif (MINGW)
        install(TARGETS ${TARGET} DESTINATION ${CMAKE_INSTALL_PREFIX})
        find_file(win_thread_dll NAMES libwinpthread-1.dll)
        if (win_thread_dll)
            file(COPY "${win_thread_dll}" DESTINATION ${CMAKE_INSTALL_PREFIX}
                    FOLLOW_SYMLINK_CHAIN
            )
            message(STATUS "Found win_thread_dll: ${win_thread_dll}")
        endif ()
        find_file(win_gcc_dll NAMES libgcc_s_seh-1.dll)
        if (win_gcc_dll)
            file(COPY "${win_gcc_dll}" DESTINATION ${CMAKE_INSTALL_PREFIX}
                    FOLLOW_SYMLINK_CHAIN
            )
            message(STATUS "Found win_gcc_dll: ${win_gcc_dll}")
        endif ()
        get_target_property(LINK_LIBS ${TARGET} LINK_LIBRARIES)
        message(STATUS "Target <target> depends on: ${LINK_LIBS}")
        foreach (item IN LISTS LINK_LIBS)
            if (IS_DIRECTORY ${item})
                continue()
            endif ()
            if (EXISTS ${item})
                file(COPY "${item}" DESTINATION ${CMAKE_INSTALL_PREFIX}
                        FOLLOW_SYMLINK_CHAIN
                )
                message(STATUS "Found : ${item}")
            else ()
                message(STATUS "Not found: ${item}")
            endif ()
        endforeach ()
        return()
    else ()
        set(install_code [[file(GET_RUNTIME_DEPENDENCIES EXECUTABLES
        ]])
        string(APPEND install_code $<TARGET_FILE:${TARGET}>)
        string(APPEND install_code [[

                RESOLVED_DEPENDENCIES_VAR RESOLVED_DEPS
                UNRESOLVED_DEPENDENCIES_VAR UNRESOLVED_DEPS
                DIRECTORIES
        ]])
        # for cmake_moudle_path

        foreach (item IN LISTS CMAKE_MODULE_PATH)
            #if end with cmake
            if (item MATCHES ".*cmake$")
                list(APPEND CMAKE_MODULE_PATH ${item})
                #获取 cmake所在的目录
                get_filename_component(item_parent ${item} DIRECTORY)
                if (EXISTS ${item_parent}/lib)
                    string(APPEND install_code ${item_parent}/lib)
                    string(APPEND install_code " ")
                    message(STATUS "search path: ${item_parent}/lib")
                endif ()
            endif ()
        endforeach ()
        string(APPEND install_code [[
              PRE_EXCLUDE_REGEXES ".*windows.*" ".*system32.*" ".*gcc_s_seh.*" ".*winpthread.*"
              POST_EXCLUDE_REGEXES ".*windows.*" ".*system32.*" ".*gcc_s_seh.*" ".*winpthread.*"
                )
        foreach(FILE ${RESOLVED_DEPS})
            file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}" TYPE SHARED_LIBRARY FILES "${FILE}")
        endforeach()
        foreach(FILE ${UNRESOLVED_DEPS})
            message(STATUS "Unresolved dependency: ${FILE}")
        endforeach()
        ]])
        install(CODE ${install_code})
        find_file(win_thread_dll NAMES libwinpthread-1.dll)
        if (win_thread_dll)
            file(COPY "${win_thread_dll}" DESTINATION ${CMAKE_INSTALL_PREFIX}
                    FOLLOW_SYMLINK_CHAIN
            )
            message(STATUS "Found win_thread_dll: ${win_thread_dll}")
        endif ()
        find_file(win_gcc_dll NAMES libgcc_s_seh-1.dll)
        if (win_gcc_dll)
            file(COPY "${win_gcc_dll}" DESTINATION ${CMAKE_INSTALL_PREFIX}
                    FOLLOW_SYMLINK_CHAIN
            )
            message(STATUS "Found win_gcc_dll: ${win_gcc_dll}")
        endif ()
        return()
    endif ()


    install(TARGETS ${TARGET}
            RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}
    )
    set(install_code
            [[
       function(install_library_with_deps LIBRARY)
            message(STATUS "Checking library :${LIBRARY}")
            list(APPEND SYSTEM_LIB_PATHS "/usr/lib" "/usr/local/lib" "/lib64/" "/usr/lib64" "C:/Windows/system32")
            foreach(SYSTEM_PATH ${SYSTEM_LIB_PATHS})
                string(FIND "${LIBRARY}" "${SYSTEM_PATH}" IS_SYSTEM_LIB)
                if (IS_SYSTEM_LIB GREATER -1)
                    message(STATUS "Skipping system library: ${LIBRARY}")
                    return()
                endif()
            endforeach()

            if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
                set(INSTALL_DEST "${CMAKE_INSTALL_PREFIX}")
            else()
                set(INSTALL_DEST "${CMAKE_INSTALL_PREFIX}/lib")
            endif()
            message(STATUS "Installing library: ${LIBRARY}")
            if (NOT EXISTS ${LIBRARY})
                return()
            endif ()

            string(TOLOWER "${LIBRARY}" LIBRARY_LOWER)


            if (NOT LIBRARY_LOWER MATCHES "\\.(so|dll)$")
                message(STATUS "The file ${LIBRARY} is not a dynamic library (.so or .dll). Returning.")
                return()
            endif()


            file(INSTALL
                DESTINATION "${INSTALL_DEST}"
                TYPE SHARED_LIBRARY
                FOLLOW_SYMLINK_CHAIN
                FILES "${LIBRARY}"
            )


            file(GET_RUNTIME_DEPENDENCIES
                LIBRARIES ${LIBRARY}
                RESOLVED_DEPENDENCIES_VAR RESOLVED_DEPS
                UNRESOLVED_DEPENDENCIES_VAR UNRESOLVED_DEPS
            )


            foreach(FILE ${RESOLVED_DEPS})
                if(NOT IS_SYMLINK ${FILE})
                    install_library_with_deps(${FILE})
                endif()
            endforeach()


            foreach(FILE ${UNRESOLVED_DEPS})
                message(STATUS "Unresolved from ${LIBRARY}: ${FILE}")
            endforeach()
        endfunction()


        file(GET_RUNTIME_DEPENDENCIES
            EXECUTABLES

    ]]
    )
    string(APPEND install_code $<TARGET_FILE:${TARGET}>)
    string(APPEND install_code [[
             RESOLVED_DEPENDENCIES_VAR RESOLVED_DEPS
            UNRESOLVED_DEPENDENCIES_VAR UNRESOLVED_DEPS
        )

        message(STATUS "Installing RESOLVED_DEPS:${RESOLVED_DEPS}")
        foreach(FILE ${RESOLVED_DEPS})
            install_library_with_deps(${FILE})
        endforeach()


        foreach(FILE ${UNRESOLVED_DEPS})
            message(STATUS "Unresolved: ${FILE}")
        endforeach()
    ]])
    install(CODE ${install_code})
endfunction()


macro(add_prebuild project_url name)
    message(STATUS "CMAKE_SYSTEM_NAME ${CMAKE_SYSTEM_NAME}")

    if (CMAKE_SYSTEM_NAME STREQUAL "Windows" AND CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(${name}_archive_name ${name}_win_amd64.tar.gz)

    elseif (CMAKE_SYSTEM_NAME STREQUAL "Linux")
        if (CMAKE_C_COMPILER MATCHES ".*aarch64.*")
            set(${name}_archive_name ${name}_linux_arm64.tar.gz)
        elseif (CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
            set(${name}_archive_name ${name}_linux_amd64.tar.gz)
        elseif (CMAKE_SYSTEM_PROCESSOR STREQUAL loongarch64)
            set(${name}_archive_name ${name}_linux_loong64.tar.gz)
        elseif (CMAKE_SYSTEM_PROCESSOR STREQUAL riscv64)
            set(${name}_archive_name ${name}_linux_riscv64.tar.gz)
        else ()
            message(FATAL_ERROR "Unknown architecture: ${CMAKE_SYSTEM_PROCESSOR}")
        endif ()
    else ()
        message(FATAL_ERROR "Unknown architecture: ${CMAKE_SYSTEM_PROCESSOR}")
    endif ()

    set(${name}_version_url ${project_url}/${name}.version)
    set(${name}_version_file ${CMAKE_CURRENT_BINARY_DIR}/${name}.version)
    set(${name}_version_file_temp ${CMAKE_CURRENT_BINARY_DIR}/${name}.version.temp)
    file(DOWNLOAD ${${name}_version_url} ${${name}_version_file_temp}
            STATUS ${name}_version_download_status
    )
    message(STATUS "Version download ${${name}_version_url} status: ${${name}_version_download_status}")
    if (NOT ${name}_version_download_status EQUAL "0;")
        message(FATAL_ERROR "Unable to download ${${name}_version_url}")
    endif ()
    file(READ ${${name}_version_file_temp} ${name}_version_temp)
    if (NOT EXISTS ${${name}_version_file})
        set(${name}_version 0)
    else ()
        file(READ ${${name}_version_file} ${name}_version)
    endif ()
    message(STATUS "Current version: ${${name}_version}")
    message(STATUS "Current Name: ${${name}_archive_name} ")
    #版本比较
    if (${name}_version_temp VERSION_GREATER ${name}_version)
        message(STATUS "New version ${${name}_version_temp} available")
        file(REMOVE_RECURSE ${CMAKE_CURRENT_BINARY_DIR}/${name})
        file(REMOVE ${CMAKE_CURRENT_BINARY_DIR}/${${name}_archive_name})
        set(${name}_version ${${name}_version_temp})
        file(RENAME ${${name}_version_file_temp} ${${name}_version_file})
        file(DOWNLOAD ${project_url}/${${name}_archive_name} ${CMAKE_CURRENT_BINARY_DIR}/${${name}_archive_name}
                STATUS ${name}_download_status
                SHOW_PROGRESS
        )
        if (NOT ${name}_download_status EQUAL "0;")
            message(FATAL_ERROR "Unable to download ${project_url}/${${name}_archive_name}")
        endif ()
        file(ARCHIVE_EXTRACT
                INPUT ${CMAKE_CURRENT_BINARY_DIR}/${${name}_archive_name}
                DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/${name}
        )
        file(REMOVE ${CMAKE_CURRENT_BINARY_DIR}/${${name}_archive_name})
    endif ()
    file(REMOVE ${${name}_version_file_temp})
    message(STATUS "Version ${${name}_version}")
    if (NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR}/${name}/${name}.cmake)
        file(REMOVE_RECURSE ${CMAKE_CURRENT_BINARY_DIR}/${name})
        file(REMOVE ${CMAKE_CURRENT_BINARY_DIR}/${${name}_archive_name})
        file(REMOVE ${${name}_version_file})
        message(FATAL_ERROR "check config fail ${CMAKE_CURRENT_BINARY_DIR}/${name}/${name}.cmake")
    endif ()
    if (NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR}/${name}/cmake AND NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR}/${name}/lib/cmake)
        message(FATAL_ERROR "check config fail ${CMAKE_CURRENT_BINARY_DIR}/${name}/lib/cmake")
    endif ()
    if (EXISTS ${CMAKE_CURRENT_BINARY_DIR}/${name}/cmake)

        list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_BINARY_DIR}/${name}/cmake")
    endif ()

    if (EXISTS ${CMAKE_CURRENT_BINARY_DIR}/${name}/lib/cmake)
        list(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_BINARY_DIR}/${name}/lib/cmake")
        list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_BINARY_DIR}/${name}/lib/cmake")
    endif ()
endmacro()