# binlab/tools/binlab-dumpbin

add_executable("binlab-dumpbin"
  "binlab-dumpbin.cpp"
)

target_include_directories("binlab-dumpbin"
  PUBLIC
  "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
  "$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>"
  "$<INSTALL_INTERFACE:include>"
)

target_link_libraries("binlab-dumpbin"
  PUBLIC COFF
)

install(TARGETS "binlab-dumpbin")
