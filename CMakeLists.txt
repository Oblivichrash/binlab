# binlab: Binary Laboratory

cmake_minimum_required(VERSION 3.10)

project(binlab VERSION 0.1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

configure_file("include/binlab/Config/Config.h.in" "${PROJECT_BINARY_DIR}/binlab/Config/Config.h")

add_subdirectory("tools/bindump")

enable_testing()

add_test(NAME Runs COMMAND bindump "C:/Windows/System32/kernel.appcore.dll")
