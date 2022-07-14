// tools/elfdump/main.cpp: Dump Binary files

#include <elf.h>

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

template <typename T>
class const_symtab_iterator {
 public:
  using value_type = std::pair<const char*, const T&>;
  using difference_type = std::uint64_t;
  using pointer = const T*;
  using reference = const T&;

  const_symtab_iterator(T* symtab, char* strtab) noexcept : sym_{symtab}, strtab_{strtab} {}

  constexpr value_type operator*() const noexcept { return {&strtab_[sym_->st_name], *sym_}; }
  constexpr pointer operator->() const noexcept { return sym_; }

  const_symtab_iterator& operator++() noexcept {
    ++sym_;
    return *this;
  }

  const_symtab_iterator operator++(int) noexcept {
    const_symtab_iterator tmp = *this;
    ++*this;
    return tmp;
  }

  constexpr bool operator!=(const_symtab_iterator& rhs) const noexcept { return rhs.sym_ != sym_; }

 private:
  T* sym_;
  char* strtab_;
};

template <typename T>
class symtab_iterator : public const_symtab_iterator<T> {
 public:
  using value_type = std::pair<char*, T&>;
  using difference_type = std::uint64_t;
  using pointer = T*;
  using reference = T&;

  using const_symtab_iterator<T>::const_symtab_iterator;

  constexpr value_type operator*() const noexcept { return {const_cast<char*>(const_symtab_iterator<T>::operator*().first), const_cast<reference>(const_symtab_iterator<T>::operator*().second)}; }
  constexpr pointer operator->() const noexcept { return this->_Ptr; }

  constexpr symtab_iterator& operator++() noexcept {
    const_symtab_iterator<T>::operator++();
    return *this;
  }

  constexpr symtab_iterator operator++(int) noexcept {
    symtab_iterator tmp = *this;
    const_symtab_iterator<T>::operator++();
    return tmp;
  }
};

template <typename Sym, typename Shdr, typename Ehdr>
std::ostream& symdump(std::ostream& os, void* base) {
  auto buff = static_cast<std::uint8_t*>(base);

  auto& ehdr = reinterpret_cast<Ehdr&>(buff[0]);
  auto shdr = reinterpret_cast<Shdr*>(&buff[ehdr.e_shoff]);

  os << std::left;

  auto shstrtab = reinterpret_cast<char*>(&buff[shdr[ehdr.e_shstrndx].sh_offset]);
  for (std::size_t i = 0; i < ehdr.e_shnum; ++i) {
    if (shdr[i].sh_type == SHT_SYMTAB/* || shdr[i].sh_type == SHT_DYNSYM*/) {
      os << &shstrtab[shdr[i].sh_name] << '\n';
      // os << &shstrtab[shdr[shdr[i].sh_link].sh_name] << '\n';

      auto symtab = reinterpret_cast<Sym*>(&buff[shdr[i].sh_offset]);
      auto strtab = reinterpret_cast<char*>(&buff[shdr[shdr[i].sh_link].sh_offset]);
      auto size = shdr[i].sh_size / shdr[i].sh_entsize;
      
      symtab_iterator<Sym> first{symtab, strtab}, last{symtab + size, strtab};
      for (auto iter = first; iter != last; ++iter) {
        os << (*iter).second << ' ' << (*iter).first << '\n';
      }
      os << '\n';
    }
  }
  os << '\n';

  os << std::right;
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
        symdump<Elf64_Sym, Elf64_Shdr, Elf64_Ehdr>(std::cout, buff.data());
        //gnuhash_dump<std::uint64_t, Elf64_Sym, Elf64_Shdr, Elf64_Ehdr>(std::cout, buff.data());
      } else if (buff[EI_CLASS] == ELFCLASS32) {
        symdump<Elf32_Sym, Elf32_Shdr, Elf32_Ehdr>(std::cout, buff.data());
        //gnuhash_dump<std::uint32_t, Elf32_Sym, Elf32_Shdr, Elf32_Ehdr>(std::cout, buff.data());
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
