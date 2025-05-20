#

add_library("bl-payload" SHARED
  "payload.cpp"
)

target_include_directories("bl-payload"
  PRIVATE "${PROJECT_SOURCE_DIR}/include" "${PROJECT_BINARY_DIR}/include"
)

install(TARGETS "bl-payload")

add_test(NAME Usage COMMAND "bl-payload")
