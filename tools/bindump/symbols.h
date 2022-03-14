// symbols.h: ELF symbols, for more about DT_GNU_HASH, see: https://flapenguin.me/elf-dt-gnu-hash
//

#ifndef BINLAB_SYMBOLS_ELF_SYMBOLS_H_
#define BINLAB_SYMBOLS_ELF_SYMBOLS_H_

#include <cstdint>
#include <iterator>
#include <stdexcept>
#include <utility>

#include "binlab/BinaryFormat/ELF.h"
#include "bin_iterator.h"

namespace binlab {
namespace ELF {

template <typename T>
struct bloom_traits;

template <>
struct bloom_traits<Elf64_Sym> {
  using value_type = std::uint64_t;
};

template <>
struct bloom_traits<Elf32_Sym> {
  using value_type = std::uint32_t;
};

template <typename T>
class bloom_filter {
 public:
  using value_type            = T;
  using hash_type             = std::uint32_t;
  using size_type             = std::uint64_t;
  using difference_type       = std::ptrdiff_t;

  using iterator              = value_type*;
  using const_iterator        = const iterator;

  static constexpr size_type bits = sizeof(value_type) * 8;

  bloom_filter(iterator first, size_type count, difference_type shift)
      : first_{first}, count_{count}, shift_{shift} {}

  // iterators
  constexpr iterator begin() noexcept { return first_; }
  constexpr const_iterator begin() const noexcept { return first_; }
  constexpr iterator end() noexcept { return first_ + count_; }
  constexpr const_iterator end() const noexcept { return first_ + count_; }

  // capacity
  constexpr size_type size() const noexcept { return count_; }

  // bloom filter
  constexpr bool filter(hash_type hash) const noexcept {
    value_type bloom = first_[(hash / bits) % count_];
    value_type mask = 0 | value_type{1} << hash1(hash) | value_type{1} << hash2(hash);
    return (bloom & mask) == mask;
  }

 private:
  difference_type hash1(hash_type hash) const noexcept { return hash % bits; }
  difference_type hash2(hash_type hash) const noexcept { return (hash >> shift_) % bits; }

  iterator first_;
  size_type count_;
  difference_type shift_;
};

template <typename Key, typename T>
class gnu_table {
 public:
  using key_type              = Key;
  using mapped_type           = T;
  using size_type             = std::size_t;
  using difference_type       = std::ptrdiff_t;

  using hash_table            = std::uint32_t*;
  using bucket_type           = std::uint32_t;
  using bucket_pointer        = const bucket_type*;
  using chain_type            = std::uint32_t;
  using chain_pointer         = const bucket_type*;

  using bloom_type            = typename bloom_traits<T>::value_type;
  using bloom_pointer         = const bloom_type*;

  gnu_table(hash_table table) : hash_{table} {
    bloom_ = reinterpret_cast<bloom_pointer>(&hash_[4]);
    bucket_ = reinterpret_cast<bucket_pointer>(&bloom_[bloom_count()]);
    chain_ = reinterpret_cast<chain_pointer>(&bucket_[bucket_count()]);

    chain_size_ = std::distance(chain_, chain_end(chain_, bucket_, bucket_count()));
  }

  constexpr size_type bucket_count() const noexcept { return hash_[0]; }
  constexpr difference_type symbol_offset() const noexcept { return hash_[1]; }
  constexpr size_type bloom_count(hash_table table) const noexcept { return hash_[2]; }
  constexpr size_type bloom_shift(hash_table table) const noexcept { return hash_[3]; }

  constexpr bloom_pointer bloom() const noexcept { return bloom_; }
  constexpr bucket_pointer bucket() const noexcept { return bucket_; }
  constexpr chain_pointer chain() const noexcept { return chain_; }

  constexpr size_type chain_count() const noexcept { return chain_size_; }

  static constexpr std::uint32_t hash_value(const Key k) {
    auto name = reinterpret_cast<std::uint32_t>(k);
    std::uint32_t h = 5381;
    for (; *name; name++) {
      h = (h << 5) + h + *name;
    }
    return h;
  }

