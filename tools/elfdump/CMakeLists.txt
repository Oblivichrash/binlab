# binlab/tools/elfdump

add_executable(elfdump
  main.cpp
  elf_stream.cpp
)

target_include_directories(elfdump
  PRIVATE "${PROJECT_SOURCE_DIR}/include"
  PRIVATE "${PROJECT_BINARY_DIR}"
)

install(TARGETS elfdump)
