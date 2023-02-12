// tools/elfdump/main.cpp: Dump ELF files

#include <elf.h>

#include <algorithm>
#include <cstring>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <type_traits>
#include <vector>

#include "binlab/Config/Config.h"
#include "elf_stream.h"
#include "iterator_facade.h"

#include "versioning.h"

#ifdef __BIONIC__
#define SHT_GNU_HASH    0x6ffffff6
#define SHT_GNU_versym  0x6fffffff
#define SHT_GNU_verneed 0x6ffffffe
#define SHT_GNU_verdef  0x6ffffffd
#endif // !__BIONIC__

// Reference: https://lists.debian.org/lsb-spec/1999/12/msg00017.html
template <typename Verdaux, typename Verdef>
std::ostream& vddump(std::ostream& os, Verdef* verdef, const char* dynstr) {
  for (auto vd = verdef; vd; vd = reinterpret_cast<Verdef*>(reinterpret_cast<char*>(vd) + vd->vd_next)) {
    os << *vd;
    for (auto vda = reinterpret_cast<Verdaux*>(reinterpret_cast<char*>(vd) + vd->vd_aux);; vda = reinterpret_cast<Verdaux*>(reinterpret_cast<char*>(vda) + vda->vda_next)) {
      os << '\t' << &dynstr[vda->vda_name];
      if (!vda->vda_next) {
        break;
      }
    }
    os << '\n';
    if (!vd->vd_next) {
      break;
    }
  }
  return os;
}

// template <typename Verdaux, typename Verdef>
// std::ostream& vddump(std::ostream& os, Verdef* verdef, const char* dynstr) {
//   for (auto vd = verdef_iterator{verdef}; vd != nullptr; ++vd) {
//     os << *vd;
//     for (auto vda = const_verdaux_iterator{reinterpret_cast<Verdaux*>(reinterpret_cast<std::size_t>(std::addressof(*vd)) + vd->vd_aux)}; vda != nullptr; ++vda) {
//       os << '\t' << &dynstr[vda->vda_name];
//     }
//     os << '\n';
//   }
//   return os;
// }

template <typename Vernaux, typename Verneed>
std::ostream& vndump(std::ostream& os, Verneed* verneed, const char* dynstr) {
  for (auto vn = verneed; vn; vn = reinterpret_cast<Verneed*>(reinterpret_cast<char*>(vn) + vn->vn_next)) {
    os << *vn << &dynstr[vn->vn_file] << '\n';
    for (auto vna = reinterpret_cast<Vernaux*>(reinterpret_cast<char*>(vn) + vn->vn_aux);; vna = reinterpret_cast<Vernaux*>(reinterpret_cast<char*>(vna) + vna->vna_next)) {
      os << *vna << &dynstr[vna->vna_name] << '\n';
      if (!vna->vna_next) {
        break;
      }
    }
    os << '\n';
    if (!vn->vn_next) {
      break;
    }
  }
  return os;
}

// template <typename Vernaux, typename Verneed>
// std::ostream& vndump(std::ostream& os, Verneed* verneed, const char* dynstr) {
//   for (auto vn = const_verneed_iterator{verneed}; vn != nullptr; ++vn) {
//     os << *vn << &dynstr[vn->vn_file] << '\n';
//     for (auto vna = const_vernaux_iterator{reinterpret_cast<Vernaux*>(reinterpret_cast<std::size_t>(std::addressof(*vn)) + vn->vn_aux)}; vna != nullptr; ++vna) {
//       os << *vna << &dynstr[vna->vna_name] << '\n';
//     }
//     os << '\n';
//   }
//   return os;
// }

template <typename Verdaux, typename Verdef, typename Size>
Verdaux* find_vda(Verdef* vd, Size ndx) {
  for (auto ptr = reinterpret_cast<char*>(vd); (vd = reinterpret_cast<Verdef*>(ptr)); ptr += vd->vd_next) {
    if (vd->vd_ndx == ndx) {
      return reinterpret_cast<Verdaux*>(ptr + vd->vd_aux);
    }
    if (!vd->vd_next) {
      break;
    }
  }
  return nullptr;
}

template <typename Vernaux, typename Verneed, typename Size>
Vernaux* find_vna(Verneed* vn, Size ndx) {
  for (auto ptr1 = reinterpret_cast<char*>(vn); (vn = reinterpret_cast<Verneed*>(ptr1)); ptr1 += vn->vn_next) {
    Vernaux* vna;
    for (auto ptr2 = ptr1 + vn->vn_aux;; ptr2 += vna->vna_next) {
      vna = reinterpret_cast<Vernaux*>(ptr2);
      if (vna->vna_other == ndx) {
        return vna;
      }
      if (!vna->vna_next) {
        break;
      }
    }
    if (!vn->vn_next) {
      break;
    }
  }
  return nullptr;
}