 private:
  static constexpr chain_pointer chain_end(chain_pointer chain, bucket_pointer bucket, size_type bucket_size) noexcept {
    auto first = bucket, last = bucket + bucket_size;
    while (--last != first) {
      if (*last) {  // find last non-empty bucket.
        break;
      }
    }

    auto iter = chain + *last;
    while (!(*iter++ & 1))
      ;
    return iter;
  }

  hash_table hash_;

  bloom_pointer bloom_;
  bucket_pointer bucket_;
  chain_pointer chain_;
  size_type chain_size_;
};

template <typename Key, typename T>
class sysv_table {
 public:
  using key_type              = Key;
  using mapped_type           = T;
  using size_type             = std::size_t;
  using difference_type       = std::ptrdiff_t;

  using hash_table            = std::uint32_t*;
  using bucket_type           = std::uint32_t;
  using bucket_pointer        = const bucket_type*;
  using chain_type            = std::uint32_t;
  using chain_pointer         = const bucket_type*;

  sysv_table(hash_table table) : hash_{table} {
    bucket_ = reinterpret_cast<bucket_pointer>(&hash_[2]);
    chain_ = reinterpret_cast<chain_pointer>(&bucket_[bucket_count()]);
  }

  constexpr size_type bucket_count() const noexcept { return hash_[0]; }
  constexpr size_type chain_count() const noexcept { return hash_[1]; }

  constexpr bucket_pointer bucket() const noexcept { return bucket_; }
  constexpr chain_pointer chain() const noexcept { return chain_; }

  static constexpr std::uint32_t hash_value(const Key k) {
    auto name = reinterpret_cast<std::uint32_t>(k);
    std::uint32_t h = 0, g = 0;
    for (; *name; name++) {
      h = (h << 4) + *name;
      if (g = h & 0xf0000000) {
        h ^= g >> 24;
      }
      h &= ~g;
    }
    return h;
  }

 private:
  hash_table hash_;

  bucket_pointer bucket_;
  chain_pointer chain_;
};

template <typename Key, typename T>
struct hash_traits {
  using key_type              = char*;
  using mapped_type           = T;
  using value_type            = mapped_type;
  using size_type             = std::size_t;
  using difference_type       = std::ptrdiff_t;
  using pointer               = value_type*;
  using const_pointer         = const value_type*;
  using reference             = value_type&;
  using const_reference       = const value_type&;
  using iterator              = value_type*;
  using const_iterator        = const value_type*;

  using hash_table            = std::uint32_t*;
  using bucket_type           = std::uint32_t;
  using bucket_pointer        = const bucket_type*;
  using chain_type            = std::uint32_t;
  using chain_pointer         = const bucket_type*;

  static constexpr bool compare(const Key val1, const Key val2) { return !std::strcmp(val1, val2); }
};

template <typename Traits>
class hash {
 public:
  using key_type              = typename Traits::key_type;
  using mapped_type           = typename Traits::mapped_type;
  using value_type            = typename Traits::value_type;
  using size_type             = typename Traits::size_type;

  using local_iterator        = typename Traits::local_iterator;
  using const_local_iterator  = typename Traits::local_const_iterator;

  using hash_table            = typename Traits::hash_table;
  using bucket_type           = typename Traits::bucket_type;
  using bucket_pointer        = typename Traits::bucket_pointer;
  using chain_type            = typename Traits::chain_type;
  using chain_pointer         = typename Traits::chain_pointer;

  hash(hash_table hash, bucket_pointer bucket, chain_pointer chain)
      : hash_{hash}, bucket_{bucket}, chain_{chain} {}

  // bucket interface
  constexpr local_iterator begin(size_type n) noexcept { return {chain_, bucket_, n}; }
  constexpr const_local_iterator begin(size_type n) const noexcept { return {chain_, bucket_, n}; }
  constexpr size_type bucket_count() const noexcept { return Traits::bucket_count(hash_); }

