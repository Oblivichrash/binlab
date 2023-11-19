// section_header.h:

#ifndef BINLAB_COFF_SECTION_HEADER_H_
#define BINLAB_COFF_SECTION_HEADER_H_

#include <iosfwd>
#include <iterator>

#include "binlab/BinaryFormat/COFF.h"

namespace binlab {
namespace COFF {

std::ostream& operator<<(std::ostream& os, const IMAGE_SECTION_HEADER& header);

inline IMAGE_SECTION_HEADER* begin(const IMAGE_NT_HEADERS64& Nt) {
  return reinterpret_cast<IMAGE_SECTION_HEADER*>(reinterpret_cast<std::size_t>(&Nt) + offsetof(std::decay_t<decltype(Nt)>, OptionalHeader) + Nt.FileHeader.SizeOfOptionalHeader);
}

inline IMAGE_SECTION_HEADER* end(const IMAGE_NT_HEADERS64& Nt) {
  return begin(Nt) + Nt.FileHeader.NumberOfSections;
}

inline IMAGE_SECTION_HEADER* begin(const IMAGE_NT_HEADERS32& Nt) {
  return reinterpret_cast<IMAGE_SECTION_HEADER*>(reinterpret_cast<std::size_t>(&Nt) + offsetof(std::decay_t<decltype(Nt)>, OptionalHeader) + Nt.FileHeader.SizeOfOptionalHeader);
}

inline IMAGE_SECTION_HEADER* end(const IMAGE_NT_HEADERS32& Nt) {
  return begin(Nt) + Nt.FileHeader.NumberOfSections;
}

class section_ref {
 public:
  using byte_type       = char;
  using byte_pointer    = char*;
  using address_type    = std::size_t;

  section_ref(const address_type& base, void* data) : base_{base}, data_{static_cast<byte_pointer>(data)} {}

  constexpr byte_type& operator[](address_type addr) { return data_[addr - base_]; }
  constexpr const byte_type& operator[](address_type addr) const { return data_[addr - base_]; }

 private:
  address_type base_;
  byte_pointer data_;
};

}  // namespace COFF
}  // namespace binlab

#endif  // BINLAB_COFF_SECTION_HEADER_H_
