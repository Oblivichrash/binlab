// resource_directory.h:

#ifndef BINLAB_COFF_RESOURCE_DIRECTORY_H_
#define BINLAB_COFF_RESOURCE_DIRECTORY_H_

#include <iosfwd>
#include <iterator>
#include <system_error>

#include "binlab/BinaryFormat/COFF.h"

namespace binlab {
namespace COFF {

#ifdef _WIN32
using WCHAR       = wchar_t;
#else  // __GNU__
using WCHAR       = char16_t;
#endif  // _WIN32

struct IMAGE_RESOURCE_DIRECTORY_STRING {
  WORD    Length;
  CHAR    NameString[1];
};

struct IMAGE_RESOURCE_DIR_STRING_U {
  WORD    Length;
  WCHAR   NameString[1];
};

std::ostream& operator<<(std::ostream& os, const IMAGE_RESOURCE_DIRECTORY& directory);
std::ostream& operator<<(std::ostream& os, const IMAGE_RESOURCE_DIRECTORY_ENTRY& entry);
std::ostream& operator<<(std::ostream& os, const IMAGE_RESOURCE_DIR_STRING_U& string);
std::ostream& operator<<(std::ostream& os, const IMAGE_RESOURCE_DATA_ENTRY& entry);

inline IMAGE_RESOURCE_DIRECTORY_ENTRY* begin(IMAGE_RESOURCE_DIRECTORY& directory) {
  return reinterpret_cast<IMAGE_RESOURCE_DIRECTORY_ENTRY*>(&directory + 1);
}

inline IMAGE_RESOURCE_DIRECTORY_ENTRY* end(IMAGE_RESOURCE_DIRECTORY& directory) {
  return reinterpret_cast<IMAGE_RESOURCE_DIRECTORY_ENTRY*>(&directory + 1 + directory.NumberOfIdEntries + directory.NumberOfNamedEntries);
}

std::ostream& dump(std::ostream& os, char* base, std::size_t vbase, IMAGE_RESOURCE_DIRECTORY* directory, std::size_t depth);
std::ostream& dump(std::ostream& os, IMAGE_SECTION_HEADER& section, char* base, IMAGE_RESOURCE_DIRECTORY& directory);

}  // namespace COFF
}  // namespace binlab

#endif  // BINLAB_COFF_RESOURCE_DIRECTORY_H_
