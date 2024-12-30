// elfdump.cpp

#include "elfdump.h"

#include <cstdio>
#include <cstring>
#include <iostream>
#include <iomanip>

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

int binlab::ELF::dump_sym64(char *buff, [[maybe_unused]] std::size_t size) {
  if (std::memcmp(buff, ELFMAG, SELFMAG)) {
    return 1;
  }
  if (buff[EI_CLASS] != ELFCLASS64) {
    return 1;
  }

  auto& os = std::cout;

  auto& ehdr = reinterpret_cast<const Elf64_Ehdr&>(buff[0]);
  auto shdr = reinterpret_cast<const Elf64_Shdr*>(&buff[ehdr.e_shoff]);
  //auto shstr = &buff[shdr[ehdr.e_shstrndx].sh_offset];
  for (auto iter = shdr; iter != shdr + ehdr.e_shnum; ++iter) {
    //os << &shstr[iter->sh_name] << '\n';
    switch (iter->sh_type) {
      case SHT_HASH: {
        auto hash = &buff[iter->sh_offset];
        auto symtab = reinterpret_cast<const Elf64_Sym*>(&buff[shdr[iter->sh_link].sh_offset]);
        auto strtab = &buff[shdr[shdr[iter->sh_link].sh_link].sh_offset];
        dump_hash(os, sysv_hash_table{symtab, strtab, hash}, strtab);
        break;
      }
      case SHT_DYNAMIC: {
        auto strtab = &buff[shdr[iter->sh_link].sh_offset];
        auto dyn = reinterpret_cast<const Elf64_Dyn*>(&buff[iter->sh_offset]);
        os << "dyn offset: " << std::hex << iter->sh_offset << '\n';
        dump_dynamic(os, dyn, strtab);
        break;
      }
      //case SHT_GNU_HASH: {
      //  auto hash = &buff[iter->sh_offset];
      //  auto symtab = reinterpret_cast<const Elf64_Sym*>(&buff[shdr[iter->sh_link].sh_offset]);
      //  auto strtab = &buff[shdr[shdr[iter->sh_link].sh_link].sh_offset];
      //  dump_hash(os, gnu_hash_table{symtab, strtab, hash}, strtab);
      //  break;
      //}
      default:
        break;
    }
  }
  return 0;
}
