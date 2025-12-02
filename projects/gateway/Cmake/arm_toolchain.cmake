set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)

# adjust this to your installed cross compiler prefix
set(CROSS_COMPILER_PREFIX arm-linux-gnueabihf)
set(CMAKE_C_COMPILER ${CROSS_COMPILER_PREFIX}-gcc)
set(CMAKE_CXX_COMPILER ${CROSS_COMPILER_PREFIX}-g++)

# sysroot (optional)
#set(CMAKE_SYSROOT /opt/gcc-arm/arm-linux-gnueabihf/sysroot)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
