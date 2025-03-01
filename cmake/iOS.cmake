
function(init_xcode_ios_sdk)
    set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/ios.toolchain.cmake CACHE STRING "for ios.toolchain.cmake path")
    set(PLATFORM OS CACHE STRING "for ios platform")
endfunction()


function(set_target_ios_version sdk_version)
    set(DEPLOYMENT_TARGET ${sdk_version} CACHE STRING "for ios sdk version")
endfunction()