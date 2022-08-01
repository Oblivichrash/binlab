// tools/elfdump/main.cpp: Dump Binary files

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

std::ostream& operator<<(std::ostream& os, const Elf32_Sym& sym) {
  os << "name: " << std::setw(8) << sym.st_name;
  os << "info: " << std::setw(4) << unsigned{sym.st_info};
  os << "other: " << std::setw(4) << unsigned{sym.st_other};
  os << "shndx: " << std::setw(8) << sym.st_shndx;
  os << "value: " << std::setw(8) << sym.st_value;
  os << "size: " << std::setw(8) << sym.st_size;
  return os;
}

std::ostream& operator<<(std::ostream& os, const Elf64_Sym& sym) {
  os << "name: " << std::setw(8) << sym.st_name;
  os << "info: " << std::setw(4) << unsigned{sym.st_info};
  os << "other: " << std::setw(4) << unsigned{sym.st_other};
  os << "shndx: " << std::setw(8) << sym.st_shndx;
  os << "value: " << std::setw(8) << sym.st_value;
  os << "size: " << std::setw(8) << sym.st_size;
  return os;
}

std::ostream& operator<<(std::ostream& os, const Elf32_Verdef& verdef) {
  os << "version: " << std::setw(4) << verdef.vd_version;
  os << "flags: " << std::setw(4) << verdef.vd_flags;
  os << "ndx: " << std::setw(4) << verdef.vd_ndx;
  os << "cnt: " << std::setw(4) << verdef.vd_cnt;
  os << "hash: " << std::setw(8) << verdef.vd_hash;
  os << "aux: " << std::setw(4) << verdef.vd_aux;
  os << "next: " << std::setw(4) << verdef.vd_next;
  return os;
}

std::ostream& operator<<(std::ostream& os, const Elf64_Verdef& verdef) {
  os << "version: " << std::setw(4) << verdef.vd_version;
  os << "flags: " << std::setw(4) << verdef.vd_flags;
  os << "ndx: " << std::setw(4) << verdef.vd_ndx;
  os << "cnt: " << std::setw(4) << verdef.vd_cnt;
  os << "hash: " << std::setw(8) << verdef.vd_hash;
  os << "aux: " << std::setw(4) << verdef.vd_aux;
  os << "next: " << std::setw(4) << verdef.vd_next;
  return os;
}

std::ostream& operator<<(std::ostream& os, const Elf32_Verneed& verneed) {
  os << "version: " << std::setw(4) << verneed.vn_version;
  os << "cnt: " << std::setw(4) << verneed.vn_cnt;
  os << "file: " << std::setw(6) << verneed.vn_file;
  os << "aux: " << std::setw(4) << verneed.vn_aux;
  os << "next: " << std::setw(4) << verneed.vn_next;
  return os;
}

std::ostream& operator<<(std::ostream& os, const Elf64_Verneed& verneed) {
  os << "version: " << std::setw(4) << verneed.vn_version;
  os << "cnt: " << std::setw(4) << verneed.vn_cnt;
  os << "file: " << std::setw(6) << verneed.vn_file;
  os << "aux: " << std::setw(4) << verneed.vn_aux;
  os << "next: " << std::setw(4) << verneed.vn_next;
  return os;
}

std::ostream& operator<<(std::ostream& os, const Elf32_Vernaux& vernaux) {
  os << "hash: " << std::setw(8) << vernaux.vna_hash;
  os << "flags: " << std::setw(4) << vernaux.vna_flags;
  os << "other: " << std::setw(4) << vernaux.vna_other;
  os << "name: " << std::setw(6) << vernaux.vna_name;
  os << "next: " << std::setw(4) << vernaux.vna_next;
  return os;
}


std::ostream& operator<<(std::ostream& os, const Elf64_Vernaux& vernaux) {
  os << "hash: " << std::setw(8) << vernaux.vna_hash;
  os << "flags: " << std::setw(4) << vernaux.vna_flags;
  os << "other: " << std::setw(4) << vernaux.vna_other;
  os << "name: " << std::setw(6) << vernaux.vna_name;
  os << "next: " << std::setw(4) << vernaux.vna_next;
  return os;
}

struct alignas(4) gnu_hash_table {
  std::uint32_t nbuckets;
  std::uint32_t symoffset;
  std::uint32_t bloom_size;
  std::uint32_t bloom_shift;
};

std::uint32_t gnu_hash(const std::uint8_t* name) {
  std::uint32_t h = 5381;
  for (; *name; name++) {
    h = (h << 5) + h + *name;
  }
  return h;
}

