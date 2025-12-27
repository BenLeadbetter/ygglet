cmake_minimum_required(VERSION 4.0.0)

project(ygglet)

option(YGGLET_TESTS "Build tests" ON)

if(YGGLET_TESTS)
  enable_testing()
endif()

include(${CMAKE_BINARY_DIR}/conan/conan_toolchain.cmake)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
include(ygglet)

setup_corrosion()

set(CXX_STANDARD 20)

add_subdirectory(audio)
