find_package(Catch2 REQUIRED)
find_package(GTest REQUIRED)

set(ENGINE_TEST_SOURCES # cmake-format: sortable
                        engine.test.cpp)

add_executable(ygglet-engine-tests ${ENGINE_TEST_SOURCES})

target_link_libraries(ygglet-engine-tests
                      PRIVATE ygglet-engine Catch2::Catch2WithMain gtest::gtest)

add_test(NAME ygglet-engine-tests COMMAND "$<TARGET_FILE:ygglet-engine-tests>")
