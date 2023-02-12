// symbols.h: ELF symbols, for more about DT_GNU_HASH, see: https://flapenguin.me/elf-dt-gnu-hash
//

#ifndef BINLAB_SYMBOLS_ELF_SYMBOLS_H_
#define BINLAB_SYMBOLS_ELF_SYMBOLS_H_

#include <cstdint>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <utility>

#include "binlab/BinaryFormat/ELF.h"

#include "iterator_facade.h"

namespace binlab {
namespace ELF {

template <typename Value, typename Reference = Value&, typename Difference = std::ptrdiff_t>
class sysv_bucket_iterator : public iterator_facade<sysv_bucket_iterator<Value, Reference, Difference>, Value, std::forward_iterator_tag, Reference, Difference> {
 public:
  using iterator_category   = std::forward_iterator_tag;

  using value_type          = Value;
  using difference_type     = Difference;
  using pointer             = std::add_pointer_t<std::remove_reference_t<Reference>>;
  using reference           = Reference;

  sysv_bucket_iterator(pointer symtab, std::uint32_t* chain, difference_type index)
      : symtab_{symtab}, chain_{chain}, index_{index} {}

  explicit operator bool() const noexcept { return index_ != STN_UNDEF; }

 private:
  reference dereference() const noexcept { return symtab_[index_]; }
  void increment() noexcept { index_ = chain_[index_]; }
  bool equals(const sysv_bucket_iterator& rhs) const noexcept { return index_ == rhs.index_; }

  pointer               symtab_;
  std::uint32_t*        chain_;
  difference_type       index_;

  friend class iterator_facade_access;
};

template <typename T>
class sysv_hash_table {
 public:
  using key_type                = char*;
  using mapped_type             = T;
  //using value_type            = std::pair<const Key, T>;
  using size_type               = std::uint64_t;
  using difference_type         = std::ptrdiff_t;

  using iterator                = mapped_type*;
  using const_iterator          = const mapped_type*;

  using local_iterator          = sysv_bucket_iterator<T>;
  using const_local_iterator    = sysv_bucket_iterator<T, const T&>;

  // hash
  using argument_type           = key_type;
  using result_type             = std::uint32_t;

  //
  using bucket_type             = std::uint32_t;
  using bucket_pointer          = bucket_type*;

  using chain_type              = std::uint32_t;
  using chain_pointer           = chain_type*;
  using chain_iterator          = chain_type*;

  sysv_hash_table(iterator symtab, char* strtab, void* hash)
      : first_{symtab}, strtab_ {strtab}, hash_{static_cast<std::uint32_t*>(hash)} {
    last_ = first_ + chain_count();

    bucket_ = reinterpret_cast<bucket_pointer>(&hash_[2]);
    chain_ = reinterpret_cast<chain_pointer>(&bucket_[bucket_count()]);
  }

  // iterators
  constexpr iterator begin() noexcept { return first_; }
  constexpr const_iterator begin() const noexcept { return first_; }
  constexpr const_iterator cbegin() const noexcept { return first_; }
  constexpr iterator end() noexcept { return last_; }
  constexpr const_iterator end() const noexcept { return last_; }
  constexpr const_iterator cend() const noexcept { return last_; }

  // capacity
  constexpr size_type size() const noexcept { return std::distance(first_, last_); }
  
  // set operations
  constexpr const_iterator find(const char* k) const noexcept {
    auto n = hash_value(k) % bucket_count();
    for (auto iter = begin(n); iter != end(n); ++iter) {
      if (!std::strcmp(k, &strtab_[iter->st_name])) {
        return std::pointer_traits<const_iterator>::pointer_to(*iter);
      }
    }
    return last_;
  }

  // bucket interface
  constexpr size_type bucket_count() const noexcept { return hash_[0]; }
  constexpr size_type bucket(const char* k) const noexcept { return hash_value(k) % bucket_count(); }
  constexpr local_iterator begin(size_type n) noexcept { return {first_, chain_, bucket_[n]}; }
  constexpr const_local_iterator begin(size_type n) const noexcept { return {first_, chain_, bucket_[n]}; }
  constexpr const_local_iterator cbegin(size_type n) const noexcept { return {first_, chain_, bucket_[n]}; }
  constexpr local_iterator end(size_type) noexcept { return {first_, chain_, STN_UNDEF}; }
  constexpr const_local_iterator end(size_type) const noexcept { return {first_, chain_, STN_UNDEF}; }
  constexpr const_local_iterator cend(size_type) const noexcept { return {first_, chain_, STN_UNDEF}; }

