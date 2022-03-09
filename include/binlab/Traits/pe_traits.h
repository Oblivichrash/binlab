// binlab/Traits/pe_traits.h:

#ifndef BINLAB_TRAITS_PE_TRAITS_H_
#define BINLAB_TRAITS_PE_TRAITS_H_

#include <cstdint>

#include "binlab/BinaryFormat/COFF.h"
#include "binlab/Traits/bin_traits.h"

namespace binlab {
namespace traits {

template <>
struct segment_traits<COFF::IMAGE_SECTION_HEADER> {
  using value_type        = COFF::IMAGE_SECTION_HEADER;
  using reference         = value_type&;
  using const_reference   = const value_type&;
  using size_type         = std::uint64_t;
  using difference_type   = std::ptrdiff_t;
  using iterator          = value_type*;
  using const_iterator    = const iterator;

  using address_type      = std::uint64_t;

  static constexpr address_type virtual_offset(const_reference segment) { return segment.VirtualAddress; }
  static constexpr size_type virtual_size(const_reference segment) { return segment.Misc.VirtualSize; }

  static constexpr address_type file_offset(const_reference segment) { return segment.PointerToRawData; }
  static constexpr size_type file_size(const_reference segment) { return segment.SizeOfRawData; }
};

}  // namespace traits
}  // namespace binlab

#endif  // !BINLAB_TRAITS_PE_TRAITS_H_
