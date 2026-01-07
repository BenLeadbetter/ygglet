find_package(boost REQUIRED)

set(ENGINE_SOURCES
    # cmake-format: sortable
    ./inc/ygglet/engine/detail/control_graph.hpp
    ./inc/ygglet/engine/engine.hpp
    ./inc/ygglet/engine/node.hpp
    ./src/ygglet/engine/detail/control_graph.cpp
    ./src/ygglet/engine/engine.cpp
    ./src/ygglet/engine/node.cpp)

add_library(ygglet-engine ${ENGINE_SOURCES})

target_include_directories(ygglet-engine PUBLIC inc)
target_include_directories(ygglet-engine PRIVATE src)

set(ENGINE_PUBLIC_LIBS # cmake-format: sortable
                       Boost::headers)
target_link_libraries(ygglet-engine PUBLIC ${ENGINE_PUBLIC_LIBS})
target_link_libraries(ygglet-engine PRIVATE ${ENGINE_PRIVATE_LIBS})

if(YGGLET_TESTS)
  add_subdirectory(test)
endif()
