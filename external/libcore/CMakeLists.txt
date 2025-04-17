include_directories(include/core)
add_subdirectory(src)
install(DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/include" # source directory
        DESTINATION "${CMAKE_INSTALL_PREFIX}/" # target directory
        FILES_MATCHING # install only matched files
        PATTERN "*.h" # select header files
        )