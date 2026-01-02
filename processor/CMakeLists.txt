find_package(boost REQUIRED)
find_package(cmajor REQUIRED)
find_package(spdlog REQUIRED)
find_package(tl-expected REQUIRED)

set(YGGLET_PROCESSOR_SOURCES
    # cmake-format: sortable
    ./src/ygglet/audio/cmajor.cpp
    ./src/ygglet/audio/cmajor.h
    ./inc/ygglet/audio/processor.h
    ./src/ygglet/audio/processor.cpp
    ./src/ygglet/audio/logger.cpp
    ./src/ygglet/audio/logger.h
    ./src/ygglet/audio/module.cpp
    ./src/ygglet/audio/module.h)

add_library(ygglet-processor ${YGGLET_PROCESSOR_SOURCES})

target_include_directories(ygglet-processor PUBLIC inc)
target_include_directories(ygglet-processor PRIVATE src)

target_link_libraries(ygglet-processor PUBLIC cmajor::cmajor tl::expected)
target_link_libraries(ygglet-processor PRIVATE spdlog::spdlog Boost::headers)

if(YGGLET_TESTS)
  add_subdirectory(test)
endif()
