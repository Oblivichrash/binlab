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
#include "elf_stream.h"
#include "iterator_facade.h"

#ifdef __BIONIC__
#define SHT_GNU_HASH    0x6ffffff6
#define SHT_GNU_versym  0x6fffffff
#define SHT_GNU_verneed 0x6ffffffe
#define SHT_GNU_verdef  0x6ffffffd
#endif // !__BIONIC__

template <typename T>
class elf_forward_iterator : public iterator_facade<elf_forward_iterator<T>, T, std::forward_iterator_tag> {
 public:
  elf_forward_iterator(T* ptr) : current_{ptr} {}

 private:
  T& dereference() const { return *current_; }
  void increment() { current_ = next(current_); }

  bool equals(const elf_forward_iterator& rhs) const { return current_ == rhs.current_; }

  T* current_;

  friend class iterator_facade_access;
};

template <typename T>
class elf_random_iterator : public iterator_facade<elf_random_iterator<T>, T, std::random_access_iterator_tag> {
 public:
  elf_random_iterator(T* ptr) : current_{ptr} {}

 private:
  T& dereference() const { return *current_; }
  void increment() { ++current_; }
  bool equals(const elf_random_iterator& rhs) const { return current_ == rhs.current_; }

  void advance(std::size_t n) { current_ += n; }

  T* current_;

  friend class iterator_facade_access;
};

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

