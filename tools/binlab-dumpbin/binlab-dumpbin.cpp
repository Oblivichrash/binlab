// tools/binlab-dumpbin/binlab-dumpbin.cpp: Dump Binary files

#include <cstddef>
#include <cstring>
#include <fstream>
#include <iterator>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include "coffdump.h"
#include "binlab/Config.h"
#include "binlab/BinaryFormat/ELF.h"

#include "symbols.h"

using namespace binlab::ELF;

template <typename Hash>
std::ostream& dump_hash(std::ostream& os, const Hash& hashtab, const char* strtab) {
  auto hasher = hashtab.hash_function();
  for (std::size_t n = 0; n < hashtab.bucket_count(); ++n) {
    for (auto iter = hashtab.begin(n); iter != hashtab.end(n); ++iter) {
      auto hash = hasher(*iter);
      os << std::setw(9) << hash << "(" << std::setw(4) << (hash % hashtab.bucket_count()) << "): " << &strtab[iter->st_name] << '\n';
    }
    os << '\n';
  }

  const char* name = "__bss_start__";
  auto iter =  hashtab.find(name);
  if (iter != hashtab.end()) {
    auto hash = hasher(*iter);
    os << std::setw(9) << hash << "(" << std::setw(4) << (hash % hashtab.bucket_count()) << "): " << &strtab[iter->st_name] << '\n';
  } else {
    os << "coundn't find: " << name << '\n';
  }
  return os;
}

template <typename Dyn>
std::ostream& dump_dynamic(std::ostream& os, Dyn* dyn, const char* strtab) {
  for (; dyn->d_tag != DT_NULL; ++dyn) {
    switch (dyn->d_tag) {
      case DT_NEEDED:
        os << &strtab[dyn->d_un.d_val] << '\n';
        break;
      default:
        break;
    }
  }
  return os;
}

std::ostream& dump_elf(std::ostream& os, char* buff) {
  if (!std::equal(buff, buff + SELFMAG, ELFMAG) || buff[EI_CLASS] != ELFCLASS64) {
    return os << "invalid ELF type\n";
  }

  auto& ehdr = reinterpret_cast<Elf64_Ehdr&>(buff[0]);
  auto shdr = reinterpret_cast<Elf64_Shdr*>(&buff[ehdr.e_shoff]);
  //auto shstr = &buff[shdr[ehdr.e_shstrndx].sh_offset];
  for (auto iter = shdr; iter != shdr + ehdr.e_shnum; ++iter) {
    //os << &shstr[iter->sh_name] << '\n';
    switch (iter->sh_type) {
      case SHT_HASH: {
        auto hash = &buff[iter->sh_offset];
        auto symtab = reinterpret_cast<Elf64_Sym*>(&buff[shdr[iter->sh_link].sh_offset]);
        auto strtab = &buff[shdr[shdr[iter->sh_link].sh_link].sh_offset];
        dump_hash(os, sysv_hash_table{symtab, strtab, hash}, strtab);
        break;
      }
      case SHT_DYNAMIC: {
        auto strtab = &buff[shdr[iter->sh_link].sh_offset];
        auto dyn = reinterpret_cast<Elf64_Dyn*>(&buff[iter->sh_offset]);
        os << "dyn offset: " << std::hex << iter->sh_offset << '\n';
        dump_dynamic(os, dyn, strtab);
        break;
      }
      case SHT_GNU_HASH: {
        auto hash = &buff[iter->sh_offset];
        auto symtab = reinterpret_cast<Elf64_Sym*>(&buff[shdr[iter->sh_link].sh_offset]);
        auto strtab = &buff[shdr[shdr[iter->sh_link].sh_link].sh_offset];
        dump_hash(os, gnu_hash_table{symtab, strtab, hash}, strtab);
        break;
      }
      default:
        break;
    }
  }
  return os;
}

int main(int argc, char* argv[]) try {
  if (argc < 2) {
    std::cerr << argv[0] << " Version " << BINLAB_VERSION_MAJOR << '.' << BINLAB_VERSION_MINOR << '\n';
    std::cout << "Usage: " << argv[0] << " <input>\n";
    return 1;
  }

  for (std::size_t i = 1; i < argc; ++i) {
    if (std::strncmp(argv[i], "--", 2)) {
      std::ifstream is{argv[i], std::ios::binary | std::ios::ate};
      std::printf("dump %s\n", argv[i]);
      const auto& count = is.tellg();
      if (count) {
        std::vector<char> buff(count);
        is.seekg(0, std::ios::beg).read(&buff[0], count);
        for (std::size_t j = 1; j < i; ++j) {
          if (!std::strcmp(argv[j], "--import64")) {
            binlab::COFF::dump_import64(&buff[0], count);
          } else if (!std::strcmp(argv[j], "--import32")) {
            binlab::COFF::dump_import32(&buff[0], count);
          } else if (!std::strcmp(argv[j], "--obj")) {
            binlab::COFF::dump_obj(&buff[0], count);
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
