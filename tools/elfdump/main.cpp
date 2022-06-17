// tools/elfdump/main.cpp: Dump Binary files

#include <elf.h>

#include <fstream>
#include <iomanip>
#include <iostream>
#include <vector>

#include "binlab/Config/Config.h"

std::ostream& memdump(std::ostream& os, void* base, std::size_t rows) {
  auto buff = static_cast<std::uint8_t*>(base);
  for (std::size_t i = 0; i < rows; ++i, buff += 16) {
    for (std::size_t j = 0; j < 16; ++j) {
      os << ' ' << std::setw(2) << std::setfill('0') << unsigned{buff[j]};
    }
    os << '\n';
  }
  return os;
}

template <typename Shdr, typename Ehdr>
std::ostream& shdrdump(std::ostream& os, void* base) {
  auto buff = static_cast<std::uint8_t*>(base);

  auto& ehdr = reinterpret_cast<Ehdr&>(buff[0]);
  auto shdr = reinterpret_cast<Shdr*>(&buff[ehdr.e_shoff]);

  auto shstrtab = reinterpret_cast<char*>(&buff[shdr[ehdr.e_shstrndx].sh_offset]);
  for (std::size_t i = 0; i < ehdr.e_shnum; ++i) {
    os << &shstrtab[shdr[i].sh_name] << '\n';
  }
  os << '\n';

  return os;
}

int main(int argc, char* argv[]) try {
  if (argc < 2) {
    std::cerr << argv[0] << " Version " << BINLAB_VERSION_MAJOR << '.' << BINLAB_VERSION_MINOR << '\n';
    std::cout << "Usage: " << argv[0] << " <file>\n";
    return 1;
  }

  if (std::ifstream is{argv[1], std::ios::binary | std::ios::ate}) {
    const auto size = is.tellg();
    std::vector<char> buff(size);
    if (is.seekg(0).read(&buff[0], size)) {
      std::cout << std::hex;
      memdump(std::cout, buff.data(), 16);
      if (buff[EI_CLASS] == ELFCLASS64) {
        shdrdump<Elf64_Shdr, Elf64_Ehdr>(std::cout, buff.data());
      } else if (buff[EI_CLASS] == ELFCLASS32) {
        shdrdump<Elf32_Shdr, Elf32_Ehdr>(std::cout, buff.data());
      } else {
        std::cerr << "the ELF class is invalid\n";
      }
    } else {
      std::cerr << "read failed\n";
    }
  } else {
    std::cerr << "open failed\n";
  }

  return 0;
} catch (const std::exception& e) {
  std::cerr << "error: " << e.what() << '\n';
  return 1;
}
