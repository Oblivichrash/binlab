// sections.h:

#ifndef BINLAB_SECTIONS_H_
#define BINLAB_SECTIONS_H_

#include <elf.h>

#include <algorithm>
#include <cstdint>
#include <iterator>
#include <type_traits>

//#include "binlab/Traits/bin_traits.h"

namespace binlab {

enum class address_mode { kFileOffset, kVirtualOffset };

template <typename T, address_mode mode>
struct section_traits;

template <>
struct section_traits<Elf64_Ehdr, address_mode::kFileOffset> {
  using value_type        = Elf64_Shdr;
  using reference         = value_type&;
  using const_reference   = const value_type&;
  using size_type         = std::uint64_t;
  using difference_type   = std::ptrdiff_t;
  using iterator          = value_type*;
  using const_iterator    = const iterator;

  using address_type      = std::uint64_t;
  using header_type       = Elf64_Ehdr;

  static constexpr iterator first_section_header(const header_type& header, void* base) {
    return reinterpret_cast<iterator>(static_cast<char*>(base)[header.e_shoff]);
  }
  static constexpr size_type section_num(const header_type& header) {
    return header.e_shnum;
  }

  static constexpr address_type offset(const_reference section) {
    return section.sh_offset;
  }
  static constexpr size_type size(const_reference section) {
    return section.sh_size;
  }
  static constexpr size_type alignment(const_reference section) {
    return section.sh_addralign;
  }
};

template <>
struct section_traits<Elf64_Ehdr, address_mode::kVirtualOffset> {
  using value_type        = Elf64_Shdr;
  using reference         = value_type&;
  using const_reference   = const value_type&;
  using size_type         = std::uint64_t;
  using difference_type   = std::ptrdiff_t;
  using iterator          = value_type*;
  using const_iterator    = const iterator;

  using address_type      = std::uint64_t;
  using header_type       = Elf64_Ehdr;

  static constexpr iterator first_section_header(const header_type& header, void* base) {
    return reinterpret_cast<iterator>(static_cast<char*>(base)[header.e_shoff]);
  }
  static constexpr size_type section_num(const header_type& header) {
    return header.e_shnum;
  }

  static constexpr address_type offset(const_reference section) {
    return section.sh_addr;
  }
  static constexpr size_type size(const_reference section) {
    return section.sh_size;
  }
  static constexpr size_type alignment(const_reference section) {
    return section.sh_addralign;
  }
};

template <typename T, typename Traits>
class sections {
 public:
  using traits_type         = traits::section_traits<T>;

  using value_type          = typename traits_type::value_type;
  using iterator            = typename traits_type::iterator;
  using const_iterator      = typename traits_type::const_iterator;
  using size_type           = typename traits_type::size_type;

  using address_type        = typename traits_type::address_type;

  sections(void* base, const_iterator first, const_iterator last) : base_{base} first_{first}, last_{last} {}
  sections(void* base, const_iterator first, size_type size) : sections{base, first, first + size} {}
  sections(const T& header, void* base) : sections{base, traits_type::first_section_header(header, base), traits_type::section_num(header)} {}

  // iterators
  iterator begin() noexcept { return first_; }
  const_iterator begin() const noexcept { return first_; }
  const_iterator cbegin() const noexcept { return first_; }
  iterator end() noexcept { return last_; }
  const_iterator end() const noexcept { return last_; }
  const_iterator cend() const noexcept { return last_; }

  // capacity
  size_type size() const noexcept { return last_ - first_; }
  bool empty() const noexcept { return last_ == first_; }

  // section
  size_type section(const address_type& address) const noexcept {
    const auto section_count = size();
    for (std::size_t i = 0; i < section_count; ++i) {
      auto section_base = traits_type::offset(first_[i]);
      auto section_size = traits_type::size(first_[i]);
      if (section_base <= address && address < section_base + section_size) {
        return i;
      }
    }
    return section_count;
  }
  size_type section_size(size_type n) const noexcept {
    return traits_type::size(first_[n]);
  }

 private:
  char* base_;
  const_iterator first_;
  const_iterator last_;
};

template <typename SegmentIt, typename AddressType>
auto virtual_offset_to_file_offset(SegmentIt first, SegmentIt last, AddressType address) -> AddressType {
  using Traits          = traits::segment_traits<typename std::iterator_traits<SegmentIt>::value_type>;

  using difference_type = typename Traits::difference_type;
  using address_type    = typename Traits::address_type;

  static_assert(std::is_convertible_v<AddressType, address_type>, "address type error");

  address_type result = 0;
  for (auto iter = first; iter != last; ++iter) {
    address_type base = Traits::virtual_offset(*iter);
    if (std::clamp(address, base, base + Traits::virtual_size(*iter)) == address) {
      difference_type offset = address - base;
      result = Traits::file_offset(*iter) + offset;
    }
  }
  return result;
}

template <typename SegmentIt, typename AddressType>
auto file_offset_to_virtual_offset(SegmentIt first, SegmentIt last, AddressType address) -> AddressType {
  using Traits          = traits::segment_traits<typename std::iterator_traits<SegmentIt>::value_type>;

  using difference_type = typename Traits::difference_type;
  using address_type    = typename Traits::address_type;

  static_assert(std::is_convertible_v<AddressType, address_type>, "address type error");

  address_type result = 0;
  for (auto iter = first; iter != last; ++iter) {
    address_type base = Traits::file_offset(*iter);
    if (std::clamp(address, base, base + Traits::file_size(*iter)) == address) {
      difference_type offset = address - base;
      result = Traits::virtual_offset(*iter) + offset;
    }
  }
  return result;
}

}  // namespace binlab

#endif  // !BINLAB_SECTIONS_H_
