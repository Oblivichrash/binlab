// tools/bindump/elfdump.cpp:

#include <algorithm>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <vector>

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
  using Elf_XWord = Elf64_Xword;

  using symbol_iterator = Elf64_Sym*;
};

template <>
struct dyn_traits<Elf32_Dyn> {
  using Elf_Sym = Elf32_Sym;
  using Elf_XWord = Elf32_Word;

  using symbol_iterator = Elf32_Sym*;
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
void DumpDynamic(Accessor<Elf_Phdr>& base, Elf_Dyn* dyn) {
  using Elf_Sym = typename dyn_traits<Elf_Dyn>::Elf_Sym;
  using Elf_XWord = typename dyn_traits<Elf_Dyn>::Elf_XWord;

  char* strtab{};
  std::size_t strsz;

  Elf_Sym* symtab{};
  std::size_t syment;

  std::uint32_t* gnu_hash{};
  std::uint32_t* hash{};

  std::vector<Elf_XWord> needed;
  Elf_XWord soname = 0;

  for (; dyn->d_tag != DT_NULL; ++dyn) {
    switch (dyn->d_tag) {
      case DT_NEEDED:
        needed.push_back(dyn->d_un.d_val);  // insert value by reference
        break;
      case DT_HASH:
        hash = reinterpret_cast<std::uint32_t*>(&base[dyn->d_un.d_ptr]);
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
      case DT_SONAME:
        soname = dyn->d_un.d_val;
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

  std::cout << "strsz: " << strsz << '\n';
  std::cout << "syment: " << syment << '\n';

  if (!needed.empty()) {
    std::cout << "needed: ";
    for (const auto v : needed) {
      std::cout << &strtab[v] << ' ';
    }
    std::cout << '\n';
  }

  std::cout << "soname: " << &strtab[soname] << '\n';

  if (hash) {
    sysv_hash_table table{symtab, strtab, hash};
    for (size_t i = 0; i < table.bucket_count(); i++) {
      for (auto iter = table.begin(i); iter != table.end(i); ++iter) {
        auto name = &strtab[iter->st_name];
        std::cout << std::setw(4) << i << std::setw(9) << table.hash_value(name) << ' ' << name << '\n';
      }
      std::cout << '\n';
    }

    auto name = "register_printf_function";
    auto iter = table.find(name);
    if (iter != table.end()) {
      std::cout << name << " found\n";
    } else {
      std::cout << name << " not found!\n";
    }
  }

  if (gnu_hash) {
    gun_hash_table table{symtab, strtab, gnu_hash};
    for (size_t i = 0; i < table.bucket_count(); i++) {
      for (auto iter = table.begin(i); iter != table.end(i); ++iter) {
        auto name = &strtab[iter->st_name];
        std::cout << std::setw(4) << i << std::setw(9) << table.hash_value(name) << ' ' << name << '\n';
      }
      std::cout << '\n';
    }

    auto name = "_ZNSt16__numpunct_cacheIcED1Ev";
    auto iter = table.find(name);
    if (iter != table.end()) {
      std::cout << name << " found\n";
    } else {
      std::cout << name << " not found!\n";
    }
  }
}

template <typename T>
struct ehdr_traits;

template <>
struct ehdr_traits<Elf64_Ehdr> {
  using section_iterator = Elf64_Shdr*;
  using segment_iterator = Elf64_Phdr*;
};

template <>
struct ehdr_traits<Elf32_Ehdr> {
  using section_iterator = Elf32_Shdr*;
  using segment_iterator = Elf32_Phdr*;
};

template <typename T>
struct phdr_traits;

template <>
struct phdr_traits<Elf64_Phdr> {
  using dynamic_iterator = Elf64_Dyn*;
};

template <>
struct phdr_traits<Elf32_Phdr> {
  using dynamic_iterator = Elf32_Dyn*;
};

template <typename T>
struct shdr_traits;

template <>
struct shdr_traits<Elf64_Shdr> {
  using rel_iterator = Elf64_Rel*;
  using rela_iterator = Elf64_Rela*;
};

template <>
struct shdr_traits<Elf32_Shdr> {
  using rel_iterator = Elf32_Rel*;
  using rela_iterator = Elf32_Rela*;
};

template <typename Ehdr, typename EhdrTraits = ehdr_traits<Ehdr>>
int DumpHeader(std::vector<char>& buff, const Ehdr* ehdr) {
  auto shdr = reinterpret_cast<typename EhdrTraits::section_iterator>(&buff[ehdr->e_shoff]);

  char* shstrtab = &buff[shdr[ehdr->e_shstrndx].sh_offset];
  for (std::size_t i = 0; i < ehdr->e_shnum; ++i) {
    //if (!std::strcmp(&shstrtab[shdr[i].sh_name], ".rodata")) {
    //  std::cout << "found .rodata, size: " << shdr[i].sh_size << '\n';
    //  char* iter = &buff[shdr[i].sh_offset];
    //  for (std::size_t j = 0; j < shdr[i].sh_size; ++j) {
    //    unsigned char c = iter[j];
    //    std::cout << (std::isprint(c) ? iter[j] : ' ') << ((j + 1) % 64 ? '\0' : '\n');
    //  }
    //  std::cout << '\n';
    //}

    std::cout << std::hex;
    //if (!std::strcmp(&shstrtab[shdr[i].sh_name], ".rela.plt")) {
    if (shdr[i].sh_type == SHT_RELA) {
      std::cout << &shstrtab[shdr[i].sh_name] << '\n';
      std::cout << "found .rela.plt, size: " << shdr[i].sh_size << '\n';
      //auto iter = reinterpret_cast<typename shdr_traits<std::iterator_traits<typename EhdrTraits::section_iterator>::value_type>::rela_iterator>(&buff[shdr[i].sh_offset]);
      //auto end = iter + shdr[i].sh_size / shdr[i].sh_entsize;
      //for (; iter != end; ++iter) {
      //  std::cout << iter->r_offset << '\n';
      //}
    }
    std::cout << std::dec;
    //std::cout << &shstrtab[shdr[i].sh_name] << '\n';
  }
  return 0;

  //using SegmentTraits = phdr_traits<std::iterator_traits<typename EhdrTraits::segment_iterator>::value_type>;

  //auto phdr = reinterpret_cast<typename EhdrTraits::segment_iterator>(&buff[ehdr->e_phoff]);
  //Accessor<std::iterator_traits<typename EhdrTraits::segment_iterator>::value_type> base{&buff[0], phdr, ehdr->e_phnum};
  //for (auto i = 0; i < ehdr->e_phnum; ++i) {
  //  switch (phdr[i].p_type) {
  //    case PT_DYNAMIC:
  //      DumpDynamic(base, reinterpret_cast<typename SegmentTraits::dynamic_iterator>(&base[phdr[i].p_vaddr]));
  //      break;
  //    default:
  //      break;
  //  }
  //}

  return 0;
}

void ELF::Dump(std::vector<char>& buff) {
  if (std::memcmp(&buff[0], ELFMAG, SELFMAG)) {
    std::cerr << "invalid ELF magic\n";
    return;
  }

  if (buff[EI_DATA] != ELFDATA2LSB) {
    std::cerr << "invalid ELF data endian\n";
    return;
  }

  if (buff[EI_CLASS] == ELFCLASS32) {
    std::cout << "dump ELF32 (little-endian)\n";
    DumpHeader(buff, reinterpret_cast<const Elf32_Ehdr*>(&buff[0]));
  } else if (buff[EI_CLASS] == ELFCLASS64) {
    std::cout << "dump ELF64 (little-endian)\n";
    DumpHeader(buff, reinterpret_cast<const Elf64_Ehdr*>(&buff[0]));
  } else {
    std::cerr << "invalid ELF class\n";
  }
}
