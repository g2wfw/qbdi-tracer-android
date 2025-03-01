
if (CMAKE_SYSTEM_NAME MATCHES "Android")

else ()
    find_package(JNI REQUIRED)
    include_directories(${JAVA_INCLUDE_PATH})
    message(${JAVA_INCLUDE_PATH})
    include_directories(${JAVA_INCLUDE_PATH2})
    message(${JAVA_INCLUDE_PATH2})
endif ()
aux_source_directory(./ smjni_src)
set(CMAKE_CXX_STANDARD 17)
add_library(smjni STATIC ${smjni_src})
target_include_directories(smjni PUBLIC ${CMAKE_CURRENT_LIST_DIR}/smjni)

