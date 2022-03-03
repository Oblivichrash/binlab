// symbols.h: reference: https://flapenguin.me/elf-dt-gnu-hash

#ifndef BINLAB_ELF_SYMBOLS_H_
#define BINLAB_ELF_SYMBOLS_H_

#include <cstdint>
#include <stdexcept>
#include <utility>

namespace binlab {
namespace ELF {

constexpr std::uint32_t gnu_hash_(const std::uint8_t* name) {
  std::uint32_t h = 5381;

  for (; *name; name++) {
    h = (h << 5) + h + *name;
  }

  return h;
}

template <typename Key, typename T>
class symbols {
 public:
  using key_type                = Key;
  using mapped_type             = T;
  using value_type              = std::pair<const Key, T>;
  using size_type               = std::uint32_t;
  using difference_type         = std::int64_t;

  using const_iterator          = const mapped_type*;
  using const_local_iterator    = const std::uint32_t*;

  symbols(const key_type strtab, const mapped_type* symtab, const void* hash)
      : strtab_{strtab}, symtab_{symtab}, hash_{static_cast<const std::uint32_t*>(hash)} {}

  const_iterator cbegin() const noexcept { return symtab_; }

  bool contains(const key_type& k) const;

  const mapped_type& at(const key_type& k) const;

  // Bucket interface
  const_local_iterator cbegin(size_type n) const noexcept {
    difference_type index = buckets()[n] - symbol_offset();
    return ((index < 0) ? nullptr : &chain()[index]);
  }
  //const_local_iterator cend(size_type n) const noexcept;
  size_type bucket_count() const noexcept { return hash_[0]; }
  //size_type bucket_size(size_type n) const noexcept;
  size_type bucket(key_type key) const noexcept {
    auto keyhash = gnu_hash_(reinterpret_cast<const std::uint8_t*>(key));
    return (keyhash % bucket_count());
  }

 private:
  // Non-standared
  using bloom_type = std::conditional_t<std::is_same_v<mapped_type, Elf64_Sym>, std::uint64_t, std::uint32_t>;
  using const_bloom_iterator = const bloom_type*;

  size_type symbol_offset() const noexcept { return hash_[1]; }
  const_local_iterator buckets() const noexcept { return reinterpret_cast<const_local_iterator>(&(bloom()[bloom_count()])); }
  bool bloom_filter(std::uint32_t keyhash) const noexcept;

  size_type bloom_count() const noexcept { return hash_[2]; }
  size_type bloom_shift() const noexcept { return hash_[3]; }
  const_bloom_iterator bloom() const noexcept { return reinterpret_cast<const_bloom_iterator>(&hash_[4]); }

  const_local_iterator chain() const noexcept { return &(buckets()[bucket_count()]); }

  const std::uint32_t* hash_;
  const mapped_type* symtab_;
  const key_type strtab_;
};

template <typename Key, typename T>
inline bool symbols<Key, T>::contains(const key_type& name) const {
  const std::uint32_t namehash = gnu_hash_(reinterpret_cast<const std::uint8_t*>(name));

  if (bloom_filter(namehash)) {
    auto n = bucket(name);
    auto sym = cbegin() + buckets()[n];
    for (auto iter = cbegin(n);; ++sym, ++iter) {
      if ((namehash | 1) == (*iter | 1)) {
        if (!std::strcmp(name, &strtab_[sym->st_name])) {
          return true;
        }
      }
      if (*iter & 1) {
        break;
      }
    }
  }
  return false;
}

template <typename Key, typename T>
auto symbols<Key, T>::at(const key_type& name) const -> const mapped_type& {
  const std::uint32_t namehash = gnu_hash_(reinterpret_cast<const std::uint8_t*>(name));

  if (bloom_filter(namehash)) {
    auto n = bucket(name);
    auto sym = cbegin() + buckets()[n];
    for (auto iter = cbegin(n);; ++sym, ++iter) {
      if ((namehash | 1) == (*iter | 1)) {
        if (!std::strcmp(name, &strtab_[sym->st_name])) {
          return *sym;
        }
      }
      if (*iter & 1) {
        break;
      }
    }
  }
  throw std::out_of_range{"invalid symbol"};
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

//inline auto gnu_hash::bucket_size(size_type n) const noexcept -> size_type {
//  size_type count = 1;
//  for (auto iter = cbegin(n); !(*iter & 1); ++iter) {
//    ++count;
//  }
//  return ++count;
//}

template <typename Key, typename T>
inline bool symbols<Key, T>::bloom_filter(std::uint32_t keyhash) const noexcept {
  constexpr auto num_bits = sizeof(bloom_type) * 8;
  auto word = bloom()[(keyhash / num_bits) % bloom_count()];
  bloom_type mask = 0;
  mask |= static_cast<bloom_type>(1) << (keyhash % num_bits);
  mask |= static_cast<bloom_type>(1) << ((keyhash >> bloom_shift()) % num_bits);
  return (word & mask) == mask;
}

}  // namespace ELF
}  // namespace binlab

#endif  // !BINLAB_ELF_SYMBOLS_H_