  // hash
  static constexpr result_type hash_value(const char* k) noexcept {
    auto name = reinterpret_cast<const std::uint8_t*>(k);
    std::uint32_t h = 0, g = 0;
    for (; *name; name++) {
      h = (h << 4) + *name;
      g = h & 0xf0000000;
      if (g) {
        h ^= g >> 24;
      }
      h &= ~g;
    }
    return result_type{h};
  }

 protected:
  constexpr size_type chain_count() const noexcept { return hash_[1]; }

 private:
  iterator first_;
  iterator last_;

  key_type strtab_;
  std::uint32_t* hash_;
  bucket_pointer bucket_;
  chain_pointer chain_;
};

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

template <typename Value, typename Reference = Value&, typename Difference = std::ptrdiff_t>
class gnu_bucket_iterator {
 public:
  using iterator_category   = std::forward_iterator_tag;

  using value_type          = Value;
  using difference_type     = Difference;
  using pointer             = std::add_pointer_t<std::remove_reference_t<Reference>>;
  using reference           = Reference;

  gnu_bucket_iterator(pointer symtab, difference_type symoff, std::uint32_t* chain, difference_type index)
      : symtab_{symtab}, chain_{chain - symoff}, index_{index} {}

  reference operator*() const noexcept { return dereference(); }
  pointer operator->() const noexcept { return std::pointer_traits<pointer>::pointer_to(**this); }

  gnu_bucket_iterator& operator++() noexcept {
    increment();
    return *this;
  }

  gnu_bucket_iterator operator++(int) noexcept {
    gnu_bucket_iterator tmp = *this;
    increment();
    return tmp;
  }

  bool operator==(const gnu_bucket_iterator& rhs) const noexcept { return equals(rhs); }
  bool operator!=(const gnu_bucket_iterator& rhs) const noexcept { return !(*this == rhs); }
  bool operator==(const std::uint32_t& hash) const noexcept { return (chain_[index_] | 1) == (hash | 1); }
  bool operator!=(const std::uint32_t& hash) const noexcept { return !(*this == hash); }
  explicit operator bool() const noexcept { return index_ != STN_UNDEF; }

 private:
  reference dereference() const noexcept { return symtab_[index_]; }
  void increment() noexcept {
    if (chain_[index_++] & 1) {
      index_ = STN_UNDEF;
    }
  }
  bool equals(const gnu_bucket_iterator& rhs) const noexcept {return index_ == rhs.index_;}

  pointer               symtab_;
  std::uint32_t*        chain_;
  difference_type       index_;
};

template <typename T>
class gnu_hash_table {
 public:
  using key_type              = char*;
  using mapped_type           = T;
  //using value_type            = std::pair<const Key, T>;
  using size_type             = std::uint64_t;
  using difference_type       = std::ptrdiff_t;

  using iterator              = mapped_type*;
  using const_iterator        = const mapped_type*;

  using local_iterator          = gnu_bucket_iterator<T>;
  using const_local_iterator    = gnu_bucket_iterator<T, const T&>;

  // hash
  using argument_type         = key_type;
  using result_type           = std::uint32_t;

  //
  using bloom_type            = typename bloom_traits<T>::value_type;
  using bloom_pointer         = bloom_type*;

  using bucket_type           = std::uint32_t;
  using bucket_pointer        = bucket_type*;

  using chain_type            = std::uint32_t;
  using chain_pointer         = chain_type*;
  using chain_iterator        = chain_type*;

  gnu_hash_table(iterator symtab, char* strtab, void* hash)
      : first_{symtab}, strtab_{strtab}, hash_{static_cast<std::uint32_t*>(hash)} {
    bloom_ = reinterpret_cast<bloom_pointer>(&hash_[4]);
    bucket_ = reinterpret_cast<bucket_pointer>(&bloom_[bloom_count()]);
    chain_ = reinterpret_cast<chain_pointer>(&bucket_[bucket_count()]);
    chain_size_ = std::distance(chain_, chain_end(chain_, bucket_, bucket_count(), symbol_offset()));

    last_ = first_ + chain_size_ + symbol_offset();
  }

