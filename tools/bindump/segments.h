// segments.h:

#ifndef BINLAB_SEGMENTS_H_
#define BINLAB_SEGMENTS_H_

#include <algorithm>
#include <cstdint>
#include <iterator>
#include <type_traits>

#include "binlab/Traits/bin_traits.h"

namespace binlab {

namespace address {

template <typename Rep, typename Type, typename Segment>
class addr {
 public:
  using traits_type         = traits::segment_traits<Segment>;

  using value_type          = Rep;
  using iterator            = typename traits_type::iterator;
  using const_iterator      = typename traits_type::const_iterator;
  using size_type           = typename traits_type::size_type;

  addr(const_iterator first, const_iterator last) : first_{first}, last_{last} {}
  addr(const_iterator first, size_type size) : first_{first}, last_{first + size} {}

 private:
  //using value_type = typename traits_type::value_type;
  //using iterator = typename traits_type::iterator;
  //using const_iterator = typename traits_type::const_iterator;
  //using size_type = typename traits_type::size_type;
  //using address_type = typename traits_type::address_type;

  const_iterator first_;
  const_iterator last_;
  Rep base;
};

}  // namespace address

template <typename T>
class segments {
 public:
  using traits_type         = traits::segment_traits<T>;

  using value_type          = typename traits_type::value_type;
  using iterator            = typename traits_type::iterator;
  using const_iterator      = typename traits_type::const_iterator;
  using size_type           = typename traits_type::size_type;
  using address_type        = typename traits_type::address_type;

  segments(const_iterator first, const_iterator last) : first_{first}, last_{last} {}
  segments(const_iterator first, size_type size) : first_{first}, last_{first + size} {}

  // iterators
  const_iterator begin() const noexcept { return first_; }
  const_iterator end() const noexcept { return last_; }

  // capacity
  size_type size() const noexcept { return std::distance(begin(), end()); }

 private:
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

#endif  // !BINLAB_SEGMENTS_H_
