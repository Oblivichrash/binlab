// tools/elfdump/main.cpp: Dump Binary files

#include <elf.h>

#include <cstring>
#include <fstream>
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

template <typename Sym, typename Shdr, typename Ehdr>
std::ostream& symdump(std::ostream& os, void* base) {
  auto buff = static_cast<std::uint8_t*>(base);

  auto& ehdr = reinterpret_cast<Ehdr&>(buff[0]);
  auto shdr = reinterpret_cast<Shdr*>(&buff[ehdr.e_shoff]);

  auto shstrtab = reinterpret_cast<char*>(&buff[shdr[ehdr.e_shstrndx].sh_offset]);
  for (std::size_t i = 0; i < ehdr.e_shnum; ++i) {
    if (shdr[i].sh_type == SHT_SYMTAB/* || shdr[i].sh_type == SHT_DYNSYM*/) {
      os << &shstrtab[shdr[i].sh_name] << '\n';
      // os << &shstrtab[shdr[shdr[i].sh_link].sh_name] << '\n';

      auto symtab = reinterpret_cast<Sym*>(&buff[shdr[i].sh_offset]);
      auto strtab = reinterpret_cast<char*>(&buff[shdr[shdr[i].sh_link].sh_offset]);

      for (std::size_t j = 0; j < shdr[i].sh_size / shdr[i].sh_entsize; ++j) {
        os << &strtab[symtab[j].st_name] << '\n';
      }
      os << '\n';
    }
  }
  os << '\n';

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
        gnuhash_dump<std::uint64_t, Elf64_Sym, Elf64_Shdr, Elf64_Ehdr>(std::cout, buff.data());
      } else if (buff[EI_CLASS] == ELFCLASS32) {
        gnuhash_dump<std::uint32_t, Elf32_Sym, Elf32_Shdr, Elf32_Ehdr>(std::cout, buff.data());
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
