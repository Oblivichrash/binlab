// tools/bindump/main.cpp: Dump Binary files

#include <algorithm>
#include <fstream>
#include <iostream>
#include <vector>

#include "binlab/Config/Config.h"
//#include <Windows.h>
#include "binlab/BinaryFormat/COFF.h"

using namespace binlab::COFF;

namespace PE {

class Accessor {
 public:
  Accessor(void* base, const IMAGE_SECTION_HEADER* first, const std::size_t num)
      : base_{static_cast<char*>(base)}, first_{first}, last_{first + num} {}

  const char& operator[](std::size_t offset) const {
    return base_[VirtualAddressToFileOffset(static_cast<DWORD>(offset))];
  }

 private:
  DWORD VirtualAddressToFileOffset(const DWORD va) const {
    auto pred = [va](const IMAGE_SECTION_HEADER& section) {
      auto section_base = section.VirtualAddress;
      return (section_base <= va) && (va < section_base + section.SizeOfRawData);
    };

    auto iter = std::find_if(first_, last_, pred);
    if (iter == last_) {
      throw std::out_of_range{"virtual address out of range"};
    }
    return iter->PointerToRawData + (va - iter->VirtualAddress);
  }

 private:
  char* base_;
  const IMAGE_SECTION_HEADER* first_;
  const IMAGE_SECTION_HEADER* last_;
};

void DumpReloc64(void* base, const IMAGE_NT_HEADERS64& nt, const IMAGE_SECTION_HEADER* sections) {
  Accessor va{base, sections, nt.FileHeader.NumberOfSections};

  const char* ptr = &va[nt.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress];
  if (ptr == nullptr) {
    return;
  }

  for (const IMAGE_BASE_RELOCATION* relocation; relocation = reinterpret_cast<const IMAGE_BASE_RELOCATION*>(ptr); ptr += relocation->SizeOfBlock) {
    if (relocation->SizeOfBlock == 0 || relocation->VirtualAddress == 0) {
      break;
    }

    if (relocation->SizeOfBlock < sizeof(*relocation)) {
      break;
    }

    std::cout << "Virtual Address: " << relocation->VirtualAddress << " Size: " << relocation->SizeOfBlock;
    auto entry = reinterpret_cast<const WORD*>(relocation + 1);
    for (std::size_t j = 0; j < (relocation->SizeOfBlock - sizeof(*relocation)) / sizeof(*entry); ++j) {
      std::cout << entry[j] << '\n';
    }
    std::cout << '\n';
  }
}

void Dump(std::istream& is) {
  if (auto size = is.seekg(0, std::ios::end).tellg()) {
    std::vector<char> buff(size);
    if (is.seekg(0, std::ios::beg).read(&buff[0], size)) {
      auto& dos = reinterpret_cast<IMAGE_DOS_HEADER&>(buff[0]);
      if (dos.e_magic == IMAGE_DOS_SIGNATURE) {
        auto& nt = reinterpret_cast<IMAGE_NT_HEADERS64&>(buff[dos.e_lfanew]);
        if (nt.Signature == IMAGE_NT_SIGNATURE && nt.OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC) {
          auto sections = IMAGE_FIRST_SECTION(&nt);
          for (std::size_t i = 0; i < nt.FileHeader.NumberOfSections; ++i) {
            std::cout << sections[i].Name << '\n';
          }
          std::cout << std::hex;
          DumpReloc64(&buff[0], nt, sections);
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
