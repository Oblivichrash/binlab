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

  symbols(const key_type strtab, const mapped_type* symtab, const void* hash) : strtab_{strtab}, first_{symtab} {
    hash_ = static_cast<const std::uint32_t*>(hash);
    bloom_ = reinterpret_cast<const_bloom_pointer>(&hash_[4]);
    buckets_ = reinterpret_cast<const_bucket_pointer>(&(bloom_[bloom_count()]));
    chain_ = reinterpret_cast<const_local_iterator>(&buckets_[bucket_count()]);
  }

  const_iterator cbegin() const noexcept { return first_; }

  bool contains(const key_type& k) const;

  const mapped_type& at(const key_type& k) const;

  // Bucket interface
  const_local_iterator cbegin(size_type n) const noexcept {
    difference_type index = buckets_[n];
    index -= symbol_offset();
    return ((index < 0) ? nullptr : &chain_[index]);
  }
  //const_local_iterator cend(size_type n) const noexcept;
  size_type bucket_count() const noexcept { return hash_[0]; }
  size_type bucket_size(size_type n) const noexcept;
  size_type bucket(key_type key) const noexcept {
    auto keyhash = gnu_hash_(reinterpret_cast<const std::uint8_t*>(key));
    return (keyhash % bucket_count());
  }

 private:
  // Non-standared
  using bloom_type = std::conditional_t<std::is_same_v<mapped_type, Elf64_Sym>, std::uint64_t, std::uint32_t>;
  using const_bloom_pointer = const bloom_type*;
  using const_bucket_pointer = const std::uint32_t*;

  size_type symbol_offset() const noexcept { return hash_[1]; }
  bool bloom_filter(std::uint32_t keyhash) const noexcept;

  size_type bloom_count() const noexcept { return hash_[2]; }
  size_type bloom_shift() const noexcept { return hash_[3]; }

  const key_type strtab_;
  const_iterator first_;

  const std::uint32_t* hash_;
  const_bloom_pointer bloom_;
  const_bucket_pointer buckets_;
  const_local_iterator chain_;
};

template <typename Key, typename T>
inline bool symbols<Key, T>::contains(const key_type& name) const {
  const std::uint32_t namehash = gnu_hash_(reinterpret_cast<const std::uint8_t*>(name));

  if (bloom_filter(namehash)) {
    auto n = namehash % bucket_count();
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
    auto n = namehash % bucket_count();
    auto sym = cbegin() + buckets_[n];
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

template <typename Key, typename T>
inline auto symbols<Key, T>::bucket_size(size_type n) const noexcept -> size_type {
  size_type count = 0;
  auto iter = cbegin(n);
  if (iter != nullptr) {
    for (; !(*iter & 1); ++iter) {
      ++count;
    }
    ++count;
  }
  return count;
}

template <typename Key, typename T>
inline bool symbols<Key, T>::bloom_filter(std::uint32_t keyhash) const noexcept {
  constexpr auto bits = sizeof(bloom_type) * 8;
  auto word = bloom_[(keyhash / bits) % bloom_count()];
  bloom_type mask = 0;
  mask |= static_cast<bloom_type>(1) << (keyhash % bits);
  mask |= static_cast<bloom_type>(1) << ((keyhash >> bloom_shift()) % bits);
  return (word & mask) == mask;
}

}  // namespace ELF
}  // namespace binlab

#endif  // !BINLAB_ELF_SYMBOLS_H_
