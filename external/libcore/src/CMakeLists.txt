cmake_minimum_required(VERSION 3.16)
project(libcore)
set(CMAKE_CXX_STANDARD 17)


set(common_source
        containers/stl_AbstractFifo.cpp
        containers/stl_ArrayBase.cpp
        containers/stl_ListenerList.cpp
        containers/stl_NamedValueSet.cpp
        containers/stl_OwnedArray.cpp
        containers/stl_PropertySet.cpp
        containers/stl_ReferenceCountedArray.cpp
        containers/stl_SparseSet.cpp
        files/stl_DirectoryIterator.cpp
        files/stl_RangedDirectoryIterator.cpp
        files/stl_File.cpp
        files/stl_FileInputStream.cpp
        files/stl_FileOutputStream.cpp
        files/stl_FileSearchPath.cpp
        files/stl_TemporaryFile.cpp
        maths/stl_BigInteger.cpp
        maths/stl_Expression.cpp
        maths/stl_Random.cpp
        memory/stl_MemoryBlock.cpp
        memory/stl_AllocationHooks.cpp
        misc/stl_Result.cpp
        misc/stl_Uuid.cpp
        misc/stl_ConsoleApplication.cpp
        misc/stl_ScopeGuard.cpp
        streams/stl_BufferedInputStream.cpp
        streams/stl_FileInputSource.cpp
        streams/stl_InputStream.cpp
        streams/stl_MemoryInputStream.cpp
        streams/stl_MemoryOutputStream.cpp
        streams/stl_SubregionStream.cpp
        system/stl_SystemStats.cpp
        text/stl_CharacterFunctions.cpp
        text/stl_Identifier.cpp
        text/stl_LocalisedStrings.cpp
        text/stl_String.cpp
        streams/stl_OutputStream.cpp
        text/stl_StringArray.cpp
        text/stl_StringPairArray.cpp
        text/stl_StringPool.cpp
        text/stl_TextDiff.cpp
        text/stl_Base64.cpp
        threads/stl_ReadWriteLock.cpp
        threads/stl_Thread.cpp
        threads/stl_ThreadPool.cpp
        threads/stl_TimeSliceThread.cpp
        time/stl_PerformanceCounter.cpp
        time/stl_RelativeTime.cpp
        time/stl_Time.cpp
        containers/stl_Variant.cpp
        files/stl_FileFilter.cpp
        files/stl_WildcardFileFilter.cpp

        files/stl_memory_file.cpp
        logging/check.cpp
        text/stl_hex_binary.cpp
        text/stl_ByteArray.cpp
        text/stl_StringTokenizer.cpp
        text/stl_Ascii.cpp
        text/stl_NumberParser.cpp
        text/stl_NumericString.cpp
        containers/stl_Collections.cpp
        system/stl_Fnmatch.c
        streams/stl_ZeroCopyByteBuffer.cpp
        streams/stl_ByteBuffer.cpp
        memory/stl_map_memory.cpp
        system/stl_Platform.cpp
        streams/stl_ByteBufferStream.cpp
        system/stl_Common.cpp
        threads/stl_StreamProcess_unix.cpp
        threads/stl_StreamProcess_win.cpp
        exception/stl_Exception.cpp
)

include_directories(logging/spdlog/include)


