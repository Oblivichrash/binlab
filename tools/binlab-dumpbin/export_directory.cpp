// export_directory.cpp:

#include "export_directory.h"

#include <iomanip>

using namespace binlab::COFF;

std::ostream& binlab::COFF::operator<<(std::ostream& os, const IMAGE_EXPORT_DIRECTORY& directory) {
  os << "Name: " << std::setw(6) << directory.Name;
  os << "Characteristics: " << std::setw(4) << directory.Characteristics;
  // os << "TimeDateStamp: " << std::ctime(reinterpret_cast<const time_t*>(&directory.TimeDateStamp));
  os << "Base: " << std::setw(4) << directory.Base;
  os << "NumberOfFunctions: " << std::setw(4) << directory.NumberOfFunctions;
  os << "NumberOfNames: " << std::setw(4) << directory.NumberOfNames;
  os << "Version: " << directory.MajorVersion << '.' << directory.MinorVersion;
  return os;
}