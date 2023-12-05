// tools/binlab-dumpbin/binlab-dumpbin.cpp: Dump Binary files

#include <cstddef>
#include <cstring>
#include <fstream>
#include <iterator>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include "binlab/Config.h"
#include "binlab/BinaryFormat/COFF.h"
#include "binlab/BinaryFormat/ELF.h"
#include "section_header.h"
#include "export_directory.h"
#include "import_descriptor.h"
#include "resource_directory.h"
#include "base_relocation.h"

#include "symbols.h"

using namespace binlab::ELF;

std::ostream& dump_coff(std::ostream& os, char* buff) {
  using namespace binlab::COFF;
  os << std::hex;

  auto& Dos = reinterpret_cast<IMAGE_DOS_HEADER&>(buff[0]);
  if (Dos.e_magic != IMAGE_DOS_SIGNATURE) {
    return os << "invalid DOS magic\n";
  }
  
  auto& Nt = reinterpret_cast<IMAGE_NT_HEADERS64&>(buff[Dos.e_lfanew]);
  if (Nt.Signature != IMAGE_NT_SIGNATURE) {
    return os << "invalid NT type\n";
  }

  constexpr std::size_t index = IMAGE_DIRECTORY_ENTRY_BASERELOC;
  std::size_t vaddr;
  switch (Nt.OptionalHeader.Magic) {
    case IMAGE_NT_OPTIONAL_HDR64_MAGIC:
      vaddr = Nt.OptionalHeader.DataDirectory[index].VirtualAddress;
      break;
    case IMAGE_NT_OPTIONAL_HDR32_MAGIC:
      vaddr = reinterpret_cast<IMAGE_NT_HEADERS32&>(Nt).OptionalHeader.DataDirectory[index].VirtualAddress;
      break;
    default:
      throw std::runtime_error{"invalid optional header magic"};
      break;
  }

  auto sections = begin(Nt);
  for (std::size_t i = 0; i < Nt.FileHeader.NumberOfSections; ++i) {
    //os << sections[i] << '\n';

    auto offset = vaddr - sections[i].VirtualAddress;
    if (vaddr < sections[i].VirtualAddress || offset > sections[i].Misc.VirtualSize) {
      continue;
    }
    auto base = buff + sections[i].PointerToRawData;

    section_ref section(sections[i].VirtualAddress, base);

    //dump(os, base, vaddr, reinterpret_cast<IMAGE_RESOURCE_DIRECTORY*>(&base[offset]), 0);
    //dump(os, sections[i], base, reinterpret_cast<IMAGE_RESOURCE_DIRECTORY&>(base[offset]));
    dump(os, sections[i], base, reinterpret_cast<IMAGE_BASE_RELOCATION&>(base[offset]));
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
  if (argc < 3) {
    std::cerr << argv[0] << " Version " << BINLAB_VERSION_MAJOR << '.' << BINLAB_VERSION_MINOR << '\n';
    std::cout << "Usage: " << argv[0] << " <input> <output>\n";
    return 1;
  }

  std::ifstream is{argv[1], std::ios::binary | std::ios::ate};
  std::ofstream os{argv[2], std::ios::binary};
  is.exceptions(std::ifstream::failbit);
  os.exceptions(std::ofstream::failbit);

  const auto& count = is.tellg();
  if (!count) {
    throw std::runtime_error{"empty file"};
  }
  std::vector<char> buff(count);
  is.seekg(0, std::ios::beg).read(&buff[0], count);

  switch (buff[0]) {
    case 'M':
      dump_coff(std::cout, &buff[0]);
      break;
    case 0x7f:  // ELFMAG0
      //dump_elf(std::cout, &buff[0]);
      break;
    default:
      break;
  }
  os.write(&buff[0], buff.size());

  return 0;
} catch (const std::exception& e) {
  std::cerr << "error: " << e.what() << '\n';
  return 1;
}
