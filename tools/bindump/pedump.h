// tools/bindump/pedump.h:

#ifndef BINLAB_BINDUMP_PEDUMP_H_
#define BINLAB_BINDUMP_PEDUMP_H_

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <vector>

#include "binlab/BinaryFormat/COFF.h"

namespace binlab {

class Accessor {
 public:
  using Section = COFF::IMAGE_SECTION_HEADER;
  using Address = std::uint64_t;

  Accessor(void* base, const Section* first, const Address num)
      : base_{static_cast<char*>(base)}, first_{first}, last_{first + num} {}

  const char& operator[](Address offset) const {
    return base_[RelativeVirtualAddressToFileOffset(static_cast<Address>(offset))];
  }

 private:
  Address RelativeVirtualAddressToFileOffset(const Address rva) const {
    auto pred = [rva](const Section& section) {
      Address section_base = section.VirtualAddress;
      return std::clamp(rva, section_base, section_base + section.SizeOfRawData) == rva;
    };

    auto iter = std::find_if(first_, last_, pred);
    if (iter == last_) {
      throw std::out_of_range{"virtual address out of range"};
    }
    return iter->PointerToRawData + (rva - iter->VirtualAddress);
  }

 private:
  char* base_;
  const Section* first_;
  const Section* last_;
};

namespace PE {

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
