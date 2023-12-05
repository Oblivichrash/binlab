// base_relocation.cpp:

#include "base_relocation.h"

#include <iomanip>

using namespace binlab::COFF;

class base_relocation_iterator {
 public:
  using iterator_category   = std::forward_iterator_tag;
  using value_type          = IMAGE_BASE_RELOCATION;
  using reference           = std::add_lvalue_reference_t<value_type>;
  using pointer             = std::add_pointer_t<value_type>;

  base_relocation_iterator(void* ptr) : ptr_{static_cast<char*>(ptr)} {}

  [[nodiscard]] reference operator*() noexcept { return reinterpret_cast<reference>(*ptr_); }
  [[nodiscard]] pointer operator->() noexcept { return std::pointer_traits<pointer>::pointer_to(**this); }

  base_relocation_iterator& operator++() noexcept {
    ptr_ += (*this)->SizeOfBlock;
    return *this;
  }

  [[nodiscard]] base_relocation_iterator operator++(int) noexcept {
    auto tmp = *this;
    ++(*this);
    return tmp;
  }

 private:
  char* ptr_;
};

class base_relocation_entry_iterator {
 public:
  using iterator_category   = std::forward_iterator_tag;
  using value_type          = IMAGE_BASE_RELOCATION;
  using reference           = std::add_lvalue_reference_t<value_type>;
  using pointer             = std::add_pointer_t<value_type>;

  base_relocation_entry_iterator(void* ptr) : ptr_{static_cast<char*>(ptr)} {}

  [[nodiscard]] reference operator*() noexcept { return reinterpret_cast<reference>(*ptr_); }
  [[nodiscard]] pointer operator->() noexcept { return std::pointer_traits<pointer>::pointer_to(**this); }

  base_relocation_entry_iterator& operator++() noexcept {
    ptr_ += (*this)->SizeOfBlock;
    return *this;
  }

  [[nodiscard]] base_relocation_entry_iterator operator++(int) noexcept {
    auto tmp = *this;
    ++(*this);
    return tmp;
  }

 private:
  char* ptr_;
};

std::ostream& binlab::COFF::operator<<(std::ostream& os, const IMAGE_BASE_RELOCATION& relocation) {
  os << std::setw(8) << relocation.VirtualAddress;
  os << std::setw(8) << relocation.SizeOfBlock;
  return os;
}

std::ostream& binlab::COFF::operator<<(std::ostream& os, const BASE_RELOCATION_ENTRY& relocation) {
  os << std::setw(8) << relocation.Type;
  os << std::setw(8) << relocation.Offset;
  return os;
}

std::ostream& binlab::COFF::dump(std::ostream& os, IMAGE_SECTION_HEADER&, char*, IMAGE_BASE_RELOCATION& relocation) {
  for (auto iter = base_relocation_iterator{&relocation}; iter->VirtualAddress; ++iter) {
    os << *iter << '\n';
    for (auto iter2 = begin(*iter); iter2 != end(*iter); ++iter2) {
      os << *iter2 << '\n';
    }
    os << '\n';
  }
  return os;
}