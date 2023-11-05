# binlab/tools/binlab-bindump

add_executable(binlab-bindump
  main.cpp
)

target_include_directories(binlab-bindump
  PUBLIC
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
  $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

install(TARGETS binlab-bindump)