 private:
  hash_table hash_;
  bucket_pointer bucket_;
  chain_pointer chain_;
};

constexpr std::uint32_t gnu_hash_(const std::uint8_t* name) {
  std::uint32_t h = 5381;
  for (; *name; name++) {
    h = (h << 5) + h + *name;
  }
  return h;
}

template <typename Symbol>
class gnu_hash {
 public:
  using value_type            = std::uint32_t;
  using size_type             = std::uint64_t;
  using pointer               = value_type*;
  using iterator              = value_type*;
  using const_iterator        = const iterator;

  using chain_iterator        = value_type*;
  using const_chain_iterator  = const chain_iterator;

  using bloom                 = bloom_filter<typename bloom_traits<Symbol>::value_type>;
  using bloom_iterator        = typename bloom::iterator;

  gnu_hash(pointer hash) : hash_{hash}, bloom_{reinterpret_cast<bloom_iterator>(&hash[4]), hash[2], hash[3]} {
    bucket_ = reinterpret_cast<bucket_iterator>(bloom_.end());
    first_ = reinterpret_cast<chain_iterator>(&bucket_[bucket_count()]) - symbol_offset();
    last_ = chain_end(bucket_, bucket_ + bucket_count(), first_);
  }

  // iterators
  constexpr iterator begin() noexcept { return first_; }
  constexpr const_iterator begin() const noexcept { return first_; }
  constexpr iterator end() noexcept { return last_; }
  constexpr const_iterator end() const noexcept { return last_; }

  // capacity
  constexpr size_type size() const noexcept { return std::distance(first_, last_); }

  // bucket interface
  constexpr chain_iterator begin(size_type n) noexcept;
  constexpr chain_iterator end(size_type n) noexcept { return begin(n + 1); }
  constexpr size_type bucket_count() const noexcept { return hash_[0]; }
  constexpr size_type bucket_size(size_type n) const noexcept { return std::distance(begin(n), end(n)); }
  constexpr size_type bucket(value_type hash) const noexcept;

  // non-standard
  constexpr size_type chain_size() const noexcept { return size() - symbol_offset(); }

 protected:
  using bucket_iterator = std::uint32_t*;
  constexpr size_type symbol_offset() const noexcept { return hash_[1]; }
  constexpr chain_iterator chain_end(bucket_iterator first, bucket_iterator last, chain_iterator chain) const noexcept;

 private:
  pointer hash_;

  bloom bloom_;
  bucket_iterator bucket_;
  chain_iterator first_;
  chain_iterator last_;
};

// bucket interface
template <typename Symbol>
constexpr auto gnu_hash<Symbol>::begin(size_type n) noexcept-> chain_iterator {
  if (n < bucket_count()) {
    if (bucket_[n] < symbol_offset()) {
      return begin(n + 1);
    } else {
      return first_ + bucket_[n];
    }
  } else {
    return last_;
  }
}

template <typename Symbol>
constexpr auto gnu_hash<Symbol>::bucket(value_type hash) const noexcept -> size_type {
  return bloom_.filter(hash) ? hash % bucket_count() : bucket_count();
}

template <typename Symbol>
constexpr auto gnu_hash<Symbol>::chain_end(bucket_iterator first, bucket_iterator last, chain_iterator chain) const noexcept -> chain_iterator {
  while (--last != first) {
    if (*last) {  // find last non-empty bucket.
      break;
    }
  }

  auto iter = chain + *last;
  while (!(*iter++ & 1))
    ;
  return iter;
}

constexpr std::uint32_t elf_hash_(const std::uint8_t* name) {
  std::uint32_t h = 0, g = 0;
  for (; *name; name++) {
    h = (h << 4) + *name;
    if (g = h & 0xf0000000) {
      h ^= g >> 24;
    }
    h &= ~g;
  }
  return h;
}

template <typename T>
class sysv_hash {
 public:
  using key_type              = char*;
  using mapped_type           = T;
  using value_type            = std::uint32_t;
  using size_type             = std::size_t;
  using pointer               = value_type*;

