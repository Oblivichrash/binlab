// tools/bindump/Accessor.h:

#ifndef BINLAB_BINDUMP_ACCESSOR_H_
#define BINLAB_BINDUMP_ACCESSOR_H_

#include "segments.h"

namespace binlab {

template <typename T>
class Accessor : segments<T> {
 public:
  using size_type         = typename traits::segment_traits<T>::size_type;
  using reference         = char&;
  using const_reference   = const char&;
  using const_iterator    = typename traits::segment_traits<T>::const_iterator;
  using address_type      = typename traits::segment_traits<T>::address_type;

  Accessor(void* base, const_iterator first, size_type num)
      : segments<T>{first, num}, base_{static_cast<char*>(base)} {}

  //const char& operator[](address_type vaddr) const {
  //  return base_[virtual_offset_to_file_offset(this->begin(), this->end(), vaddr)];
  //}

  // element access
  constexpr reference operator[](size_type vaddr) {
    return base_[virtual_offset_to_file_offset(this->first(), this->end(), vaddr)];
  }
  constexpr const_reference operator[](size_type vaddr) const {
    return base_[virtual_offset_to_file_offset(this->first(), this->end(), vaddr)];
  }

 private:
  char* base_;
};

}  // namespace binlab

#endif  // !BINLAB_BINDUMP_ACCESSOR_H_
