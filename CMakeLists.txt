#

cmake_minimum_required(VERSION 3.20)

project(BINLAB VERSION 0.1)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

configure_file("include/binlab/Config.h.in" "include/binlab/Config.h")

include(CTest)

add_subdirectory("tools/bl-dumpbin")