template <typename Bloom, typename Sym>
std::ostream& symdump(std::ostream& os, Sym* dynsym, std::size_t size, char* dynstr, gnu_hash_table& table) {
  auto bloom = reinterpret_cast<Bloom*>(&table + 1);
  auto buckets = reinterpret_cast<std::uint32_t*>(&bloom[table.bloom_size]);
  auto chain = &buckets[table.nbuckets];

  for (std::size_t j = 0; j < size; ++j) {
    auto name = &dynstr[dynsym[j].st_name];
    auto namehash = gnu_hash(reinterpret_cast<std::uint8_t*>(name));
    
    auto word = bloom[(namehash / (sizeof(Bloom) * 8)) % table.bloom_size];
    Bloom mask = 0 | (Bloom)1 << (namehash % (sizeof(Bloom) * 8)) | (Bloom)1 << ((namehash >> table.bloom_shift) % (sizeof(Bloom) * 8));

    if ((word & mask) != mask) {
      os << "not in bloom filter: " << name << '\n';
      continue;
    }

    std::uint32_t symix = buckets[namehash % table.nbuckets];
    if (symix < table.symoffset) {
      os << "invalid symbol index: " << name << '\n';
      continue;
    }
    
    for (std::size_t k = symix;; ++k) {
      auto symbol_name = &dynstr[dynsym[k].st_name];
      auto hash = chain[k - table.symoffset];
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

template <typename Verdaux, typename Verdef>
std::ostream& vddump(std::ostream& os, Verdef* verdef, const char* dynstr) {
  for (auto vd = verdef; vd; vd = next(vd)) {
    os << *vd;
    auto first = reinterpret_cast<Verdaux*>(reinterpret_cast<char*>(vd) + vd->vd_aux);
    std::for_each_n(first, vd->vd_cnt, [dynstr, &os](const Verdaux& vda) { os << '\t' << &dynstr[vda.vda_name]; }); 
    os << '\n';
  }
  return os;
}

template <typename Vernaux, typename Verneed>
std::ostream& vndump(std::ostream& os, Verneed* verneed, const char* dynstr) {
  for (auto vn = verneed; vn; vn = next(vn)) {
    os << *vn << &dynstr[vn->vn_file] << '\n';
    if (vn->vn_aux) {
      auto first = reinterpret_cast<Vernaux*>(reinterpret_cast<char*>(vn) + vn->vn_aux);
      std::for_each_n(first, vn->vn_cnt, [dynstr, &os](const Vernaux& vna) { os << vna << &dynstr[vna.vna_name] << '\n'; }); 
    }
    os << '\n';
  }
  return os;
}

template <typename Verdaux, typename Verdef, typename Vernaux, typename Verneed, typename Versym, typename Sym>
std::ostream& symdump(std::ostream& os, Sym* dynsym, const char* dynstr, Versym* versym, std::size_t size, Verneed* verneed, Verdef* verdef) {
  for (std::size_t i = 0; i < size; ++i) {
    os << dynsym[i] << ' ' << &dynstr[dynsym[i].st_name];
    if (!dynsym[i].st_value && !dynsym[i].st_size && !dynsym[i].st_shndx) {
      for (auto vn = verneed; vn; vn = next(vn)) {
        if (vn->vn_aux) {
          for (auto vna = reinterpret_cast<Vernaux*>(reinterpret_cast<char*>(vn) + vn->vn_aux); vna; vna = next(vna)) {
            if (vna->vna_other == (versym[i] & 0x7fff)) {
              os << '@' << &dynstr[vna->vna_name];
            }
          }
        }
      }
    } else {
      for (auto vd = verdef; vd; vd = next(vd)) {
        if (vd->vd_ndx == (versym[i] & 0x7fff)) {
          auto vda = reinterpret_cast<Verdaux*>(reinterpret_cast<char*>(vd) + vd->vd_aux);
          os << '@' << &dynstr[vda->vda_name];
        }
      }
    }
    os << '\n';
  }
  return os;
}

template <typename Verdaux, typename Verdef, typename Vernaux, typename Verneed, typename Sym, typename Shdr>
std::ostream& symdump(std::ostream& os, char* base, Shdr* first, Shdr* last, char* shstrtab) {
  auto iter = std::find_if(first, last, [](const Shdr& s) { return s.sh_type == SHT_GNU_versym; });
  if (iter != last) {
    os << &shstrtab[iter->sh_name] << '\n';
    os << &shstrtab[first[iter->sh_link].sh_name] << '\n';
    os << &shstrtab[first[first[iter->sh_link].sh_link].sh_name] << '\n';

    auto versym = reinterpret_cast<decltype(Vernaux::vna_other)*>(&base[iter->sh_offset]);
    auto dynsym = reinterpret_cast<Sym*>(&base[first[iter->sh_link].sh_offset]);
    auto dynstr = reinterpret_cast<char*>(&base[first[first[iter->sh_link].sh_link].sh_offset]);
    auto size = iter->sh_size / iter->sh_entsize;

    iter = std::find_if(first, last, [](const Shdr& s) { return s.sh_type == SHT_GNU_verneed; });
    auto verneed = (iter != last) ? reinterpret_cast<Verneed*>(&base[iter->sh_offset]) : nullptr;

    iter = std::find_if(first, last, [](const Shdr& s) { return s.sh_type == SHT_GNU_verdef; });
    auto verdef = (iter != last) ? reinterpret_cast<Verdef*>(&base[iter->sh_offset]) : nullptr;

    symdump<Verdaux, Verdef, Vernaux, Verneed>(os, dynsym, dynstr, versym, size, verneed, verdef);
    os << '\n';
    
    vddump<Verdaux>(os, verdef, dynstr);
    os << '\n';
    vndump<Vernaux>(os, verneed, dynstr);
  }
  return os;
}

template <typename Bloom, typename Sym, typename Shdr>
std::ostream& gnuhash_dump(std::ostream& os, char* base, Shdr* first, Shdr* last, char* shstrtab) {
  auto iter = std::find_if(first, last, [](const Shdr& s) { return s.sh_type == SHT_GNU_HASH; });
  if (iter != last) {
    os << &shstrtab[iter->sh_name] << '\n';
    os << &shstrtab[first[iter->sh_link].sh_name] << '\n';
    os << &shstrtab[first[first[iter->sh_link].sh_link].sh_name] << '\n';
      
    auto size = first[iter->sh_link].sh_size / first[iter->sh_link].sh_entsize;
    auto table = reinterpret_cast<gnu_hash_table*>(&base[iter->sh_offset]);
    auto dynsym = reinterpret_cast<Sym*>(&base[first[iter->sh_link].sh_offset]);
    auto dynstr = reinterpret_cast<char*>(&base[first[first[iter->sh_link].sh_link].sh_offset]);

    symdump<Bloom>(os, dynsym, size, dynstr, *table);
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
      std::cout << std::hex << std::left;
      if (buff[EI_CLASS] == ELFCLASS64) {
        auto& ehdr = reinterpret_cast<Elf64_Ehdr&>(buff[0]);
        auto first = reinterpret_cast<Elf64_Shdr*>(&buff[ehdr.e_shoff]);
        auto last = first + ehdr.e_shnum;
        
        auto shstrtab = reinterpret_cast<char*>(&buff[first[ehdr.e_shstrndx].sh_offset]);
        
        symdump<Elf64_Verdaux, Elf64_Verdef, Elf64_Vernaux, Elf64_Verneed, Elf64_Sym, Elf64_Shdr>(std::cout, buff.data(), first, last, shstrtab);
        gnuhash_dump<std::uint64_t, Elf64_Sym, Elf64_Shdr>(std::cout, buff.data(), first, last, shstrtab);
      } else if (buff[EI_CLASS] == ELFCLASS32) {
        auto& ehdr = reinterpret_cast<Elf32_Ehdr&>(buff[0]);
        auto first = reinterpret_cast<Elf32_Shdr*>(&buff[ehdr.e_shoff]);
        auto last = first + ehdr.e_shnum;
        
        auto shstrtab = reinterpret_cast<char*>(&buff[first[ehdr.e_shstrndx].sh_offset]);
        
        symdump<Elf32_Verdaux, Elf32_Verdef, Elf32_Vernaux, Elf32_Verneed, Elf32_Sym, Elf32_Shdr>(std::cout, buff.data(), first, last, shstrtab);
        gnuhash_dump<std::uint32_t, Elf32_Sym, Elf32_Shdr>(std::cout, buff.data(), first, last, shstrtab);
      } else {
        std::cerr << "the ELF class is invalid\n";
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
