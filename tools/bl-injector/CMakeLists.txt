#

add_executable("bl-injector"
  "main.cpp"
)

target_include_directories("bl-injector"
  PRIVATE "${PROJECT_SOURCE_DIR}/include" "${PROJECT_BINARY_DIR}/include"
)

add_subdirectory("payload")

install(TARGETS "bl-injector")

add_test(NAME Usage COMMAND "bl-injector")
