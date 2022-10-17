// versioning.h:

#ifndef BINLAB_VERSIONING_H_
#define BINLAB_VERSIONING_H_

#include <iterator>
#include <memory>

template <typename Verdef>
class const_verdef_iterator {
 public:
  using iterator_category   = std::forward_iterator_tag;

  using value_type          = Verdef;
  using pointer             = const value_type*;
  using reference           = const value_type&;

  const_verdef_iterator() noexcept : ptr_{} {}
  const_verdef_iterator(Verdef* ptr) noexcept : ptr_{ptr} {}

  reference operator*() const noexcept {
    return *ptr_;
  }
  pointer operator->() const noexcept {
    return std::pointer_traits<pointer>::pointer_to(**this);
  }

  const_verdef_iterator& operator++() noexcept {
    if (ptr_->vd_next) {
      ptr_ = reinterpret_cast<value_type*>(reinterpret_cast<char*>(ptr_) + ptr_->vd_next);
    } else {
      ptr_ = nullptr;
    }
    return *this;
  }
  const_verdef_iterator& operator++(int) noexcept {
    const_verdef_iterator tmp = *this;
    ++*this;
    return tmp;
  }

  bool operator==(const const_verdef_iterator& rhs) const noexcept {
    return ptr_ == rhs.ptr_;
  }
  bool operator!=(const const_verdef_iterator& rhs) const noexcept {
    return !(*this == rhs);
  }

 private:
  value_type* ptr_;
};

template <typename Verdef>
class verdef_iterator : public const_verdef_iterator<Verdef> {
 public:
  using iterator_category = std::forward_iterator_tag;

  using value_type        = Verdef;
  using pointer           = value_type*;
  using reference         = value_type&;

#if defined(__GNUC__)
  verdef_iterator() : const_verdef_iterator<Verdef>{} {}
  verdef_iterator(Verdef* ptr) : const_verdef_iterator<Verdef>{ptr} {}
#else
  using const_verdef_iterator<Verdef>::const_verdef_iterator;
#endif  // !__GNUC__

  reference operator*() const noexcept {
    return const_cast<reference>(const_verdef_iterator<value_type>::operator*());
  }
  pointer operator->() const noexcept {
    return std::pointer_traits<pointer>::pointer_to(**this);
  }

  verdef_iterator& operator++() noexcept {
    const_verdef_iterator<value_type>::operator++();
    return *this;
  }

  verdef_iterator operator++(int) noexcept {
    verdef_iterator tmp = *this;
    const_verdef_iterator<value_type>::operator++();
    return tmp;
  }
};

template <typename Verdaux>
class const_verdaux_iterator {
 public:
  using iterator_category   = std::forward_iterator_tag;

  using value_type          = Verdaux;
  using pointer             = const value_type*;
  using reference           = const value_type&;

  const_verdaux_iterator() noexcept : ptr_{} {}
  const_verdaux_iterator(Verdaux* ptr) noexcept : ptr_{ptr} {}

  reference operator*() const noexcept {
    return *ptr_;
  }
  pointer operator->() const noexcept {
    return std::pointer_traits<pointer>::pointer_to(**this);
  }

  const_verdaux_iterator& operator++() noexcept {
    if (ptr_->vda_next) {
      ptr_ = reinterpret_cast<value_type*>(reinterpret_cast<char*>(ptr_) + ptr_->vda_next);
    } else {
      ptr_ = nullptr;
    }
    return *this;
  }
  const_verdaux_iterator& operator++(int) noexcept {
    const_verdaux_iterator tmp = *this;
    ++*this;
    return tmp;
  }

  bool operator==(const const_verdaux_iterator& rhs) const noexcept {
    return ptr_ == rhs.ptr_;
  }
  bool operator!=(const const_verdaux_iterator& rhs) const noexcept {
    return !(*this == rhs);
  }

 private:
  value_type* ptr_;
};

template <typename Verdaux>
class verdaux_iterator : public const_verdaux_iterator<Verdaux> {
 public:
  using iterator_category = std::forward_iterator_tag;

  using value_type        = Verdaux;
  using pointer           = value_type*;
  using reference         = value_type&;

#if defined(__GNUC__)
  verdaux_iterator() : const_verdaux_iterator<Verdaux>{} {}
  verdaux_iterator(Verdaux* ptr) : const_verdaux_iterator<Verdaux>{ptr} {}
#else
  using const_verdaux_iterator<Verdaux>::const_verdaux_iterator;
#endif  // !__GNUC__

  reference operator*() const noexcept {
    return const_cast<reference>(const_verdaux_iterator<value_type>::operator*());
  }
  pointer operator->() const noexcept {
    return std::pointer_traits<pointer>::pointer_to(**this);
  }

  verdaux_iterator& operator++() noexcept {
    const_verdaux_iterator<value_type>::operator++();
    return *this;
  }

  verdaux_iterator operator++(int) noexcept {
    verdaux_iterator tmp = *this;
    const_verdaux_iterator<value_type>::operator++();
    return tmp;
  }
};