template <typename Verdaux, typename Verdef, typename Versym>
std::ostream& vd_dump(std::ostream& os, Verdef* verdef, char* strtab, Versym versym) {
  if (versym == 0) {
    return os << "(local)";
  }

  if (versym == 1) {
    return os << "(global)";
  }

  if (auto vda = find_vda<Verdaux>(verdef, versym & 0x7fff)) {
    os << '@' << &strtab[vda->vda_name];
  }
  if (versym & 0x8000) {
    os << "(hidden)";
  }

  return os;
}

template <typename Vernaux, typename Verneed, typename Versym>
std::ostream& vn_dump(std::ostream& os, Verneed* verneed, char* strtab, Versym versym) {
  if (versym == 0) {
    return os << "(local)";
  }

  if (versym == 1) {
    return os << "(global)";
  }

  if (auto vna = find_vna<Vernaux>(verneed, versym)) {
    os << '@' << &strtab[vna->vna_name];
  }
  return os;
}

// Reference: https://flapenguin.me/elf-dt-hash
struct alignas(4) elf_hash_table {
  std::uint32_t nbucket;
  std::uint32_t nchain;
};

static constexpr std::uint32_t elf_hash(const std::uint8_t* name) {
  std::uint32_t h = 0, g = 0;
  for (; *name; name++) {
    h = (h << 4) + *name;
    if ((g = h & 0xf0000000)) {
      h ^= g >> 24;
    }
    h &= ~g;
  }
  return h;
}

template <typename Verdaux, typename Verdef, typename Vernaux, typename Verneed, typename Sym>
std::ostream& elf_hash_dump(std::ostream& os, Verdef* verdef, Verneed* verneed, decltype(Vernaux::vna_other)* versym, elf_hash_table* table, Sym* symtab, char* strtab) {
  auto bucket = reinterpret_cast<std::uint32_t*>(table + 1);
  auto chain = reinterpret_cast<std::uint32_t*>(&bucket[table->nbucket]);

  for (std::size_t i = 0; i < table->nbucket; ++i) {
    if (!bucket[i]) { continue; }  // empty bucket

    for (auto index = bucket[i]; index; index = chain[index]) {
      auto name = &strtab[symtab[index].st_name];
      auto hash = elf_hash(reinterpret_cast<std::uint8_t*>(name));

      os << std::setw(4) << (hash % table->nbucket) << ' ';
      os << std::setw(8) << std::right << std::setfill('0') << hash << std::left << std::setfill(' ') << ' ';

      os << symtab[index] << name;
      if (symtab[index].st_value) {
        vd_dump<Verdaux>(os, verdef, strtab, versym[index]);
      } else {
        vn_dump<Vernaux>(os, verneed, strtab, versym[index]);
      }
      os << '\n';
    }
    os << '\n';
  }
  return os;
}

// Reference: https://flapenguin.me/elf-dt-gnu-hash
struct alignas(4) gnu_hash_table {
  std::uint32_t nbuckets;
  std::uint32_t symoffset;
  std::uint32_t bloom_size;
  std::uint32_t bloom_shift;
};

static constexpr std::uint32_t gnu_hash(const std::uint8_t* name) {
  std::uint32_t h = 5381;
  for (; *name; name++) {
    h = (h << 5) + h + *name;
  }
  return h;
}

template <typename Verdaux, typename Verdef, typename Vernaux, typename Verneed, typename Bloom, typename Sym>
std::ostream& gnu_hash_dump(std::ostream& os, Verdef* verdef, Verneed* verneed, decltype(Vernaux::vna_other)* versym, gnu_hash_table* table, Sym* symtab, char* strtab) {
  auto bloom = reinterpret_cast<Bloom*>(table + 1);
  auto buckets = reinterpret_cast<std::uint32_t*>(&bloom[table->bloom_size]);
  auto chain = &buckets[table->nbuckets];

  for (std::size_t i = 0; i < table->symoffset; ++i) {
    os << symtab[i] << &strtab[symtab[i].st_name];
    if (symtab[i].st_value) {
      vd_dump<Verdaux>(os, verdef, strtab, versym[i]);
    } else {
      vn_dump<Vernaux>(os, verneed, strtab, versym[i]);
    }
    os << '\n';
  }
  os << '\n';

  for (std::size_t i = 0; i < table->nbuckets; ++i) {
    if (!buckets[i]) { continue; }  // empty bucket

    for (std::size_t j = buckets[i];; ++j) {
      auto name = &strtab[symtab[j].st_name];
      auto hash = gnu_hash(reinterpret_cast<std::uint8_t*>(name));

      os << std::setw(4) << (hash % table->nbuckets) << ' ';
      os << std::setw(8) << std::right << std::setfill('0') << hash << ':' << std::setw(8) << chain[j - table->symoffset] << std::left << std::setfill(' ') << ' ';

      os << symtab[j] << name;
      if (symtab[j].st_value) {
        vd_dump<Verdaux>(os, verdef, strtab, versym[j]);
      } else {
        vn_dump<Vernaux>(os, verneed, strtab, versym[j]);
      }
      os << '\n';

      if (chain[j - table->symoffset] & 1) {
        break;
      }
    }
    os << '\n';
  }
  return os;
}

