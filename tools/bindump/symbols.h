// symbols.h: ELF symbols, for more about DT_GNU_HASH, see: https://flapenguin.me/elf-dt-gnu-hash
//

#ifndef BINLAB_SYMBOLS_ELF_SYMBOLS_H_
#define BINLAB_SYMBOLS_ELF_SYMBOLS_H_

#include <cstdint>
#include <iterator>
#include <stdexcept>
#include <utility>

#include "binlab/BinaryFormat/ELF.h"

namespace binlab {
namespace ELF {

template <typename T>
struct bloom_traits;

template <>
struct bloom_traits<Elf64_Sym> {
  using value_type = std::uint64_t;
  static constexpr std::size_t num_bits = sizeof(value_type) * 8;
};

template <>
struct bloom_traits<Elf32_Sym> {
  using value_type = std::uint32_t;
  static constexpr std::size_t num_bits = sizeof(value_type) * 8;
};

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
  using key_type              = Key;
  using mapped_type           = T;
  using value_type            = std::pair<const Key, T>;
  using size_type             = std::uint64_t;
  using difference_type       = std::int64_t;

  using const_iterator        = const mapped_type*;
  using const_local_iterator  = const std::uint32_t*;

  symbols(const key_type strtab, const mapped_type* symtab, const void* hash) : strtab_{strtab}, first_{symtab} {
    hash_     = static_cast<const std::uint32_t*>(hash);
    bloom_    = reinterpret_cast<const_bloom_pointer>(&hash_[4]);
    buckets_  = reinterpret_cast<const_bucket_pointer>(&bloom_[bloom_count()]);
    chain_    = reinterpret_cast<const_local_iterator>(&buckets_[bucket_count()]);

    auto last_bucket = bucket_count() - 1;
    last_ = first_ + buckets_[last_bucket] + bucket_size(last_bucket);
  }

  // iterators
  const_iterator begin() const noexcept { return first_; }
  const_iterator end() const noexcept { return last_; }

  // capacity
  size_type size() const noexcept { return std::distance(begin(), end()); }

  // set operations
  const_iterator find(const key_type& k) const noexcept;

  // bucket interface
  const_local_iterator begin(size_type n) const noexcept;
  //const_local_iterator end(size_type n) const noexcept;
  size_type bucket_count() const noexcept { return hash_[0]; }
  size_type bucket_size(size_type n) const noexcept;
  size_type bucket(key_type k) const noexcept;

 private:
  // non-standared
  using bloom_type = typename bloom_traits<mapped_type>::value_type;
  using const_bloom_pointer = const bloom_type*;
  using const_bucket_pointer = const std::uint32_t*;

  size_type symbol_offset() const noexcept { return hash_[1]; }
  bool bloom_filter(std::uint32_t keyhash) const noexcept;

  size_type bloom_count() const noexcept { return hash_[2]; }
  size_type bloom_shift() const noexcept { return hash_[3]; }

  const key_type strtab_;
  const_iterator first_;
  const_iterator last_;

  const std::uint32_t* hash_;
  const_bloom_pointer bloom_;
  const_bucket_pointer buckets_;
  const_local_iterator chain_;
};

template <typename Key, typename T>
inline auto symbols<Key, T>::find(const key_type& k) const noexcept -> const_iterator{
  const std::uint32_t hash = gnu_hash_(reinterpret_cast<const std::uint8_t*>(k));

  if (!bloom_filter(hash)) {
    return end();
  }

  auto n = hash % bucket_count();
  if (auto iter = begin(n)) {
    for (auto sym_iter = first_ + buckets_[n];; ++sym_iter, ++iter) {
      if ((hash | 1) == (*iter | 1)) {
        if (!std::strcmp(k, &strtab_[sym_iter->st_name])) {
          return sym_iter;
        }
      }
      if (*iter & 1) {
        break;
      }
    }
  }
  return end();
}

template <typename Key, typename T>
inline auto symbols<Key, T>::begin(size_type n) const noexcept -> const_local_iterator {
  auto index = buckets_[n];
  if (index < symbol_offset()) {
    return nullptr;
  } else {
    return chain_ + buckets_[n] - symbol_offset();
  }
}

//template <typename Key, typename T>
//inline auto symbols<Key, T>::end(size_type n) const noexcept -> const_local_iterator {
//  return begin(n) + bucket_size(n);
//}

template <typename Key, typename T>
inline auto symbols<Key, T>::bucket_size(size_type n) const noexcept -> size_type {
  if (auto iter = begin(n)) {
    size_type counter = 0;
    for (; !(*iter & 1); ++iter) {
      ++counter;
    }
    return counter + 1;
  }
  return 0;
}

template <typename Key, typename T>
inline auto symbols<Key, T>::bucket(key_type key) const noexcept -> size_type {
  auto keyhash = gnu_hash_(reinterpret_cast<const std::uint8_t*>(key));
  return (keyhash % bucket_count());
}

template <typename Key, typename T>
inline bool symbols<Key, T>::bloom_filter(std::uint32_t keyhash) const noexcept {
  constexpr auto bits = bloom_traits<mapped_type>::num_bits;
  auto word = bloom_[(keyhash / bits) % bloom_count()];
  bloom_type mask = 0;
  mask |= static_cast<bloom_type>(1) << (keyhash % bits);
  mask |= static_cast<bloom_type>(1) << ((keyhash >> bloom_shift()) % bits);
  return (word & mask) == mask;
}

}  // namespace ELF
}  // namespace binlab

#endif  // !BINLAB_SYMBOLS_ELF_SYMBOLS_H_
