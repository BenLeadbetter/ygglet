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

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
include(ygglet)

setup_corrosion()

set(CXX_STANDARD 20)

add_subdirectory(engine)
add_subdirectory(processor)