template <typename Verdaux, typename Verdef, typename Vernaux, typename Verneed, typename Bloom, typename Sym, typename Shdr>
std::ostream& sh_sym_dump(std::ostream& os, char* base, Shdr* first, Shdr* last, char* shstrtab) {
  auto iter = std::find_if(first, last, [](const Shdr& s) { return s.sh_type == SHT_GNU_versym; });
  if (iter != last) {
    os << &shstrtab[iter->sh_name] << '\n';

    auto versym = reinterpret_cast<decltype(Vernaux::vna_other)*>(&base[iter->sh_offset]);
    auto dynsym = reinterpret_cast<Sym*>(&base[first[iter->sh_link].sh_offset]);
    auto dynstr = reinterpret_cast<char*>(&base[first[first[iter->sh_link].sh_link].sh_offset]);
    //auto size = iter->sh_size / iter->sh_entsize;

    iter = std::find_if(first, last, [](const Shdr& s) { return s.sh_type == SHT_GNU_verneed; });
    auto verneed = (iter != last) ? reinterpret_cast<Verneed*>(&base[iter->sh_offset]) : nullptr;

    iter = std::find_if(first, last, [](const Shdr& s) { return s.sh_type == SHT_GNU_verdef; });
    auto verdef = (iter != last) ? reinterpret_cast<Verdef*>(&base[iter->sh_offset]) : nullptr;
    
    vddump<Verdaux>(os, verdef, dynstr);
    os << '\n';
    vndump<Vernaux>(os, verneed, dynstr);

    iter = std::find_if(first, last, [](const Shdr& s) { return s.sh_type == SHT_GNU_HASH; });
    if (iter != last) {
      auto table = reinterpret_cast<gnu_hash_table*>(&base[iter->sh_offset]);
      gnu_hash_dump<Verdaux, Verdef, Vernaux, Verneed, Bloom>(os, verdef, verneed, versym, table, dynsym, dynstr);
    }

    iter = std::find_if(first, last, [](const Shdr& s) { return s.sh_type == SHT_HASH; });
    if (iter != last) {
      auto table = reinterpret_cast<elf_hash_table*>(&base[iter->sh_offset]);
      elf_hash_dump<Verdaux, Verdef, Vernaux, Verneed>(os, verdef, verneed, versym, table, dynsym, dynstr);
    }
  }
  return os;
}

std::ostream& sh64dump(std::ostream& os, char* base) {
  auto& ehdr = reinterpret_cast<Elf64_Ehdr&>(base[0]);
  auto first = reinterpret_cast<Elf64_Shdr*>(&base[ehdr.e_shoff]);
  auto last = first + ehdr.e_shnum;

  auto shstrtab = reinterpret_cast<char*>(&base[first[ehdr.e_shstrndx].sh_offset]);
  sh_sym_dump<Elf64_Verdaux, Elf64_Verdef, Elf64_Vernaux, Elf64_Verneed, std::uint64_t, Elf64_Sym, Elf64_Shdr>(os, base, first, last, shstrtab);
  return os;
}

std::ostream& sh32dump(std::ostream& os, char* base) {
  auto& ehdr = reinterpret_cast<Elf32_Ehdr&>(base[0]);
  auto first = reinterpret_cast<Elf32_Shdr*>(&base[ehdr.e_shoff]);
  auto last = first + ehdr.e_shnum;

  auto shstrtab = reinterpret_cast<char*>(&base[first[ehdr.e_shstrndx].sh_offset]);
  sh_sym_dump<Elf32_Verdaux, Elf32_Verdef, Elf32_Vernaux, Elf32_Verneed, std::uint32_t, Elf32_Sym, Elf32_Shdr>(os, base, first, last, shstrtab);
  return os;
}

