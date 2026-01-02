find_package(Catch2 REQUIRED)

set(YGGLET_AUDIO_TEST_SOURCES processor.test.cpp)

add_executable(ygglet-processor-tests ${YGGLET_AUDIO_TEST_SOURCES})

target_link_libraries(ygglet-processor-tests PRIVATE ygglet-processor
                                                     Catch2::Catch2WithMain)

add_test(NAME ygglet-processor-tests
         COMMAND "$<TARGET_FILE:ygglet-processor-tests>")
