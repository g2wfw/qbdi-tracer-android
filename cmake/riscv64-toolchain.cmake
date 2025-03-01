# mingw-toolchain.cmake

# Set the target system to Windows
# Specify the system name
set(CMAKE_SYSTEM_NAME Linux)

# Set the architecture to loongarch64
set(CMAKE_SYSTEM_PROCESSOR riscv64)
set(RISCV64_CROSS ON)
# Specify the cross compiler prefix (64-bit)
set(CMAKE_C_COMPILER riscv64-linux-gnu-gcc-8)
set(CMAKE_CXX_COMPILER riscv64-linux-gnu-g++-8)

# Specify the path to the MinGW installation (adjust the path if necessary)
set(CMAKE_FIND_ROOT_PATH /usr/riscv64-linux-gnu)

# Adjust the search path for libraries and headers
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
