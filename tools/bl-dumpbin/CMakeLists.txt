#

add_executable("bl-dumpbin"
  "main.cpp"
)

target_include_directories("bl-dumpbin"
  PRIVATE "${PROJECT_SOURCE_DIR}/include" "${PROJECT_BINARY_DIR}/include"
)

install(TARGETS "bl-dumpbin")
