// bin_iterator.h

#ifndef BINLAB_ITERATOR_BIN_ITERATOR_H_
#define BINLAB_ITERATOR_BIN_ITERATOR_H_

#include <cstdint>

namespace binlab {
namespace ELF {

//template <typename Derived, typename T, typename Category>
//class bucket_const_iterator_facade {
// public:
//  using iterator_category = Category;
//
//  using value_type      = T;
//  using size_type       = std::size_t;
//  using difference_type = std::ptrdiff_t;
//  using pointer         = const value_type*;
//  using reference       = const value_type&;
//
//  using bucket_type     = std::uint32_t;
//  using bucket_pointer  = const bucket_type*;
//
//  reference operator*() const { return derived().dereference(); }
//  Derived& operator++() {
//    derived().increment();
//    return asDerived();
//  }
//  Derived operator++(int) {
//    Derived result(derived());
//    derived().increment();
//    return result;
//  }
//  friend bool operator==(bucket_const_iterator_facade& lhs, bucket_const_iterator_facade& rhs) {
//    return lhs.derived().equals(rhs.derived());
//  }
//
// private:
//  Derived& derived() noexcept { return *static_cast<Derived*>(this); }
//  Derived const& derived() const noexcept { return *static_cast<Derived const*>(this); }
//};
//
//template <typename T>
//class my_sysv_const_iterator
//    : public bucket_const_iterator_facade<my_sysv_const_iterator<T>, T, std::forward_iterator_tag> {
// public:
//  T& dereference() const noexcept { return *ptr_; }
//  void increment() noexcept { ptr_ = &chain_[bucket_[*ptr_]]; }
//  bool equals(const my_sysv_const_iterator& rhs) const noexcept { return ptr_ == rhs.ptr_; }
//
// protected:
//  bucket_pointer bucket_;
//
//  pointer chain_;
//  pointer ptr_;
//};

template <typename T>
class const_bucket_iterator {
 public:
  using iterator_category = std::forward_iterator_tag;

  using value_type      = T;
  using size_type       = std::size_t;
  using difference_type = std::ptrdiff_t;
  using pointer         = const value_type*;
  using reference       = const value_type&;

  using bucket_type     = std::uint32_t;
  using bucket_pointer  = const bucket_type*;

  const_bucket_iterator(pointer chain, bucket_pointer bucket, size_type n)
      : chain_{chain}, bucket_{bucket}, ptr_{&chain[bucket[n]]} {}

  reference operator*() const noexcept { return *ptr_; }
  pointer operator->() const noexcept { return std::pointer_traits<pointer>::pointer_to(**this); }

  bool operator==(const const_bucket_iterator& rhs) const noexcept { return ptr_ == rhs.ptr_; }
  bool operator!=(const const_bucket_iterator& rhs) const noexcept { return !(*this == rhs); }

  difference_type position() const noexcept { return std::distance(chain_, ptr_); }

 protected:
  bucket_pointer bucket_;

  pointer chain_;
  pointer ptr_;
};

template <typename T>
class const_sysv_iterator : public const_bucket_iterator<T> {
 public:
  const_sysv_iterator(pointer chain, bucket_pointer bucket, size_type n)
      : base{chain, bucket, n} {}

  const_sysv_iterator& operator++() noexcept {
    ptr_ = &chain_[bucket_[*ptr_]];
    return *this;
  }

  const_sysv_iterator operator++(int) noexcept {
    const_sysv_iterator tmp = *this;
    ptr_ = &chain_[bucket_[*ptr_]];
    return tmp;
  }

  explicit operator bool() const noexcept { return **this; }

 private:
  using base            = const_bucket_iterator<T>;
};

template <typename T>
class sysv_iterator : public const_sysv_iterator<T> {
 public:
  using pointer         = value_type*;
  using reference       = value_type&;

  sysv_iterator(pointer chain, bucket_pointer bucket, size_type n)
      : base{chain, bucket, n} {}

  reference operator*() const noexcept { return const_cast<reference>(base::operator*()); }
  pointer operator->() const noexcept { return std::pointer_traits<pointer>::pointer_to(**this); }

  sysv_iterator& operator++() noexcept {
    base::operator++();
    return *this;
  }

  sysv_iterator operator++(int) noexcept {
    sysv_iterator tmp = *this;
    base::operator++();
    return tmp;
  }

 private:
  using base            = const_sysv_iterator<T>;
};

template <typename T>
class const_gnu_iterator : public const_bucket_iterator<T> {
 public:
  const_gnu_iterator(pointer chain, bucket_pointer bucket, size_type n)
      : base{chain, bucket, n} {}

  const_gnu_iterator& operator++() noexcept {
    ++ptr_;
    return *this;
  }

  const_gnu_iterator operator++(int) noexcept {
    const_gnu_iterator tmp = *this;
    ++ptr_;
    return tmp;
  }

  explicit operator bool() const noexcept { return !(**this | 1); }

 private:
  using base            = const_bucket_iterator<T>;
};

template <typename T>
class gnu_iterator : public const_gnu_iterator<T> {
 public:
  using pointer         = value_type*;
  using reference       = value_type&;

  gnu_iterator(pointer chain, bucket_pointer bucket, size_type n)
      : base{chain, bucket, n} {}

  reference operator*() const noexcept { return const_cast<reference>(base::operator*()); }
  pointer operator->() const noexcept { return std::pointer_traits<pointer>::pointer_to(**this); }

  gnu_iterator& operator++() noexcept {
    base::operator++();
    return *this;
  }

  gnu_iterator operator++(int) noexcept {
    gnu_iterator tmp = *this;
    base::operator++();
    return tmp;
  }

 private:
  using base            = const_gnu_iterator<T>;
};

}  // namespace ELF
}  // namespace binlab

#endif  // !BINLAB_ITERATOR_BIN_ITERATOR_H_
