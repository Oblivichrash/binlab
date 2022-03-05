// tools/bindump/elfdump.cpp:

#include "elfdump.h"
#include "symbols.h"

using namespace binlab;
using namespace binlab::ELF;

inline std::ostream& operator<<(std::ostream& os, const std::pair<const char*, const Elf32_Sym&>& symbol) {
  const auto s = symbol.second;
  os << std::setw(16) << "name: " << symbol.first << '\n';
  os << std::setw(16) << "bind: " << std::showbase << ELF32_ST_BIND(s.st_info) << '\n';
  os << std::setw(16) << "type: " << std::showbase << ELF32_ST_TYPE(s.st_info) << '\n';
  os << std::setw(16) << "visibility: " << std::showbase << ELF32_ST_VISIBILITY(s.st_other) << '\n';
  os << std::setw(16) << "shndx: " << std::showbase << s.st_shndx << '\n';
  os << std::setw(16) << "value: " << std::showbase << s.st_value << '\n';
  os << std::setw(16) << "size: " << std::showbase << s.st_size;
  return os;
}

inline std::ostream& operator<<(std::ostream& os, const std::pair<const char*, const Elf64_Sym&>& symbol) {
  const auto s = symbol.second;
  os << std::setw(16) << "name: " << symbol.first << '\n';
  os << std::setw(16) << "bind: " << std::showbase << ELF64_ST_BIND(s.st_info) << '\n';
  os << std::setw(16) << "type: " << std::showbase << ELF64_ST_TYPE(s.st_info) << '\n';
  os << std::setw(16) << "visibility: " << std::showbase << ELF64_ST_VISIBILITY(s.st_other) << '\n';
  os << std::setw(16) << "shndx: " << std::showbase << s.st_shndx << '\n';
  os << std::setw(16) << "value: " << std::showbase << s.st_value << '\n';
  os << std::setw(16) << "size: " << std::showbase << s.st_size;
  return os;
}

void ELF::Dump(std::vector<char>& buff) {
  if (!std::memcmp(&buff[0], ELFMAG, SELFMAG)) {
    if (buff[EI_CLASS] == ELFCLASS32) {
      if (buff[EI_DATA] == ELFDATA2LSB) {
        std::cout << "dump ELF32 (little-endian)\n";
        //Dump32LE(buff);
      } else if (buff[EI_DATA] == ELFDATA2MSB) {
        std::cerr << "big endian\n";
      } else {
        std::cerr << "invalid ELF data endian\n";
      }
    } else if (buff[EI_CLASS] == ELFCLASS64) {
      if (buff[EI_DATA] == ELFDATA2LSB) {
        std::cout << "dump ELF64 (little-endian)\n";
        Dump64LE(buff);
      } else if (buff[EI_DATA] == ELFDATA2MSB) {
        std::cerr << "big endian\n";
      } else {
        std::cerr << "invalid ELF data endian\n";
      }
    } else {
      std::cerr << "invalid ELF class\n";
    }
  }
}

void ELF::Dump64LE(std::vector<char>& buff) {
  auto ehdr = reinterpret_cast<Elf64_Ehdr*>(&buff[0]);
  auto phdr = reinterpret_cast<Elf64_Phdr*>(&buff[ehdr->e_phoff]);

  Accessor base{&buff[0], phdr, ehdr->e_phnum};
  for (auto i = 0; i < ehdr->e_phnum; ++i) {
    switch (phdr[i].p_type) {
      case PT_DYNAMIC:
        Dump(base, reinterpret_cast<const Elf64_Dyn*>(&base[phdr[i].p_offset]));
        break;
      default:
        break;
    }
  }
}

void ELF::Dump(const Accessor& base, const Elf64_Dyn* dyn) {
  const char* strtab{};
  std::size_t strsz;

  const Elf64_Sym* symtab{};
  std::size_t syment;

  const std::uint32_t* gnu_hash{};

  for (; dyn->d_tag != DT_NULL; ++dyn) {
    switch (dyn->d_tag) {
      case DT_STRTAB:
        strtab = reinterpret_cast<const char*>(&base[dyn->d_un.d_ptr]);
        break;
      case DT_SYMTAB:
        symtab = reinterpret_cast<const Elf64_Sym*>(&base[dyn->d_un.d_ptr]);
        break;
      case DT_STRSZ:
        strsz = dyn->d_un.d_val;
        break;
      case DT_SYMENT:
        syment = dyn->d_un.d_val;
        break;
      case DT_GNU_HASH:
        gnu_hash = reinterpret_cast<const std::uint32_t*>(&base[dyn->d_un.d_ptr]);
        break;
      default:
        break;
    }
  }

  if (strtab == nullptr || symtab == nullptr || gnu_hash == nullptr) {
    std::cerr << "missing needed symbol info\n";
    return;
  }

  symbols symbol{strtab, symtab, static_cast<const void*>(gnu_hash)};

  const char* name = "_ZTISt13runtime_error";
  std::cout << std::hex << std::pair{name, symbol.at(name)} << std::dec << '\n';

  for (std::uint32_t i = 0; i < symbol.bucket_count(); i++) {
    std::cout << "bucket " << i << ": " << symbol.bucket_size(i) << '\n';
  }
}
