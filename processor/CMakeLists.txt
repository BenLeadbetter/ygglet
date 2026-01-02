find_package(boost REQUIRED)
find_package(cmajor REQUIRED)
find_package(spdlog REQUIRED)
find_package(tl-expected REQUIRED)

set(YGGLET_PROCESSOR_SOURCES
    # cmake-format: sortable
    ./inc/ygglet/processor/processor.hpp
    ./src/ygglet/processor/cmajor.cpp
    ./src/ygglet/processor/cmajor.hpp
    ./src/ygglet/processor/logger.cpp
    ./src/ygglet/processor/logger.hpp
    ./src/ygglet/processor/module.cpp
    ./src/ygglet/processor/module.hpp
    ./src/ygglet/processor/processor.cpp)

add_library(ygglet-processor ${YGGLET_PROCESSOR_SOURCES})

target_include_directories(ygglet-processor PUBLIC inc)
target_include_directories(ygglet-processor PRIVATE src)

target_link_libraries(ygglet-processor PUBLIC cmajor::cmajor tl::expected)
target_link_libraries(ygglet-processor PRIVATE spdlog::spdlog Boost::headers)

if(YGGLET_TESTS)
  add_subdirectory(test)
endif()