  // iterators
  constexpr iterator begin() noexcept { return first_; }
  constexpr const_iterator begin() const noexcept { return first_; }
  constexpr iterator end() noexcept { return last_; }
  constexpr const_iterator end() const noexcept { return last_; }

  // capacity
  constexpr size_type size() const noexcept { return std::distance(first_, last_); }

  // set operations
  constexpr const_iterator find(const char* k) const noexcept {
    auto hash = hash_value(k);
    auto n = hash % bucket_count();
    if (filter(hash)) {
      for (auto iter = begin(n); iter != end(n); ++iter) {
        if (hash == iter && !std::strcmp(k, &strtab_[iter->st_name])) {
          return std::pointer_traits<const_iterator>::pointer_to(*iter);
        }
      }
    }
    return std::pointer_traits<const_iterator>::pointer_to(*end(n));
  }

  // bucket interface
  constexpr size_type bucket_count() const noexcept { return hash_[0]; }
  constexpr size_type bucket(const char* k) const noexcept {
    auto hash = hash_value(k);
    auto n = hash % bucket_count();
    return filter(hash) ? hash % bucket_count() : bucket_count();
  }
  constexpr local_iterator begin(size_type n) noexcept { return {first_, symbol_offset(), chain_, bucket_[n]}; }
  constexpr const_local_iterator begin(size_type n) const noexcept { return {first_, symbol_offset(), chain_, bucket_[n]}; }
  constexpr const_local_iterator cbegin(size_type n) const noexcept { return {first_, symbol_offset(), chain_, bucket_[n]}; }
  constexpr local_iterator end(size_type) noexcept { return {first_, symbol_offset(), chain_, STN_UNDEF}; }
  constexpr const_local_iterator end(size_type) const noexcept { return {first_, symbol_offset(), chain_, STN_UNDEF}; }
  constexpr const_local_iterator cend(size_type) const noexcept { return {first_, symbol_offset(), chain_, STN_UNDEF}; }

  // hash
  static constexpr result_type hash_value(const char* k) noexcept {
    auto name = reinterpret_cast<const std::uint8_t*>(k);
    std::uint32_t h = 5381;
    for (; *name; name++) {
      h = (h << 5) + h + *name;
    }
    return result_type{h};
  }

 protected:
  constexpr difference_type symbol_offset() const noexcept { return hash_[1]; }
  constexpr size_type bloom_count() const noexcept { return hash_[2]; }
  constexpr size_type bloom_shift() const noexcept { return hash_[3]; }

  constexpr size_type chain_count() const noexcept { return chain_size_; }

  // chain
  static constexpr chain_pointer chain_end(chain_pointer chain, bucket_pointer bucket, size_type bucket_size, difference_type symbol_offset) {
    auto first = bucket, last = bucket + bucket_size;
    while (--last != first) {
      if (*last > symbol_offset) {  // find last non-empty bucket.
        break;
      }
    }

    auto iter = chain + (*last - symbol_offset);
    while (!(*iter++ & 1))
      ;
    return iter;
  }

  // bloom
  static constexpr size_type bits = sizeof(bloom_type) * 8;

  difference_type bloom_hash1(result_type hash) const noexcept { return hash % bits; }
  difference_type bloom_hash2(result_type hash) const noexcept { return (hash >> bloom_shift()) % bits; }

  constexpr bool filter(result_type hash) const noexcept {
    bloom_type bloom = bloom_[(hash / bits) % bloom_count()];
    bloom_type mask = 0 | bloom_type{1} << bloom_hash1(hash) | bloom_type{1} << bloom_hash2(hash);
    return (bloom & mask) == mask;
  }

 private:
  iterator first_;
  iterator last_;

  key_type strtab_;
  std::uint32_t* hash_;
  bloom_pointer bloom_;
  bucket_pointer bucket_;
  chain_pointer chain_;
  size_type chain_size_;
};

}  // namespace ELF
}  // namespace binlab

#endif  // !BINLAB_SYMBOLS_ELF_SYMBOLS_H_
