# Support standalone builds (when invoked via cargo/build.rs)
if(NOT TARGET Catch2::Catch2WithMain)
  find_package(Catch2 REQUIRED)
endif()

set(PROCESSOR_TEST_SOURCES # cmake-format: sortable processor.test.cpp
    kernal_manager.test.cpp)

add_executable(ygglet-processor-tests ${PROCESSOR_TEST_SOURCES})

target_link_libraries(ygglet-processor-tests PRIVATE ygglet-processor
                                                     Catch2::Catch2WithMain)

add_test(NAME ygglet-processor-tests
         COMMAND "$<TARGET_FILE:ygglet-processor-tests>")
