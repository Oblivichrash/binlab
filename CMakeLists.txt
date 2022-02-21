# binlab: Binary Laboratory

cmake_minimum_required(VERSION 3.10)

project(binlab VERSION 0.1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

configure_file("include/binlab/Config/Config.h.in" "${PROJECT_BINARY_DIR}/binlab/Config/Config.h")

add_subdirectory("tools/bindump")

enable_testing()

add_test(NAME DumpPE32 COMMAND bindump "${PROJECT_SOURCE_DIR}/sample/PE32/kernel32.dll")
add_test(NAME DumpPE64 COMMAND bindump "${PROJECT_SOURCE_DIR}/sample/PE64/kernel32.dll")

add_test(NAME DumpELF64LE COMMAND bindump "${PROJECT_SOURCE_DIR}/sample/ELF64LE/libstdc++.so.6")
