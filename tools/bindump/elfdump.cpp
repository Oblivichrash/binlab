// tools/bindump/elfdump.cpp:

#include "elfdump.h"

using namespace binlab;
using namespace binlab::ELF;

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

constexpr std::uint32_t gnu_hash_(const std::uint8_t* name) {
  std::uint32_t h = 5381;

  for (; *name; name++) {
    h = (h << 5) + h + *name;
  }

  return h;
}

class gnu_hash {
 public:
  using key_type = const char*;
  using mapped_type = Elf64_Sym;
  using bloom_type = std::uint64_t;
  using size_type = std::uint32_t;
  using const_local_iterator = const std::uint32_t*;

  gnu_hash(const void* hash) : hash_{static_cast<const std::uint32_t*>(hash)} {}

  // Bucket interface
  const_local_iterator cbegin(size_type n) const noexcept;
  //const_local_iterator cend(size_type n) const noexcept;
  size_type bucket_count() const noexcept;
  //size_type bucket_size(size_type n) const noexcept;
  size_type bucket(const key_type key) const noexcept;

  // Non-standared
  size_type symbol_offset() const noexcept { return hash_[1]; }
  const_local_iterator buckets() const noexcept { return reinterpret_cast<const_local_iterator>(&(bloom()[bloom_count()])); }
  bool bloom_filter(std::uint32_t keyhash) const noexcept;

 private:
  using const_bloom_iterator = const bloom_type*;

  size_type bloom_count() const noexcept { return hash_[2]; }
  size_type bloom_shift() const noexcept { return hash_[3]; }
  const_bloom_iterator bloom() const noexcept { return reinterpret_cast<const_bloom_iterator>(&hash_[4]); }

  const_local_iterator chain() const noexcept { return &(buckets()[bucket_count()]); }

  const std::uint32_t* hash_;
};

// Bucket interface
inline auto gnu_hash::cbegin(size_type n) const noexcept -> const_local_iterator {
  return &chain()[(buckets()[n]) - symbol_offset()];
}

//inline auto gnu_hash::cend(size_type n) const noexcept -> const_local_iterator {
//  if (n < bucket_count()) {
//    return cbegin(n + 1);
//  } else {
//    auto iter = cbegin(n);
//    while (!(*iter++ & 1));  // bucket end with ((*iter & 1) == true)
//    return iter;
//  }
//}

inline auto gnu_hash::bucket_count() const noexcept -> size_type {
  return hash_[0];
}

//inline auto gnu_hash::bucket_size(size_type n) const noexcept -> size_type {
//  size_type count = 1;
//  for (auto iter = cbegin(n); !(*iter & 1); ++iter) {
//    ++count;
//  }
//  return ++count;
//}

inline auto gnu_hash::bucket(const key_type key) const noexcept -> size_type {
  auto keyhash = ::gnu_hash_(reinterpret_cast<const std::uint8_t*>(key));
  return (keyhash % bucket_count());
}

inline bool gnu_hash::bloom_filter(std::uint32_t keyhash) const noexcept {
  constexpr auto num_bits = sizeof(bloom_type) * 8;
  auto word = bloom()[(keyhash / num_bits) % bloom_count()];
  bloom_type mask = 0;
  mask |= static_cast<bloom_type>(1) << (keyhash % num_bits);
  mask |= static_cast<bloom_type>(1) << ((keyhash >> bloom_shift()) % num_bits);
  return (word & mask) == mask;
}

// Reference: https://flapenguin.me/elf-dt-gnu-hash
const Elf64_Sym* GNULookup(const char* strtab, const Elf64_Sym* symtab, const std::uint32_t* hashtab, const char* name) {
  const std::uint32_t namehash = gnu_hash_(reinterpret_cast<const std::uint8_t*>(name));

  gnu_hash hash{static_cast<const void*>(hashtab)};
  if (!hash.bloom_filter(namehash)) {
    std::cerr << "at least one bit is not set, symbol is surely missing\n";
    return nullptr;
  }

  auto bucket = hash.bucket(name);

  auto symix = hash.buckets()[bucket];
  if (symix < hash.symbol_offset()) {
    std::cerr << "invalid symbol index\n";
    return nullptr;
  }

  auto first = hash.cbegin(bucket);
  for (const std::uint32_t* iter = first; iter; ++iter, ++symix) {
    if ((namehash | 1) == (*iter | 1)) {
      if (!std::strcmp(name, &strtab[symtab[symix].st_name])) {
        return &symtab[symix];
      }
    }
    if (*iter & 1) {
      break;
    }
  }
  return nullptr;
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

  std::string name = "_ZTISt13runtime_error";
  auto sym = GNULookup(strtab, symtab, gnu_hash, name.c_str());
  std::cout << std::hex;
  if (sym != nullptr) {
    std::cout << std::setw(16) << "name: " << &strtab[sym->st_name] << '\n';
    std::cout << std::setw(16) << "bind: " << std::showbase << ELF64_ST_BIND(sym->st_info) << '\n';
    std::cout << std::setw(16) << "type: " << std::showbase << ELF64_ST_TYPE(sym->st_info) << '\n';
    std::cout << std::setw(16) << "visibility: " << std::showbase << ELF64_ST_VISIBILITY(sym->st_other) << '\n';
    std::cout << std::setw(16) << "shndx: " << std::showbase << sym->st_shndx << '\n';
    std::cout << std::setw(16) << "value: " << std::showbase << sym->st_value << '\n';
    std::cout << std::setw(16) << "size: " << std::showbase << sym->st_size << '\n';
  } else {
    std::cerr << "couldn't find symbol: " << name << '\n';
  }
  std::cout << std::dec;
}
