# cmake_minimum_required must be first for standalone builds
cmake_minimum_required(VERSION 3.20)

# Support standalone builds (when invoked via cargo/build.rs)
if(NOT CMAKE_PROJECT_NAME)
  project(ygglet-processor)
endif()

if(NOT TARGET Boost::headers)
  find_package(Boost REQUIRED)
endif()
if(NOT TARGET cmajor::cmajor)
  find_package(cmajor REQUIRED)
endif()
if(NOT TARGET foonathan_memory)
  find_package(foonathan_memory REQUIRED)
endif()
if(NOT TARGET spdlog::spdlog)
  find_package(spdlog REQUIRED)
endif()
if(NOT TARGET rollbear::strong_type)
  find_package(strong_type REQUIRED)
endif()
if(NOT TARGET tl::expected)
  find_package(tl-expected REQUIRED)
endif()

# Handle external engine library (when built via cargo)
if(DEFINED YGGLET_ENGINE_INCLUDE_DIR AND DEFINED YGGLET_ENGINE_LIB_DIR)
  if(NOT TARGET ygglet-engine)
    add_library(ygglet-engine STATIC IMPORTED)
    set_target_properties(
      ygglet-engine
      PROPERTIES IMPORTED_LOCATION "${YGGLET_ENGINE_LIB_DIR}/libygglet-engine.a"
                 INTERFACE_INCLUDE_DIRECTORIES "${YGGLET_ENGINE_INCLUDE_DIR}")
    # Engine's public dependencies
    target_link_libraries(
      ygglet-engine INTERFACE Boost::headers rollbear::strong_type tl::expected)
  endif()
endif()

set(PROCESSOR_SOURCES
    # cmake-format: sortable
    ./inc/ygglet/processor/kernal_manager.hpp
    ./inc/ygglet/processor/module.hpp
    ./inc/ygglet/processor/node.hpp
    ./inc/ygglet/processor/processor.hpp
    ./src/ygglet/processor/cmajor.cpp
    ./src/ygglet/processor/cmajor.hpp
    ./src/ygglet/processor/kernal_manager.cpp
    ./src/ygglet/processor/logger.cpp
    ./src/ygglet/processor/logger.hpp
    ./src/ygglet/processor/module.cpp
    ./src/ygglet/processor/node.cpp
    ./src/ygglet/processor/processor.cpp)

add_library(ygglet-processor ${PROCESSOR_SOURCES})

target_include_directories(ygglet-processor PUBLIC inc)
target_include_directories(ygglet-processor PRIVATE src)

set(PROCESSOR_PUBLIC_LIBS
    # cmake-format: sortable
    Boost::headers cmajor::cmajor foonathan_memory rollbear::strong_type
    tl::expected ygglet-engine)
set(PROCESSOR_PRIVATE_LIBS # cmake-format: sortable
                           spdlog::spdlog)
target_link_libraries(ygglet-processor PUBLIC ${PROCESSOR_PUBLIC_LIBS})
target_link_libraries(ygglet-processor PRIVATE ${PROCESSOR_PRIVATE_LIBS})

if(YGGLET_TESTS)
  add_subdirectory(test)
endif()
