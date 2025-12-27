cmake_minimum_required(4.0.0)

project(ygglet)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
include(ygglet)

setup_corrosion()
