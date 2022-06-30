# binlab/tools/pedump

add_executable(pedump
  main.cpp
)

target_include_directories(pedump
  PRIVATE "${PROJECT_SOURCE_DIR}/include"
  PRIVATE "${PROJECT_BINARY_DIR}"
)

install(TARGETS pedump)
