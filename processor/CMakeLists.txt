find_package(boost REQUIRED)
find_package(cmajor REQUIRED)
find_package(foonathan_memory REQUIRED)
find_package(spdlog REQUIRED)
find_package(strong_type REQUIRED)
find_package(tl-expected REQUIRED)

set(PROCESSOR_SOURCES
    # cmake-format: sortable
    ./inc/ygglet/processor/graph.hpp
    ./inc/ygglet/processor/kernal_manager.hpp
    ./inc/ygglet/processor/module.hpp
    ./inc/ygglet/processor/node.hpp
    ./inc/ygglet/processor/processor.hpp
    ./src/ygglet/processor/cmajor.cpp
    ./src/ygglet/processor/cmajor.hpp
    ./src/ygglet/processor/graph.cpp
    ./src/ygglet/processor/kernal_manager.cpp
    ./src/ygglet/processor/logger.cpp
    ./src/ygglet/processor/logger.hpp
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
    tl::expected)
set(PROCESSOR_PRIVATE_LIBS # cmake-format: sortable
                           spdlog::spdlog)
target_link_libraries(ygglet-processor PUBLIC ${PROCESSOR_PUBLIC_LIBS})
target_link_libraries(ygglet-processor PRIVATE ${PROCESSOR_PRIVATE_LIBS})

if(YGGLET_TESTS)
  add_subdirectory(test)
endif()