template <typename Verneed>
class const_verneed_iterator {
 public:
  using iterator_category   = std::forward_iterator_tag;

  using value_type          = Verneed;
  using pointer             = const value_type*;
  using reference           = const value_type&;

  const_verneed_iterator() noexcept : ptr_{} {}
  const_verneed_iterator(Verneed* ptr) noexcept : ptr_{ptr} {}

  reference operator*() const noexcept {
    return *ptr_;
  }
  pointer operator->() const noexcept {
    return std::pointer_traits<pointer>::pointer_to(**this);
  }

  const_verneed_iterator& operator++() noexcept {
    if (ptr_->vn_next) {
      ptr_ = reinterpret_cast<value_type*>(reinterpret_cast<char*>(ptr_) + ptr_->vn_next);
    } else {
      ptr_ = nullptr;
    }
    return *this;
  }
  const_verneed_iterator& operator++(int) noexcept {
    const_verneed_iterator tmp = *this;
    ++*this;
    return tmp;
  }

  bool operator==(const const_verneed_iterator& rhs) const noexcept {
    return ptr_ == rhs.ptr_;
  }
  bool operator!=(const const_verneed_iterator& rhs) const noexcept {
    return !(*this == rhs);
  }

 private:
  value_type* ptr_;
};

template <typename Verneed>
class verneed_iterator : public const_verneed_iterator<Verneed> {
 public:
  using iterator_category = std::forward_iterator_tag;

  using value_type        = Verneed;
  using pointer           = value_type*;
  using reference         = value_type&;

#if defined(__GNUC__)
  verneed_iterator() : const_verneed_iterator<Verneed>{} {}
  verneed_iterator(Verneed* ptr) : const_verneed_iterator<Verneed>{ptr} {}
#else
  using const_verneed_iterator<Verneed>::const_verneed_iterator;
#endif  // !__GNUC__

  reference operator*() const noexcept {
    return const_cast<reference>(const_verneed_iterator<value_type>::operator*());
  }
  pointer operator->() const noexcept {
    return std::pointer_traits<pointer>::pointer_to(**this);
  }

  verneed_iterator& operator++() noexcept {
    const_verneed_iterator<value_type>::operator++();
    return *this;
  }

  verneed_iterator operator++(int) noexcept {
    verneed_iterator tmp = *this;
    const_verneed_iterator<value_type>::operator++();
    return tmp;
  }
};

template <typename Vernaux>
class const_vernaux_iterator {
 public:
  using iterator_category   = std::forward_iterator_tag;

  using value_type          = Vernaux;
  using pointer             = const value_type*;
  using reference           = const value_type&;

  const_vernaux_iterator() noexcept : ptr_{} {}
  const_vernaux_iterator(Vernaux* ptr) noexcept : ptr_{ptr} {}

  reference operator*() const noexcept {
    return *ptr_;
  }
  pointer operator->() const noexcept {
    return std::pointer_traits<pointer>::pointer_to(**this);
  }

  const_vernaux_iterator& operator++() noexcept {
    if (ptr_->vna_next) {
      ptr_ = reinterpret_cast<value_type*>(reinterpret_cast<char*>(ptr_) + ptr_->vna_next);
    } else {
      ptr_ = nullptr;
    }
    return *this;
  }
  const_vernaux_iterator& operator++(int) noexcept {
    const_vernaux_iterator tmp = *this;
    ++*this;
    return tmp;
  }

  bool operator==(const const_vernaux_iterator& rhs) const noexcept {
    return ptr_ == rhs.ptr_;
  }
  bool operator!=(const const_vernaux_iterator& rhs) const noexcept {
    return !(*this == rhs);
  }

 private:
  value_type* ptr_;
};

template <typename Vernaux>
class vernaux_iterator : public const_vernaux_iterator<Vernaux> {
 public:
  using iterator_category = std::forward_iterator_tag;

  using value_type        = Vernaux;
  using pointer           = value_type*;
  using reference         = value_type&;

#if defined(__GNUC__)
  vernaux_iterator() : const_vernaux_iterator<Vernaux>{} {}
  vernaux_iterator(Vernaux* ptr) : const_vernaux_iterator<Vernaux>{ptr} {}
#else
  using const_vernaux_iterator<Vernaux>::const_vernaux_iterator;
#endif  // !__GNUC__

  reference operator*() const noexcept {
    return const_cast<reference>(const_vernaux_iterator<value_type>::operator*());
  }
  pointer operator->() const noexcept {
    return std::pointer_traits<pointer>::pointer_to(**this);
  }

  vernaux_iterator& operator++() noexcept {
    const_vernaux_iterator<value_type>::operator++();
    return *this;
  }

  vernaux_iterator operator++(int) noexcept {
    vernaux_iterator tmp = *this;
    const_vernaux_iterator<value_type>::operator++();
    return tmp;
  }
};

#endif  // !BINLAB_VERSIONING_H_