template <typename Verdaux, typename Verdef, typename Vernaux, typename Verneed, typename Bloom, typename Sym, typename Dyn, typename Phdr>
std::ostream& ph_sym_dump(std::ostream& os, char* base, Dyn* dyn, Phdr* first, Phdr* last) {
  elf_hash_table* elf_hash = nullptr;

  char* strtab = nullptr;
  Sym* symtab;
  //decltype(Dyn::d_un.d_val) strsz;
  //decltype(Dyn::d_un.d_val) syment;

  gnu_hash_table* gnu_hash = nullptr;

  decltype(Vernaux::vna_other)* versym;
  Verdef* verdef = nullptr;
  //decltype(Dyn::d_un.d_val) verdefnum;
  Verneed* verneed = nullptr;
  //decltype(Dyn::d_un.d_val) verneednum;

  for (; dyn->d_tag != DT_NULL; ++dyn) {
    auto addr = dyn->d_un.d_ptr;
    auto iter = std::find_if(first, last, [addr](const Phdr& phdr) { return phdr.p_vaddr <= addr && addr < phdr.p_vaddr + phdr.p_filesz; });
    addr = (iter != last ? (addr - iter->p_vaddr + iter->p_offset) : 0);
    switch (dyn->d_tag) {
      //case DT_NULL:
      //  return os;
      case DT_HASH:
        elf_hash = reinterpret_cast<elf_hash_table*>(&base[addr]);
        break;
      case DT_STRTAB:
        strtab = &base[addr];
        break;
      case DT_SYMTAB:
        symtab = reinterpret_cast<Sym*>(&base[addr]);
        break;
      //case DT_STRSZ:
      //  strsz = dyn->d_un.d_val;
      //  break;
      //case DT_SYMENT:
      //  syment = dyn->d_un.d_val;
      //  break;
      case DT_GNU_HASH:
        gnu_hash = reinterpret_cast<gnu_hash_table*>(&base[addr]);
        break;
      case DT_VERSYM:
        versym = reinterpret_cast<decltype(Vernaux::vna_other)*>(&base[addr]);
        break;
      case DT_VERDEF:
        verdef = reinterpret_cast<Verdef*>(&base[addr]);
        break;
      //case DT_VERDEFNUM:
      //  verdefnum = dyn->d_un.d_val;
      //  break;
      case DT_VERNEED:
        verneed = reinterpret_cast<Verneed*>(&base[addr]);
        break;
      //case DT_VERNEEDNUM:
      //  verneednum = dyn->d_un.d_val;
      //  break;
      default:
        break;
    }
  }

  if (verdef) {
    vddump<Verdaux>(os, verdef, strtab) << '\n';
  }
  if (verneed) {
    vndump<Vernaux>(os, verneed, strtab) << '\n';
  }
  return os;

  if (gnu_hash) {
    gnu_hash_dump<Verdaux, Verdef, Vernaux, Verneed, Bloom>(os, verdef, verneed, versym, gnu_hash, symtab, strtab);
  }

  if (elf_hash) {
    elf_hash_dump<Verdaux, Verdef, Vernaux, Verneed>(os, verdef, verneed, versym, elf_hash, symtab, strtab);
  }
  return os;
}

std::ostream& ph64dump(std::ostream& os, char* base) {
  auto& ehdr = reinterpret_cast<Elf64_Ehdr&>(base[0]);
  auto first = reinterpret_cast<Elf64_Phdr*>(&base[ehdr.e_phoff]);
  auto last = first + ehdr.e_phnum;

  auto iter = std::find_if(first, last, [](const Elf64_Phdr& s) { return s.p_type == PT_DYNAMIC; });
  if (iter == last) {
    os << "couldn't find PT_DYNAMIC\n";
    return os;
  }

  auto dyn = reinterpret_cast<Elf64_Dyn*>(&base[iter->p_offset]);
  ph_sym_dump<Elf64_Verdaux, Elf64_Verdef, Elf64_Vernaux, Elf64_Verneed, std::uint64_t, Elf64_Sym>(os, base, dyn, first, last);

  return os;
}

std::ostream& ph32dump(std::ostream& os, char* base) {
  auto& ehdr = reinterpret_cast<Elf32_Ehdr&>(base[0]);
  auto first = reinterpret_cast<Elf32_Phdr*>(&base[ehdr.e_phoff]);
  auto last = first + ehdr.e_phnum;

  auto iter = std::find_if(first, last, [](const Elf32_Phdr& s) { return s.p_type == PT_DYNAMIC; });
  if (iter == last) {
    os << "couldn't find PT_DYNAMIC\n";
    return os;
  }

  auto dyn = reinterpret_cast<Elf32_Dyn*>(&base[iter->p_offset]);
  ph_sym_dump<Elf32_Verdaux, Elf32_Verdef, Elf32_Vernaux, Elf32_Verneed, std::uint32_t, Elf32_Sym>(os, base, dyn, first, last);

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
      std::cout << std::hex << std::left;
      switch (buff[EI_CLASS]) {
        case ELFCLASS64:
          //sh64dump(std::cout, &buff[0]);
          ph64dump(std::cout, &buff[0]);
          break;
        case ELFCLASS32:
          //sh32dump(std::cout, &buff[0]);
          ph32dump(std::cout, &buff[0]);
          break;
        default:
          std::cout << "invalid EI_CLASS\n";
          break;
      }
      std::cout << std::right;
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
