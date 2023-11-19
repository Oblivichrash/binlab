# binlab/tools/binlab-dumpbin/COFF

add_library("COFF"
  "section_header.cpp"
  "export_directory.cpp"
  "import_descriptor.cpp"
  "resource_directory.cpp"
)

target_include_directories("COFF"
  PUBLIC
  "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
  "$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>"
  "$<INSTALL_INTERFACE:include>"
  INTERFACE
  "${CMAKE_CURRENT_SOURCE_DIR}"
)
