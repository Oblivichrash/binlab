# binlab: Binary Laboratory

cmake_minimum_required(VERSION 3.10)

project(binlab VERSION 0.1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(gcc_like_cxx "$<COMPILE_LANG_AND_ID:CXX,ARMClang,AppleClang,Clang,GNU,LCC>")
set(msvc_cxx "$<COMPILE_LANG_AND_ID:CXX,MSVC>")

add_compile_options(
  "$<${gcc_like_cxx}:$<BUILD_INTERFACE:-Wall;-Wextra;-Wshadow;-Wformat=2;-Wunused>>"
  "$<${msvc_cxx}:$<BUILD_INTERFACE:-W3>>"
)

configure_file("include/binlab/Config/Config.h.in" "${PROJECT_BINARY_DIR}/binlab/Config/Config.h")

add_subdirectory("tools/bindump")

enable_testing()

add_test(NAME DumpPE32 COMMAND bindump "${PROJECT_SOURCE_DIR}/test/sample/PE32/kernel32.dll")
add_test(NAME DumpPE64 COMMAND bindump "${PROJECT_SOURCE_DIR}/test/sample/PE64/kernel32.dll")

add_test(NAME DumpELF64LE COMMAND bindump "${PROJECT_SOURCE_DIR}/test/sample/ELF64LE/libstdc++.so.6")