  using chain_iterator        = value_type*;
  using const_chain_iterator  = const chain_iterator;

  sysv_hash(key_type strtab, mapped_type* symtab, pointer hash)
      : strtab_{strtab}, symtab_{symtab}, hash_{hash}, bucket_{&hash[2]} {
    first_ = &bucket_[hash_[0]];
    last_ = first_ + hash_[1];
  }

  constexpr size_type bucket_count() const noexcept { return hash_[0]; }

  mapped_type* find(key_type k) {
    const uint32_t hash = elf_hash_(reinterpret_cast<std::uint8_t*>(k));
    for (uint32_t i = bucket_[hash % bucket_count()]; i; i = first_[i]) {
      if (strcmp(k, strtab_ + symtab_[i].st_name) == 0) {
        return &symtab_[i];
      }
    }
  }

  sysv_iterator<std::uint32_t> begin(size_type n) { return {first_, bucket_, n}; }

 private:
  using bucket_iterator = pointer;

  pointer hash_;

  key_type strtab_;
  mapped_type* symtab_;
  bucket_iterator bucket_;
  chain_iterator first_;
  chain_iterator last_;
};

template <typename Key, typename T>
class symbols {
 public:
  using key_type              = Key;
  using mapped_type           = T;
  using value_type            = std::pair<const Key, T>;
  using size_type             = std::uint64_t;
  using difference_type       = std::ptrdiff_t;

  using iterator              = mapped_type*;
  using const_iterator        = const iterator;

  using hash_table            = gnu_hash<mapped_type>;
  using hash_table_pointer    = typename hash_table::pointer;

  using local_iterator        = iterator;
  using const_local_iterator  = const local_iterator;

  symbols(key_type strtab, mapped_type* symtab, void* hash)
      : strtab_{strtab}, first_{symtab}, hash_{static_cast<hash_table_pointer>(hash)} {
    last_ = first_ + hash_.size();
  }

  // iterators
  iterator begin() noexcept { return first_; }
  const_iterator begin() const noexcept { return first_; }
  iterator end() noexcept { return last_; }
  const_iterator end() const noexcept { return last_; }

  // capacity
  size_type size() const noexcept { return std::distance(begin(), end()); }

  // set operations
  iterator find(const key_type k) noexcept;

  // bucket interface
  local_iterator begin(size_type n) noexcept { return first_ + std::distance(hash_.begin(), hash_.begin(n)); }
  local_iterator end(size_type n) noexcept { return first_ + std::distance(hash_.begin(), hash_.end(n)); }
  size_type bucket_count() const noexcept { return hash_.bucket_count(); }
  size_type bucket_size(size_type n) const noexcept { return hash_.bucket_size(); }
  size_type bucket(key_type k) const noexcept;

 private:
  key_type strtab_;
  iterator first_;
  iterator last_;

  hash_table hash_;
};

template <typename Key, typename T>
inline auto symbols<Key, T>::find(const key_type k) noexcept -> iterator {
  std::uint32_t hash = gnu_hash_(reinterpret_cast<const std::uint8_t*>(k));
  auto n = hash_.bucket(hash);

  auto chain = hash_.begin(n);
  for (auto iter = begin(n); iter != end(n); ++iter, ++chain) {
    if ((hash | 1) == (*chain | 1)) {
      if (!std::strcmp(k, &strtab_[iter->st_name])) {
        return iter;
      }
    }
  }
  return end();
}

template <typename Key, typename T>
inline auto symbols<Key, T>::bucket(key_type key) const noexcept -> size_type {
  return hash_.bucket(gnu_hash_(reinterpret_cast<const std::uint8_t*>(key)));
}

}  // namespace ELF
}  // namespace binlab

#endif  // !BINLAB_SYMBOLS_ELF_SYMBOLS_H_
