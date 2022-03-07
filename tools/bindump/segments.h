// segments.h:

#ifndef BINLAB_SEGMENTS_H_
#define BINLAB_SEGMENTS_H_

#include <cstdint>

namespace binlab {

template <typename T>
struct segment_traits;

template <typename T, typename Traits = segment_traits<T>>
class segments {
 public:
  using value_type = T;
  using iterator = T*;
  using const_iterator = const T*;
  using size_type = typename Traits::size_type;
  using address_type = typename Traits::address_type;

  using traits_type = Traits;

  segments(const_iterator first, const_iterator last) : first_{first}, last_{last} {}
  segments(const_iterator first, size_type size) : first_{first}, last_{first + size} {}

  // iterators
  const_iterator begin() const noexcept { return first_; }
  const_iterator end() const noexcept { return last_; }

  // capacity
  size_type size() const noexcept { return std::distance(begin(), end()); }

 protected:
  const_iterator first_;
  const_iterator last_;
};

}  // namespace binlab

#endif  // !BINLAB_SEGMENTS_H_
