// tools/bindump/pedump.h:

#ifndef BINLAB_BINDUMP_PEDUMP_H_
#define BINLAB_BINDUMP_PEDUMP_H_

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <vector>

#include "binlab/BinaryFormat/COFF.h"
#include "segments.h"

namespace binlab {

template <>
struct segment_traits<COFF::IMAGE_SECTION_HEADER> {
  using value_type = COFF::IMAGE_SECTION_HEADER;
  using address_type = std::uint64_t;
  using size_type = std::uint64_t;

  static constexpr address_type virtual_address(const value_type& segment) { return segment.VirtualAddress; }
  static constexpr address_type file_offset(const value_type& segment) { return segment.PointerToRawData; }

  static constexpr size_type file_size(const value_type& segment) { return segment.SizeOfRawData; }
  static constexpr size_type memory_size(const value_type& segment) { return segment.Misc.VirtualSize; }
};

namespace PE {

class Accessor : segments<COFF::IMAGE_SECTION_HEADER> {
 public:
  Accessor(void* base, const_iterator first, size_type num) : segments(first, num), base_{static_cast<char*>(base)} {}

  const char& operator[](address_type offset) const {
    return base_[RelativeVirtualAddressToFileOffset(static_cast<address_type>(offset))];
  }

 private:
  address_type RelativeVirtualAddressToFileOffset(const address_type rva) const {
    auto pred = [rva](const value_type& section) {
      address_type section_base = traits_type::virtual_address(section);
      return std::clamp(rva, section_base, section_base + traits_type::file_size(section)) == rva;
    };

    auto iter = std::find_if(first_, last_, pred);
    if (iter == last_) {
      throw std::out_of_range{"virtual address out of range"};
    }
    return traits_type::file_offset(*iter) + (rva - traits_type::virtual_address(*iter));
  }

 private:
  char* base_;
};

void Dump(std::vector<char>& buff);
void Dump(const Accessor& base, const COFF::IMAGE_NT_HEADERS32& nt);
void Dump(const Accessor& base, const COFF::IMAGE_NT_HEADERS64& nt);

void Dump32(const Accessor& base, const COFF::IMAGE_EXPORT_DIRECTORY* directory);
void Dump32(const Accessor& base, const COFF::IMAGE_IMPORT_DESCRIPTOR* descriptor);
void Dump32(const Accessor& base, const COFF::IMAGE_BASE_RELOCATION* relocation);

void Dump64(const Accessor& base, const COFF::IMAGE_EXPORT_DIRECTORY* directory);
void Dump64(const Accessor& base, const COFF::IMAGE_IMPORT_DESCRIPTOR* descriptor);
void Dump64(const Accessor& base, const COFF::IMAGE_BASE_RELOCATION* relocation);

void Dump(const Accessor& base, const COFF::IMAGE_THUNK_DATA32* thunk);
void Dump(const Accessor& base, const COFF::IMAGE_THUNK_DATA64* thunk);

}  // namespace PE
}  // namespace binlab

#endif  // !BINLAB_BINDUMP_PEDUMP_H_
