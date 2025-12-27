find_package(Catch2 REQUIRED)

set(YGGLET_AUDIO_TEST_SOURCES processor.test.cpp)

add_executable(ygglet-audio-tests ${YGGLET_AUDIO_TEST_SOURCES})

target_link_libraries(ygglet-audio-tests PRIVATE ygglet-audio
                                                 Catch2::Catch2WithMain)

add_test(NAME ygglet-audio-tests COMMAND "$<TARGET_FILE:ygglet-audio-tests>")
