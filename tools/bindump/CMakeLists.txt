# binlab/tools/bindump

add_executable(bindump
  main.cpp
#  pedump.cpp
#  elfdump.cpp
)

target_include_directories(bindump
  PRIVATE "${PROJECT_SOURCE_DIR}/include"
  PRIVATE "${PROJECT_BINARY_DIR}"
)

install(TARGETS "bindump")
