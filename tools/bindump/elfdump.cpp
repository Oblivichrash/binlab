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

std::uint32_t gnu_hash(const std::uint8_t* name) {
  std::uint32_t h = 5381;

  for (; *name; name++) {
    h = (h << 5) + h + *name;
  }

  return h;
}

// Reference: https://flapenguin.me/elf-dt-gnu-hash
template <typename bloom_el_t>
const Elf64_Sym* GNULookup(const char* strtab, const Elf64_Sym* symtab, const std::uint32_t* hashtab, const char* name) {
  constexpr auto num_bits = sizeof(bloom_el_t) * 8;

  const std::uint32_t namehash = gnu_hash(reinterpret_cast<const std::uint8_t*>(name));

  auto nbuckets = hashtab[0];
  auto symoffset = hashtab[1];
  auto bloom_size = hashtab[2];
  auto bloom_shift = hashtab[3];
  auto bloom = reinterpret_cast<const bloom_el_t*>(&hashtab[4]);
  auto buckets = reinterpret_cast<const std::uint32_t*>(&bloom[bloom_size]);
  auto chain = &buckets[nbuckets];

  bloom_el_t word = bloom[(namehash / num_bits) % bloom_size];
  bloom_el_t mask = 0;
  mask |= static_cast<bloom_el_t>(1) << (namehash % num_bits);
  mask |= static_cast<bloom_el_t>(1) << ((namehash >> bloom_shift) % num_bits);

  if ((word & mask) != mask) {
    std::cerr << "at least one bit is not set, symbol is surely missing\n";
    return nullptr;
  }

  auto symix = buckets[namehash % nbuckets];
  if (symix < symoffset) {
    std::cerr << "invalid symbol index\n";
    return nullptr;
  }

  for (std::uint32_t hash;; ++symix) {
    hash = chain[symix - symoffset];

    if ((namehash | 1) == (hash | 1)) {
      if (!std::strcmp(name, &strtab[symtab[symix].st_name])) {
        return &symtab[symix];
      }
    }

    // Chain ends with an element with the lowest bit set to 1.
    if (hash & 1) {
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
  auto sym = GNULookup<std::uint64_t>(strtab, symtab, gnu_hash, name.c_str());
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
