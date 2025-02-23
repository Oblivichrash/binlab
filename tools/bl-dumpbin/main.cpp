//

#include <cstdio>
#include <fstream>
#include <vector>

#include "binlab/Config.h"
#include "binlab/BinaryFormat/COFF.h"

using namespace binlab::COFF;

int dump_nt64_sections(const char* buff) {
  const auto& Dos = reinterpret_cast<const IMAGE_DOS_HEADER&>(buff[0]);
  if (Dos.e_magic == IMAGE_DOS_SIGNATURE) {
    const auto& Nt64 = reinterpret_cast<const IMAGE_NT_HEADERS64&>(buff[Dos.e_lfanew]);
    if (Nt64.Signature == IMAGE_NT_SIGNATURE && Nt64.OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC) {
      const auto Sections = IMAGE_FIRST_SECTION(&Nt64);
      for (std::size_t i = 0; i < Nt64.FileHeader.NumberOfSections; ++i) {
        std::printf("%8s\n", Sections[i].Name);
      }
    }
  }
  return 0;
}

int dump_obj64_sections(const char* buff) {
  auto& FileHeader = reinterpret_cast<const IMAGE_FILE_HEADER&>(buff[0]);
  std::printf("NumberOfSections: %d\n", FileHeader.NumberOfSections);
  auto Sections = reinterpret_cast<const IMAGE_SECTION_HEADER*>(&buff[sizeof(FileHeader) + FileHeader.SizeOfOptionalHeader]);
  for (std::size_t i = 0; i < FileHeader.NumberOfSections; ++i) {
    std::printf("%8s\n", Sections[i].Name);
  }
  return 0;
}

int main(int argc, char* argv[]) {
  if (argc < 2) {
    std::printf("%s ver: %d.%d\n", argv[0], BINLAB_VERSION_MAJOR, BINLAB_VERSION_MINOR);
    return 0;
  }

  std::ifstream is{argv[1], std::ios::binary | std::ios::ate};
  if (is) {
    std::printf("dump %s\n", argv[1]);
    const auto& count = is.tellg();
    if (count) {
      std::vector<char> buff(count);
      if (is.seekg(0, std::ios::beg).read(&buff[0], count)) {
        //dump_nt64_sections(&buff[0]);
        dump_obj64_sections(&buff[0]);
      }
    }
  }
  return 0;
}
