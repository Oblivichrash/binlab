// resource_directory.h:

#ifndef BINLAB_COFF_RESOURCE_DIRECTORY_H_
#define BINLAB_COFF_RESOURCE_DIRECTORY_H_

#include <iosfwd>
#include <iterator>
#include <stdexcept>

#include "binlab/BinaryFormat/COFF.h"

namespace binlab {
namespace COFF {

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