template <typename Bloom, typename Sym, typename Shdr, typename Ehdr>
std::ostream& gnuhash_dump(std::ostream& os, void* base) {
  auto buff = static_cast<std::uint8_t*>(base);

  auto& ehdr = reinterpret_cast<Ehdr&>(buff[0]);
  auto shdr = reinterpret_cast<Shdr*>(&buff[ehdr.e_shoff]);

  auto shstrtab = reinterpret_cast<char*>(&buff[shdr[ehdr.e_shstrndx].sh_offset]);
  for (std::size_t i = 0; i < ehdr.e_shnum; ++i) {
    if (shdr[i].sh_type == SHT_GNU_HASH) {
      os << &shstrtab[shdr[i].sh_name] << '\n';
      //os << &shstrtab[shdr[shdr[i].sh_link].sh_name] << '\n';
      //os << &shstrtab[shdr[shdr[shdr[i].sh_link].sh_link].sh_name] << '\n';
      
      auto table = reinterpret_cast<gnu_hash_table*>(&buff[shdr[i].sh_offset]);
      auto bloom = reinterpret_cast<Bloom*>(table + 1);
      auto buckets = reinterpret_cast<std::uint32_t*>(&bloom[table->bloom_size]);
      auto chain = &buckets[table->nbuckets];

      auto dynsym = reinterpret_cast<Sym*>(&buff[shdr[shdr[i].sh_link].sh_offset]);
      auto dynstr = reinterpret_cast<char*>(&buff[shdr[shdr[shdr[i].sh_link].sh_link].sh_offset]);

      for (std::size_t j = 0; j < shdr[shdr[i].sh_link].sh_size / shdr[shdr[i].sh_link].sh_entsize; ++j) {
        auto name = &dynstr[dynsym[j].st_name];
        
        auto namehash = gnu_hash(reinterpret_cast<std::uint8_t*>(name));
        
        auto word = bloom[(namehash / (sizeof(Bloom) * 8)) % table->bloom_size];
        Bloom mask = 0 | (Bloom)1 << (namehash % (sizeof(Bloom) * 8)) | (Bloom)1 << ((namehash >> table->bloom_shift) % (sizeof(Bloom) * 8));

        if ((word & mask) != mask) {
          os << "not in bloom filter: " << name << '\n';
          continue;
        }
    
        std::uint32_t symix = buckets[namehash % table->nbuckets];
        if (symix < table->symoffset) {
          os << "invalid symbol index: " << name << '\n';
          continue;
        }
        
        for (std::size_t k = symix;; ++k) {
          auto symbol_name = &dynstr[dynsym[k].st_name];
          auto hash = chain[k - table->symoffset];
          if ((namehash | 1) == (hash | 1) && !std::strcmp(name, symbol_name)) {
            os << "found: " << name << '\n';
            break;
          }
          if (hash & 1) {
            os << "not found: " << name << '\n';
            break;
          }
        }
      }
    }
  }
  os << '\n';

  return os;
}

// Reference: https://lists.debian.org/lsb-spec/1999/12/msg00017.html
Elf32_Verneed* next(Elf32_Verneed* vn) { return vn->vn_next ? reinterpret_cast<Elf32_Verneed*>(reinterpret_cast<char*>(vn) + vn->vn_next) : nullptr; }
Elf64_Verneed* next(Elf64_Verneed* vn) { return vn->vn_next ? reinterpret_cast<Elf64_Verneed*>(reinterpret_cast<char*>(vn) + vn->vn_next) : nullptr; }
Elf32_Vernaux* next(Elf32_Vernaux* vna) { return vna->vna_next ? reinterpret_cast<Elf32_Vernaux*>(reinterpret_cast<char*>(vna) + vna->vna_next) : nullptr; }
Elf64_Vernaux* next(Elf64_Vernaux* vna) { return vna->vna_next ? reinterpret_cast<Elf64_Vernaux*>(reinterpret_cast<char*>(vna) + vna->vna_next) : nullptr; }

Elf32_Verdef* next(Elf32_Verdef* vd) { return vd->vd_next ? reinterpret_cast<Elf32_Verdef*>(reinterpret_cast<char*>(vd) + vd->vd_next) : nullptr; }
Elf64_Verdef* next(Elf64_Verdef* vd) { return vd->vd_next ? reinterpret_cast<Elf64_Verdef*>(reinterpret_cast<char*>(vd) + vd->vd_next) : nullptr; }
Elf32_Verdaux* next(Elf32_Verdaux* vda) { return vda->vda_next ? reinterpret_cast<Elf32_Verdaux*>(reinterpret_cast<char*>(vda) + vda->vda_next) : nullptr; }
Elf64_Verdaux* next(Elf64_Verdaux* vda) { return vda->vda_next ? reinterpret_cast<Elf64_Verdaux*>(reinterpret_cast<char*>(vda) + vda->vda_next) : nullptr; }

