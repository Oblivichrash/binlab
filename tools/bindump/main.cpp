// tools/bindump/main.cpp: Dump Binary files

#include <fstream>
#include <iostream>
#include <vector>

#include "binlab/Config/Config.h"
#include "binlab/BinaryFormat/COFF.h"

using namespace binlab::COFF;

namespace PE {

void Dump(std::istream& is) {
  if (auto size = is.seekg(0, std::ios::end).tellg()) {
    std::vector<char> buff(size);
    if (is.seekg(0, std::ios::beg).read(&buff[0], size)) {
      auto& dos = reinterpret_cast<IMAGE_DOS_HEADER&>(buff[0]);
      if (dos.e_magic == IMAGE_DOS_SIGNATURE) {
        auto& nt = reinterpret_cast<IMAGE_NT_HEADERS64&>(buff[dos.e_lfanew]);
        if (nt.Signature == IMAGE_NT_SIGNATURE) {
          auto sections = IMAGE_FIRST_SECTION(&nt);
          for (std::size_t i = 0; i < nt.FileHeader.NumberOfSections; ++i) {
            std::cout << sections[i].Name << '\n';
          }
        }
      }
    }
  }
}

}  // namespace PE

int main(int argc, char* argv[]) try {
  if (argc < 2) {
    std::cerr << argv[0] << " Version " << BINLAB_VERSION_MAJOR << '.' << BINLAB_VERSION_MINOR << '\n';
    std::cout << "Usage: " << argv[0] << " <file>\n";
    return 1;
  }

    if (std::ifstream is{argv[1], std::ios::binary}) {
    switch (is.peek()) {
      case 'M':
        PE::Dump(is);
        break;
      case 0x7f:
        //ELF::Dump(is);
        break;
      default:
        break;
    }
  }

  return 0;
} catch (const std::exception& e) {
  std::cerr << "error: " << e.what() << '\n';
  return 1;
}
