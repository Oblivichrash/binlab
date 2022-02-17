// tools/bindump/elfdump.cpp:

#include "elfdump.h"

using namespace binlab;
using namespace binlab::ELF;

void ELF::Dump(std::vector<char>& buff) {
  auto& ehdr = reinterpret_cast<Elf64_Ehdr&>(buff[0]);
  if (!std::memcmp(&buff[0], ELFMAG, SELFMAG)) {
    switch (ehdr.e_ident[EI_CLASS]) {
      case ELFCLASS32:
        break;
      case ELFCLASS64:
        Dump64LE(buff);
        break;
      default:
        std::cerr << "invalid ELF class\n";
        break;
    }
  }
}

void ELF::Dump64LE(std::vector<char>& buff) {
  auto& ehdr = reinterpret_cast<Elf64_Ehdr&>(buff[0]);
  auto phdrs = reinterpret_cast<ELF::Elf64_Phdr*>(&buff[ehdr.e_phoff]);

  Accessor base{&buff[0], phdrs, ehdr.e_phnum};
  for (auto i = 0; i < ehdr.e_phnum; ++i) {
    switch (phdrs[i].p_type) {
      case PT_DYNAMIC:
        Dump(base, reinterpret_cast<const ELF::Elf64_Dyn*>(&base[phdrs[i].p_offset]));
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

struct gnu_hash_table_header {
  std::uint32_t nbuckets;
  std::uint32_t symoffset;
  std::uint32_t bloom_size;
  std::uint32_t bloom_shift;
};

void Dump(const Accessor& base, const gnu_hash_table_header* hash) {
  auto bloom = reinterpret_cast<const std::uint64_t*>(hash + 1);  // for 64 bits
  auto buckets = reinterpret_cast<const std::uint32_t*>(&bloom[hash->bloom_size]);
  auto chain = reinterpret_cast<const std::uint32_t*>(&buckets[hash->nbuckets]);

  const char name[] = "func1";
  auto namehash = gnu_hash(reinterpret_cast<const std::uint8_t*>(name));

  constexpr auto ELFCLASS_BITS = sizeof(bloom[0]) * 8;

  auto word = bloom[(namehash / ELFCLASS_BITS) % hash->bloom_size];
  auto mask = std::decay_t<decltype(bloom[0])>{};

  mask |= static_cast<decltype(mask)>(1) << (namehash % ELFCLASS_BITS);
  mask |= static_cast<decltype(mask)>(1) << ((namehash >> hash->bloom_shift) % ELFCLASS_BITS);

  if ((word & mask) != mask) {
    std::cerr << "at least one bit is not set, symbol is surely missing\n";
    return;
  }

  auto symix = buckets[namehash % hash->nbuckets];
  if (symix < hash->symoffset) {
    std::cerr << "invalid symbol index\n";
    return;
  }

  //while (true) {
  //  const char* symname = strtab + symtab[symix].st_name;
  //  const uint32_t hash = chain[symix - symoffset];

  //  if ((namehash | 1) == (hash | 1) && strcmp(name, symname) == 0) {
  //    return &symtab[symix];
  //  }

  //  /* Chain ends with an element with the lowest bit set to 1. */
  //  if (hash & 1) {
  //    break;
  //  }

  //  symix++;
  //}
}

void ELF::Dump(const Accessor& base, const ELF::Elf64_Dyn* dyns) {
  const gnu_hash_table_header* hash;

  for (auto dyn = dyns; dyn->d_tag != DT_NULL; ++dyn) {
    switch (dyn->d_tag) {
      case DT_GNU_HASH:
        hash = reinterpret_cast<const gnu_hash_table_header*>(&base[dyn->d_un.d_ptr]);
        break;
      default:
        break;
    }
  }
}