template <typename Verdaux, typename Verdef, typename Vernaux, typename Verneed, typename Sym, typename Shdr, typename Ehdr>
std::ostream& symdump(std::ostream& os, void* base) {
  auto buff = static_cast<std::uint8_t*>(base);

  auto& ehdr = reinterpret_cast<Ehdr&>(buff[0]);
  auto shdr = reinterpret_cast<Shdr*>(&buff[ehdr.e_shoff]);
  auto shstrtab = reinterpret_cast<char*>(&buff[shdr[ehdr.e_shstrndx].sh_offset]);

  Shdr *first = shdr, *last = shdr + ehdr.e_shnum, *iter;

  os << std::left;

  iter = std::find_if(shdr, last, [](const Shdr& s) { return s.sh_type == SHT_GNU_versym; });
  if (iter != last) {
    os << &shstrtab[iter->sh_name] << '\n';
    os << &shstrtab[shdr[iter->sh_link].sh_name] << '\n';
    os << &shstrtab[shdr[shdr[iter->sh_link].sh_link].sh_name] << '\n';

    auto versym = reinterpret_cast<decltype(Vernaux::vna_other)*>(&buff[iter->sh_offset]);
    auto dynsym = reinterpret_cast<Sym*>(&buff[shdr[iter->sh_link].sh_offset]);
    auto dynstr = reinterpret_cast<char*>(&buff[shdr[shdr[iter->sh_link].sh_link].sh_offset]);
    auto size = iter->sh_size / iter->sh_entsize;

    iter = std::find_if(first, last, [](const Shdr& s) { return s.sh_type == SHT_GNU_verneed; });
    auto verneed = (iter != last) ? reinterpret_cast<Verneed*>(&buff[iter->sh_offset]) : nullptr;

    iter = std::find_if(shdr, last, [](const Shdr& s) { return s.sh_type == SHT_GNU_verdef; });
    auto verdef = (iter != last) ? reinterpret_cast<Verdef*>(&buff[iter->sh_offset]) : nullptr;

    for (std::size_t j = 0; j < size; ++j) {
      os << dynsym[j] << ' ' << &dynstr[dynsym[j].st_name];
      if (!dynsym[j].st_value && !dynsym[j].st_size && !dynsym[j].st_shndx) {
        for (auto vn = verneed; vn; vn = next(vn)) {
          if (vn->vn_aux) {
            for (auto vna = reinterpret_cast<Vernaux*>(reinterpret_cast<char*>(vn) + vn->vn_aux); vna; vna = next(vna)) {
              if (vna->vna_other == (versym[j] & 0x7fff)) {
                os << '@' << &dynstr[vna->vna_name];
              }
            }
          }
        }
      } else {
        for (auto vd = verdef; vd; vd = next(vd)) {
          if (vd->vd_ndx == (versym[j] & 0x7fff)) {
            auto vda = reinterpret_cast<Verdaux*>(reinterpret_cast<char*>(vd) + vd->vd_aux);
            os << '@' << &dynstr[vda->vda_name];
          }
        }
      }
      os << '\n';
    }
    os << '\n';

    for (auto vd = verdef; vd; vd = next(vd)) {
      os << *vd;
      for (auto vda = reinterpret_cast<Verdaux*>(reinterpret_cast<char*>(vd) + vd->vd_aux); vda; vda = next(vda)) {
        os << '\t' << &dynstr[vda->vda_name];
      }
      os << '\n';
    }
    os << '\n';

    for (auto vn = verneed; vn; vn = next(vn)) {
      os << *vn << &dynstr[vn->vn_file] << '\n';
      if (vn->vn_aux) {
        for (auto vna = reinterpret_cast<Vernaux*>(reinterpret_cast<char*>(vn) + vn->vn_aux); vna; vna = next(vna)) {
          os << *vna << &dynstr[vna->vna_name] << '\n';
        }
      }
      os << '\n';
    }
  }

  os << std::right;
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
      //memdump(std::cout, buff.data(), 16);
      if (buff[EI_CLASS] == ELFCLASS64) {
        symdump<Elf64_Verdaux, Elf64_Verdef, Elf64_Vernaux, Elf64_Verneed, Elf64_Sym, Elf64_Shdr, Elf64_Ehdr>(std::cout, buff.data());
        //gnuhash_dump<std::uint64_t, Elf64_Sym, Elf64_Shdr, Elf64_Ehdr>(std::cout, buff.data());
        //verddump<Elf64_Verdaux, Elf64_Verdef, Elf64_Shdr, Elf64_Ehdr>(std::cout, buff.data());
        //verndump<Elf64_Vernaux, Elf64_Verneed, Elf64_Shdr, Elf64_Ehdr>(std::cout, buff.data());
      } else if (buff[EI_CLASS] == ELFCLASS32) {
        symdump<Elf32_Verdaux, Elf32_Verdef, Elf32_Vernaux, Elf32_Verneed, Elf32_Sym, Elf32_Shdr, Elf32_Ehdr>(std::cout, buff.data());
        //symdump<Elf32_Sym, Elf32_Shdr, Elf32_Ehdr>(std::cout, buff.data());
        //gnuhash_dump<std::uint32_t, Elf32_Sym, Elf32_Shdr, Elf32_Ehdr>(std::cout, buff.data());
        //verddump<Elf32_Verdaux, Elf32_Verdef, Elf32_Shdr, Elf32_Ehdr>(std::cout, buff.data());
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