if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "-- Debug build for core")
    if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
        if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
            if (NOT MINGW)
                message(FATAL_ERROR "windows only support mingw")
            endif ()
        endif ()
        list(APPEND common_source stl_core.cpp)
    elseif (CMAKE_SYSTEM_NAME STREQUAL "Linux" OR CMAKE_SYSTEM_NAME STREQUAL "Android")
        list(APPEND common_source stl_core.cpp)
        # Linux系统
    elseif (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
        list(APPEND common_source stl_core.mm)
    elseif (CMAKE_SYSTEM_NAME STREQUAL "iOS")
        message(FATAL_ERROR "not support ${CMAKE_SYSTEM_NAME}")
    elseif (CMAKE_SYSTEM_NAME STREQUAL "Android")
        message(FATAL_ERROR "not support ${CMAKE_SYSTEM_NAME}")
    else ()
        message(FATAL_ERROR "not support ${CMAKE_SYSTEM_NAME}")
    endif ()
    if (BUILD_SHARED_LIBS)
        add_library(core SHARED ${common_source})
    else ()
        add_library(core STATIC ${common_source})
    endif ()
    add_library(stl::core ALIAS core)
else ()
    message(STATUS "-- Relase build for core")

    if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
        if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
            if (NOT MINGW)
                message(FATAL_ERROR "windows only support mingw")
            endif ()
        endif ()
        list(APPEND common_source stl_core.cpp)
        #[[set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ffunction-sections -fdata-sections -s")
        set(CMAKE_CXX_LINK_FLAGS "${CMAKE_CXX_LINK_FLAGS} -Wl,--gc-sections")]]
    elseif (CMAKE_SYSTEM_NAME STREQUAL "Linux" OR CMAKE_SYSTEM_NAME STREQUAL "Android")
        list(APPEND common_source stl_core.cpp)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ffunction-sections -fdata-sections ")
        set(CMAKE_CXX_LINK_FLAGS "${CMAKE_CXX_LINK_FLAGS} -Wl,--gc-sections")
        # Linux系统
    elseif (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
        list(APPEND common_source stl_core.mm)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ffunction-sections -fdata-sections ")
        set(CMAKE_CXX_LINK_FLAGS "${CMAKE_CXX_LINK_FLAGS} -dead_strip ")
        #set_source_files_properties(your_objc_file.m PROPERTIES LANGUAGE OBJC)
    elseif (CMAKE_SYSTEM_NAME STREQUAL "iOS")
        message(FATAL_ERROR "not support ${CMAKE_SYSTEM_NAME}")
    elseif (CMAKE_SYSTEM_NAME STREQUAL "Android")
        message(FATAL_ERROR "not support ${CMAKE_SYSTEM_NAME}")
    else ()
        message(FATAL_ERROR "not support ${CMAKE_SYSTEM_NAME}")
    endif ()
    if (BUILD_SHARED_LIBS)
        add_library(core SHARED ${common_source})
    else ()
        add_library(core STATIC ${common_source})
    endif ()
    add_library(stl::core ALIAS core)
    target_compile_options(core PRIVATE -O2)
endif ()
if (MINGW)
    target_link_libraries(core PRIVATE -static-libstdc++)
endif ()

#target_include_directories(core PUBLIC ${CMAKE_SOURCE_DIR})
if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_link_libraries(core PUBLIC shlwapi version winmm)
    add_custom_command(TARGET core POST_BUILD
            COMMAND ${CMAKE_STRIP} -s $<TARGET_FILE:core>
            COMMENT "Stripping symbols from the binary"
    )
elseif (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_link_libraries(core PUBLIC libstdc++.a pthread dl)
    # Linux系统
elseif (CMAKE_SYSTEM_NAME STREQUAL "Android")
    add_definitions(-DSTL_LINUX)
    target_link_libraries(core PUBLIC log android)
elseif (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    target_link_libraries(core "-framework Foundation")
    target_link_libraries(core "-framework AppKit")
    target_link_libraries(core "-framework IOKit")
    target_link_libraries(core "-framework Security")
elseif (CMAKE_SYSTEM_NAME STREQUAL "iOS")
    message(FATAL_ERROR "not support ${CMAKE_SYSTEM_NAME}")
else ()
    message(FATAL_ERROR "not support ${CMAKE_SYSTEM_NAME}")
endif ()
#set(CMAKE_VERBOSE_MAKEFILE ON)
set(CORE_TEST OFF)
if (CORE_TEST)
    add_executable(core_test example/main.cpp example/logger.cpp)
    target_link_libraries(core_test stl::core)
    if (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
        target_link_libraries(core_test "-framework Foundation")
        target_link_libraries(core_test "-framework AppKit")
    endif ()
    install(TARGETS core_test DESTINATION ${CMAKE_INSTALL_PREFIX}/bin/)
endif ()

include_directories(./)
#安装库文件
target_include_directories(core INTERFACE ${CMAKE_CURRENT_LIST_DIR}/../include/)
#install stl core

if (UNIX)
    # Linux and other UNIX-like systems
    set(CMAKE_INSTALL_BIN "${CMAKE_INSTALL_PREFIX}/bin/")
    set(CMAKE_INSTALL_LIB "${CMAKE_INSTALL_PREFIX}/lib/")
    install(TARGETS core EXPORT core
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIB}
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIB}
            RUNTIME DESTINATION ${CMAKE_INSTALL_BIN})
else ()
    install(TARGETS core
            EXPORT core)
endif ()

#install headers

install(FILES FindSTL_CORE.cmake DESTINATION ${CMAKE_INSTALL_PREFIX}/cmake/)
