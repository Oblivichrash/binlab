// tools/bindump/elfdump.cpp:

#include <algorithm>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>

#include "elfdump.h"
#include "segments.h"
#include "symbols.h"
#include "Accessor.h"

using namespace binlab;
using namespace binlab::ELF;

template <typename T>
struct sym_traits;

template<>
struct sym_traits<Elf64_Sym> {
  using value_type        = Elf64_Sym;
  using const_reference   = const value_type&;

  static constexpr auto bind(const_reference sym) { return ELF64_ST_BIND(sym.st_info); }
  static constexpr auto type(const_reference sym) { return ELF64_ST_TYPE(sym.st_info); }

  static constexpr auto visibility(const_reference sym) { return ELF64_ST_VISIBILITY(sym.st_info); }
};

template<>
struct sym_traits<Elf32_Sym> {
  using value_type        = Elf32_Sym;
  using const_reference   = const value_type&;

  static constexpr auto bind(const_reference sym) { return ELF32_ST_BIND(sym.st_info); }
  static constexpr auto type(const_reference sym) { return ELF32_ST_TYPE(sym.st_info); }

  static constexpr auto visibility(const_reference sym) { return ELF32_ST_VISIBILITY(sym.st_info); }
};

template <typename T>
struct dyn_traits;

template <>
struct dyn_traits<Elf64_Dyn> {
  using Elf_Sym = Elf64_Sym;
};

template <>
struct dyn_traits<Elf32_Dyn> {
  using Elf_Sym = Elf32_Sym;
};

template <typename T>
inline std::ostream& operator<<(std::ostream& os, const std::pair<char*, T*>& symbol) {
  using Traits = sym_traits<T>;

  const auto& s = *symbol.second;
  os << std::setw(16) << "name: " << symbol.first << '\n';
  os << std::setw(16) << "bind: " << std::showbase << Traits::bind(s) << '\n';
  os << std::setw(16) << "type: " << std::showbase << Traits::type(s) << '\n';
  os << std::setw(16) << "visibility: " << std::showbase << Traits::visibility(s) << '\n';
  os << std::setw(16) << "shndx: " << std::showbase << s.st_shndx << '\n';
  os << std::setw(16) << "value: " << std::showbase << s.st_value << '\n';
  os << std::setw(16) << "size: " << std::showbase << s.st_size;
  return os;
}

template <typename Elf_Phdr, typename Elf_Dyn>
void DumpSym(Accessor<Elf_Phdr>& base, Elf_Dyn* dyn) {
  using Traits = dyn_traits<Elf_Dyn>;

  using Elf_Sym = typename dyn_traits<Elf_Dyn>::Elf_Sym;

  char* strtab{};
  std::size_t strsz;

  Elf_Sym* symtab{};
  std::size_t syment;

  std::uint32_t* gnu_hash{};
  std::uint32_t* elf_hash{};

  for (; dyn->d_tag != DT_NULL; ++dyn) {
    switch (dyn->d_tag) {
      case DT_HASH:
        elf_hash = reinterpret_cast<std::uint32_t*>(&base[dyn->d_un.d_ptr]);
        break;
      case DT_STRTAB:
        strtab = reinterpret_cast<char*>(&base[dyn->d_un.d_ptr]);
        break;
      case DT_SYMTAB:
        symtab = reinterpret_cast<Elf_Sym*>(&base[dyn->d_un.d_ptr]);
        break;
      case DT_STRSZ:
        strsz = dyn->d_un.d_val;
        break;
      case DT_SYMENT:
        syment = dyn->d_un.d_val;
        break;
      case DT_GNU_HASH:
        gnu_hash = reinterpret_cast<std::uint32_t*>(&base[dyn->d_un.d_ptr]);
        break;
      default:
        break;
    }
  }

  if (strtab == nullptr || symtab == nullptr) {
    std::cerr << "missing needed symbol info\n";
    return;
  }

  std::cout << std::hex;

  if (elf_hash) {
    sysv_hash_table table{symtab, strtab, elf_hash};

    for (const auto& s : table) {
      std::cout << &strtab[s.st_name] << '\n';
    }
    std::cout << '\n';

    for (size_t i = 0; i < table.bucket_count(); i++) {
      std::cout << "bucket: " << i << '\n';
      for (auto iter = table.begin(i); iter != table.end(i); ++iter) {
        std::cout << &strtab[iter->st_name] << '\n';
      }
      std::cout << '\n';
    }
  }

  if (gnu_hash) {
    gun_hash_table table{symtab, strtab, gnu_hash};

    std::size_t count = 0;
    for (const auto& s : table) {
      std::cout << count << ": " << &strtab[s.st_name] << '\n';
      ++count;
    }
    std::cout << '\n';

    for (size_t i = 0; i < table.bucket_count(); i++) {
      std::cout << "bucket: " << i << '\n';
      for (auto iter = table.begin(i); iter != table.end(i); ++iter) {
        std::cout << &strtab[iter->st_name] << '\n';
      }
      std::cout << '\n';
    }
  }
}

void Dump64LE(std::vector<char>& buff) {
  auto ehdr = reinterpret_cast<Elf64_Ehdr*>(&buff[0]);
  auto phdr = reinterpret_cast<Elf64_Phdr*>(&buff[ehdr->e_phoff]);

  Accessor<Elf64_Phdr> base{&buff[0], phdr, ehdr->e_phnum};
  for (auto i = 0; i < ehdr->e_phnum; ++i) {
    switch (phdr[i].p_type) {
      case PT_DYNAMIC:
        DumpSym(base, reinterpret_cast<Elf64_Dyn*>(&base[phdr[i].p_vaddr]));
        break;
      default:
        break;
    }
  }
}

void Dump32LE(std::vector<char>& buff) {
  auto ehdr = reinterpret_cast<Elf32_Ehdr*>(&buff[0]);
  auto phdr = reinterpret_cast<Elf32_Phdr*>(&buff[ehdr->e_phoff]);

  Accessor<Elf32_Phdr> base{&buff[0], phdr, ehdr->e_phnum};
  for (auto i = 0; i < ehdr->e_phnum; ++i) {
    switch (phdr[i].p_type) {
      case PT_DYNAMIC:
        DumpSym(base, reinterpret_cast<Elf32_Dyn*>(&base[phdr[i].p_vaddr]));
        break;
      default:
        break;
    }
  }
}

void ELF::Dump(std::vector<char>& buff) {
  if (!std::memcmp(&buff[0], ELFMAG, SELFMAG)) {
    if (buff[EI_CLASS] == ELFCLASS32) {
      if (buff[EI_DATA] == ELFDATA2LSB) {
        std::cout << "dump ELF32 (little-endian)\n";
        Dump32LE(buff);
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
