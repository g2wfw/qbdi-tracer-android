macro(SET_OPTION option value)
    set(${option} ${value} CACHE INTERNAL "" FORCE)
endmacro()
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
SET_OPTION(DOBBY_DEBUG OFF)
add_definitions(-DDOBBY_LOGGING_DISABLE)
SET_OPTION(DOBBY_GENERATE_SHARED OFF)
SET_OPTION(Plugin.Android.BionicLinkerUtil ON)
SET_OPTION(Plugin.ImportTableReplace ON)
add_subdirectory(Dobby)
#xdl source file
file(GLOB XDL_SRC xDL/xdl/src/main/cpp/*.c)
add_library(xdl STATIC ${XDL_SRC})
target_include_directories(xdl PUBLIC xDL/xdl/src/main/cpp/include xDL/xdl/src/main/cpp/)


#backtrace
add_subdirectory(backtrace)

#xhook
file(GLOB XHOOK_SRC xHook/libxhook/jni/*.c)
add_library(xHook STATIC ${XHOOK_SRC})
target_include_directories(xHook PUBLIC xHook/libxhook/jni/)


#dex
add_subdirectory(libcore)
add_subdirectory(libsmjni)