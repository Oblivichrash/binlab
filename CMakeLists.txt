# binlab: Binary Laboratory

cmake_minimum_required(VERSION 3.20)

project(BINLAB VERSION 0.1)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(gcc_like_cxx "$<COMPILE_LANG_AND_ID:CXX,ARMClang,AppleClang,Clang,GNU,LCC>")
set(msvc_cxx "$<COMPILE_LANG_AND_ID:CXX,MSVC>")

add_compile_options(
  "$<${gcc_like_cxx}:$<BUILD_INTERFACE:-Wall;-Wextra;-Werror;-Wshadow;-Wformat=2;-Wunused>>"
  "$<${msvc_cxx}:$<BUILD_INTERFACE:-W3>>"
)

configure_file("include/binlab/Config.h.in" "include/binlab/Config.h")
install(FILES "${PROJECT_BINARY_DIR}/include/binlab/Config.h" DESTINATION "include/binlab")

add_subdirectory("lib/BinaryFormat/COFF")
add_subdirectory("tools/binlab-dumpbin")

include(InstallRequiredSystemLibraries)
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_PACKAGE_VERSION_MAJOR "${BINLAB_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${BINLAB_VERSION_MINOR}")
include(CPack)

include(CTest)
add_subdirectory("test/binlab-dumpbin")
