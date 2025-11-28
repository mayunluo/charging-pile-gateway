cmake_minimum_required(VERSION 3.13)
project(gateway C)

# Optionally set toolchain file (for cross compile)
if(NOT CMAKE_TOOLCHAIN_FILE)
  # you can call cmake with -DCMAKE_TOOLCHAIN_FILE=cmake/arm_toolchain.cmake
endif()

add_subdirectory(src)
