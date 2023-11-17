// import_descriptor.h:

#ifndef BINLAB_COFF_IMPORT_DESCRIPTOR_H_
#define BINLAB_COFF_IMPORT_DESCRIPTOR_H_

#include <iosfwd>
#include <iterator>

#include "binlab/BinaryFormat/COFF.h"
#include "section_header.h"

namespace binlab {
namespace COFF {

std::ostream& operator<<(std::ostream& os, const IMAGE_IMPORT_DESCRIPTOR& descriptor);

constexpr bool operator==(const IMAGE_IMPORT_DESCRIPTOR* desc, std::default_sentinel_t) { return !desc->Name; }
constexpr bool operator==(const IMAGE_THUNK_DATA64* thunk, std::default_sentinel_t) { return !thunk->u1.AddressOfData; }

class import_table64_ref {
 public:
  import_table64_ref(std::size_t virtual_address, section_ref& section) : section_{section}, vaddr_{virtual_address} {}

  IMAGE_IMPORT_DESCRIPTOR* begin() { return reinterpret_cast<IMAGE_IMPORT_DESCRIPTOR*>(&section_[vaddr_]); }
  auto end() { return std::default_sentinel; }
  IMAGE_THUNK_DATA64* begin(const IMAGE_IMPORT_DESCRIPTOR& desc) { return reinterpret_cast<IMAGE_THUNK_DATA64*>(&section_[desc.OriginalFirstThunk]); }
  auto end(const IMAGE_IMPORT_DESCRIPTOR&) { return std::default_sentinel; }

 private:
  section_ref section_;
  std::size_t vaddr_;
};

std::ostream& dump_import64(std::ostream& os, section_ref& section, std::size_t vaddr);

}  // namespace COFF
}  // namespace binlab

#endif  // BINLAB_COFF_IMPORT_DESCRIPTOR_H_
