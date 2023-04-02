// tools/bindump/main.cpp: Dump Binary files

#include <cstddef>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <vector>

#include "binlab/Config/Config.h"
#include "binlab/BinaryFormat/COFF.h"
#include "binlab/BinaryFormat/ELF.h"

#include "symbols.h"

using namespace binlab::COFF;
using namespace binlab::ELF;

std::ostream& dump_coff(std::ostream& os, char* buff) {
  auto& Dos = reinterpret_cast<IMAGE_DOS_HEADER&>(buff[0]);
  if (Dos.e_magic != IMAGE_DOS_SIGNATURE) {
    return os << "invalid DOS magic\n";
  }
  
  auto& Nt = reinterpret_cast<IMAGE_NT_HEADERS64&>(buff[Dos.e_lfanew]);
  if (Nt.Signature != IMAGE_NT_SIGNATURE || Nt.OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR64_MAGIC) {
    return os << "invalid NT type\n";
  }

  auto first = reinterpret_cast<IMAGE_SECTION_HEADER*>(reinterpret_cast<std::size_t>(&Nt) + offsetof(std::decay_t<decltype(Nt)>, OptionalHeader) + Nt.FileHeader.SizeOfOptionalHeader);
  auto last = first + Nt.FileHeader.NumberOfSections;
  os << std::hex;

  for (auto iter = first; iter != last; ++iter) {
    if (iter->Characteristics & IMAGE_SCN_MEM_EXECUTE) {
      os << std::string{reinterpret_cast<char*>(iter->Name), sizeof(iter->Name)} << '\t';
      os << "offset: " << iter->PointerToRawData << '\t';
      os << "size: " << iter->SizeOfRawData << '\t';
      os << "Characteristics: " << iter->Characteristics << '\n';
    }
  }

  return os;
}

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

  const char* name = "_IO_fread";
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
  auto shstr = &buff[shdr[ehdr.e_shstrndx].sh_offset];
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
    std::cout << "Usage: " << argv[0] << " <file>\n";
    return 1;
  }

  if (std::ifstream is{argv[1], std::ios::binary | std::ios::ate}) {
    const auto count = static_cast<std::size_t>(is.tellg());
    std::vector<char> buff(count);
    if (is.seekg(0, std::ios::beg).read(&buff[0], count)) {
      switch (buff[0]) {
        case 'M':
          dump_coff(std::cout, &buff[0]);
          break;
        case 0x7f:  // ELFMAG0
          dump_elf(std::cout, &buff[0]);
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
