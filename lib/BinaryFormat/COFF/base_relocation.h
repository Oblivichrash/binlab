// base_relocation.h:

#ifndef BINLAB_COFF_BASE_RELOCATION_H_
#define BINLAB_COFF_BASE_RELOCATION_H_

#include <iosfwd>
#include <iterator>

#include "binlab/BinaryFormat/COFF.h"
#include "section_header.h"

namespace binlab {
namespace COFF {

// Based relocation types.
enum {
  IMAGE_REL_BASED_ABSOLUTE              = 0,
  IMAGE_REL_BASED_HIGH                  = 1,
  IMAGE_REL_BASED_LOW                   = 2,
  IMAGE_REL_BASED_HIGHLOW               = 3,
  IMAGE_REL_BASED_HIGHADJ               = 4,
  IMAGE_REL_BASED_MACHINE_SPECIFIC_5    = 5,
  IMAGE_REL_BASED_RESERVED              = 6,
  IMAGE_REL_BASED_MACHINE_SPECIFIC_7    = 7,
  IMAGE_REL_BASED_MACHINE_SPECIFIC_8    = 8,
  IMAGE_REL_BASED_MACHINE_SPECIFIC_9    = 9,
  IMAGE_REL_BASED_DIR64                 = 10
};

union BASE_RELOCATION_ENTRY {
  WORD Item;
  struct {
    WORD Offset : 12;
    WORD Type : 4;
  };
};

constexpr bool operator==(const IMAGE_BASE_RELOCATION* relocation, std::default_sentinel_t) { return !relocation->VirtualAddress; }

std::ostream& operator<<(std::ostream& os, const IMAGE_BASE_RELOCATION& relocation);
std::ostream& operator<<(std::ostream& os, const BASE_RELOCATION_ENTRY& relocation);

inline BASE_RELOCATION_ENTRY* begin(IMAGE_BASE_RELOCATION& relocation) {
  return reinterpret_cast<BASE_RELOCATION_ENTRY*>(&relocation + 1);
}

inline BASE_RELOCATION_ENTRY* end(IMAGE_BASE_RELOCATION& relocation) {
  return reinterpret_cast<BASE_RELOCATION_ENTRY*>(reinterpret_cast<std::size_t>(&relocation) + relocation.SizeOfBlock);
}

std::ostream& dump(std::ostream& os, IMAGE_SECTION_HEADER& section, char* base, IMAGE_BASE_RELOCATION& relocation);

}  // namespace COFF
}  // namespace binlab

#endif  // BINLAB_COFF_BASE_RELOCATION_H_
