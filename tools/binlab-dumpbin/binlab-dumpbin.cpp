// tools/binlab-dumpbin/binlab-dumpbin.cpp: Dump Binary files

#include <cstddef>
#include <cstring>
#include <fstream>
#include <iterator>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include "binlab/Config.h"
#include "coffdump.h"
#include "elfdump.h"

int main(int argc, char* argv[]) try {
  if (argc < 2) {
    std::cerr << argv[0] << " Version " << BINLAB_VERSION_MAJOR << '.' << BINLAB_VERSION_MINOR << '\n';
    std::cout << "Usage: " << argv[0] << " <input>\n";
    return 1;
  }

  for (int i = 1; i < argc; ++i) {
    if (std::strncmp(argv[i], "--", 2)) {
      std::ifstream is{argv[i], std::ios::binary | std::ios::ate};
      std::printf("dump %s\n", argv[i]);
      const auto& count = is.tellg();
      if (count) {
        std::vector<char> buff(count);
        is.seekg(0, std::ios::beg).read(&buff[0], count);
        for (int j = 1; j < i; ++j) {
          if (!std::strcmp(argv[j], "--import64")) {
            binlab::COFF::dump_import64(&buff[0], count);
          } else if (!std::strcmp(argv[j], "--import32")) {
            binlab::COFF::dump_import32(&buff[0], count);
          } else if (!std::strcmp(argv[j], "--obj-reloc")) {
            binlab::COFF::dump_obj_reloc(&buff[0], count);
          } else if (!std::strcmp(argv[j], "--obj-sym")) {
            binlab::COFF::dump_obj_sym(&buff[0], count);
          } else if (!std::strcmp(argv[j], "--sym64")) {
            binlab::ELF::dump_sym64(&buff[0], count);
          } else {
            continue;
          }
        }
      }
    }
  }

  return 0;
} catch (const std::exception& e) {
  std::cerr << "error: " << e.what() << '\n';
  return 1;
}
