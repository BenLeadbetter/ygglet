find_package(boost REQUIRED)
find_package(cmajor REQUIRED)
find_package(spdlog REQUIRED)
find_package(tl-expected REQUIRED)

set(YGGLET_AUDIO_SOURCES ./inc/ygglet/audio/processor.h
                         ./src/ygglet/audio/processor.cpp)

add_library(ygglet-audio ${YGGLET_AUDIO_SOURCES})

target_include_directories(ygglet-audio PUBLIC inc)

target_link_libraries(ygglet-audio PUBLIC cmajor::cmajor tl::expected)
target_link_libraries(ygglet-audio PRIVATE spdlog::spdlog Boost::headers)

if(YGGLET_TESTS)
  add_subdirectory(test)
endif()
