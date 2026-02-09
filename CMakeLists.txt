cmake_minimum_required(VERSION 4.0.0)

project(ygglet)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE
      Debug
      CACHE STRING "Choose the type of build." FORCE)
endif()

option(YGGLET_TESTS "Build tests" ON)

if(YGGLET_TESTS)
  enable_testing()
endif()

include("${CMAKE_TOOLCHAIN_FILE}")

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(engine)
add_subdirectory(processor)
