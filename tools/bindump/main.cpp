// tools/bindump/main.cpp: Dump Binary files

#include <fstream>
#include <iostream>

#include "binlab/Config/Config.h"
#include "pedump.h"
#include "elfdump.h"

using namespace binlab;

int main(int argc, char* argv[]) try {
  if (argc < 2) {
    std::cerr << argv[0] << " Version " << BINLAB_VERSION_MAJOR << '.' << BINLAB_VERSION_MINOR << '\n';
    std::cout << "Usage: " << argv[0] << " <file>\n";
    return 1;
  }

  if (std::ifstream is{argv[1], std::ios::binary | std::ios::ate}) {
    const auto& size = is.tellg();
    std::vector<char> buff(size);
    if (is.seekg(0, std::ios::beg).read(&buff[0], size)) {
      switch (buff[0]) {
        case 'M':
          PE::Dump(buff);
          break;
        case 0x7f:  // ELFMAG0
          ELF::Dump(buff);
          break;
        default:
          break;
      }
    }
  }

  return 0;
} catch (const std::exception& e) {
  std::cerr << "error: " << e.what() << '\n';
  return 1;
}
