// section_header.cpp:

#include "section_header.h"

#include <cstring>
#include <iomanip>

using namespace binlab::COFF;

std::ostream& binlab::COFF::operator<<(std::ostream& os, const IMAGE_SECTION_HEADER& header) {
  char name[sizeof(header.Name) + 1] = {0};
  std::memcpy(name, header.Name, sizeof(header.Name));
  os << std::setw(8) << name;
  os << std::setw(8) << header.Misc.VirtualSize;
  os << std::setw(8) << header.VirtualAddress;
  os << std::setw(8) << header.SizeOfRawData;
  os << std::setw(8) << header.PointerToRawData;
  os << std::setw(8) << header.PointerToRelocations;
  os << std::setw(8) << header.PointerToLinenumbers;
  os << std::setw(8) << header.NumberOfRelocations;
  os << std::setw(8) << header.NumberOfLinenumbers;
  os << std::setw(8) << header.Characteristics;
  return os;
}
