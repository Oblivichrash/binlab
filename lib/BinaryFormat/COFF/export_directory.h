// export_directory.h:

#ifndef BINLAB_COFF_EXPORT_DIRECTORY_H_
#define BINLAB_COFF_EXPORT_DIRECTORY_H_

#include <iosfwd>
#include <iterator>

#include "binlab/BinaryFormat/COFF.h"

namespace binlab {
namespace COFF {

std::ostream& operator<<(std::ostream& os, const IMAGE_EXPORT_DIRECTORY& directory);

}  // namespace COFF
}  // namespace binlab

#endif  // BINLAB_COFF_EXPORT_DIRECTORY_H_
