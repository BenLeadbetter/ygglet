# cmake_minimum_required must be first for standalone builds
cmake_minimum_required(VERSION 3.20)

# Support standalone builds (when invoked via cargo/build.rs)
if(NOT CMAKE_PROJECT_NAME)
  project(ygglet-engine)
endif()

if(NOT TARGET Boost::headers)
  find_package(Boost REQUIRED)
endif()
if(NOT TARGET tl::expected)
  find_package(tl-expected REQUIRED)
endif()
if(NOT TARGET rollbear::strong_type)
  find_package(strong_type REQUIRED)
endif()

set(ENGINE_SOURCES
    # cmake-format: sortable
    ./src/ygglet/engine/connection.cpp
    ./src/ygglet/engine/detail/control_graph.cpp
    ./src/ygglet/engine/detail/render_graph.cpp
    ./src/ygglet/engine/engine.cpp
    ./src/ygglet/engine/node.cpp)

add_library(ygglet-engine ${ENGINE_SOURCES})

target_include_directories(ygglet-engine PUBLIC inc)
target_include_directories(ygglet-engine PRIVATE src)

set(ENGINE_PUBLIC_LIBS # cmake-format: sortable
                       Boost::headers rollbear::strong_type tl::expected)
target_link_libraries(ygglet-engine PUBLIC ${ENGINE_PUBLIC_LIBS})
target_link_libraries(ygglet-engine PRIVATE ${ENGINE_PRIVATE_LIBS})

if(YGGLET_TESTS)
  add_subdirectory(test)
endif()
