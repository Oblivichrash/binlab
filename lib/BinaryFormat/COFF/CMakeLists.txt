# binlab/tools/binlab-dumpbin/COFF

add_library("COFF"
  "section_header.cpp"
  "export_directory.cpp"
  "import_descriptor.cpp"
  "resource_directory.cpp"
  "base_relocation.cpp"
)

target_include_directories("COFF"
  PRIVATE "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
  INTERFACE "$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>" "$<INSTALL_INTERFACE:include>" "${CMAKE_CURRENT_SOURCE_DIR}"
)
