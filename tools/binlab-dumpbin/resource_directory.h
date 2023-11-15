// resource_directory.h:

#ifndef BINLAB_COFF_RESOURCE_DIRECTORY_H_
#define BINLAB_COFF_RESOURCE_DIRECTORY_H_

#include <iosfwd>

#include "binlab/BinaryFormat/COFF.h"

std::ostream& dump(std::ostream& os, char* base, binlab::COFF::IMAGE_RESOURCE_DIRECTORY* directory, std::size_t depth);

#endif  // BINLAB_COFF_RESOURCE_DIRECTORY_H_
