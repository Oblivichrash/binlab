// resource_directory.cpp:

#include "resource_directory.h"

#include <fstream>
#include <iomanip>
#include <locale>
#include <string>

using namespace binlab::COFF;

std::ostream& binlab::COFF::operator<<(std::ostream& os, const IMAGE_RESOURCE_DIRECTORY& directory) {
  os << std::setw(8) << directory.Characteristics;
  os << std::setw(8) << directory.TimeDateStamp;
  os << std::setw(8) << directory.MajorVersion;
  os << std::setw(8) << directory.MinorVersion;
  os << std::setw(8) << directory.NumberOfNamedEntries;
  os << std::setw(8) << directory.NumberOfIdEntries;
  return os;
}

std::ostream& binlab::COFF::operator<<(std::ostream& os, const IMAGE_RESOURCE_DIRECTORY_ENTRY& entry) {
  os << std::setw(8) << entry.NameOffset;
  os << std::setw(9) << entry.OffsetToData;
  os << std::setw(8) << entry.OffsetToDirectory;
  return os;
}

std::ostream& binlab::COFF::operator<<(std::ostream& os, const IMAGE_RESOURCE_DIR_STRING_U& string) {
  std::locale loc;
  std::string name(string.Length, '.');
  std::use_facet<std::ctype<std::decay_t<decltype(string.NameString[0])>>>(loc).narrow(string.NameString, string.NameString + string.Length, '.', name.data());
  os << "Name(" << std::setw(4) << string.Length << "): " << name;
  return os;
}

std::ostream& binlab::COFF::operator<<(std::ostream& os, const IMAGE_RESOURCE_DATA_ENTRY& entry) {
  os << std::setw(8) << entry.OffsetToData;
  os << std::setw(8) << entry.Size;
  os << std::setw(8) << entry.CodePage;
  return os;
}

static int res_count = 0;
std::ostream& binlab::COFF::dump(std::ostream& os, char* base, std::size_t vbase, IMAGE_RESOURCE_DIRECTORY* directory, std::size_t depth) {
  os << std::setw((depth + 1) * 2) << ' ';
  //os << std::setw(10) << "res dir:" << *directory << '\n';
  auto entry = begin(*directory);
  for (std::size_t i = 0; i < directory->NumberOfIdEntries + directory->NumberOfNamedEntries; ++i) {
    os << std::setw((depth + 1) * 2) << ' ';
    //os << std::setw(10) << "res ent:" << entry[i];
    if (entry[i].NameIsString) {
      os << reinterpret_cast<IMAGE_RESOURCE_DIR_STRING_U&>(base[entry[i].NameOffset]);
    } else {
      //os << "ID: " << std::setw(4);
      os << entry[i].Id;
    }
    os << '\n';

    if (entry[i].DataIsDirectory) {
      dump(os, base, vbase, reinterpret_cast<IMAGE_RESOURCE_DIRECTORY*>(&base[entry[i].OffsetToDirectory]), depth + 1);
    } else {
      auto& data = reinterpret_cast<IMAGE_RESOURCE_DATA_ENTRY&>(base[entry[i].OffsetToData]);
      ++res_count;

      char filename[32];
      std::snprintf(filename, sizeof(filename), "res/%d.%d.bin", res_count, entry[i].Id);

      os << std::setw((depth + 2) * 2) << ' ' << data << " index: " << filename << '\n';

      std::ofstream bin{filename, std::ios::ate | std::ios::binary};
      bin.write(&base[data.OffsetToData - vbase], data.Size);
    }
  }
  return os;
}

std::ostream& dump_rt_string(std::ostream& os, IMAGE_SECTION_HEADER& section, char* base, IMAGE_RESOURCE_DIRECTORY& directory2) {
  auto entry2 = begin(directory2);
  for (std::size_t j = 0; j < directory2.NumberOfIdEntries + directory2.NumberOfNamedEntries; ++j) {
    if (entry2[j].NameIsString) {
      throw std::runtime_error{"2nd level resource directory entry name is string"};
    }

    if (!entry2[j].DataIsDirectory) {
      throw std::runtime_error{"2nd level resource directory entry for string table is not directory"};
    }

    auto& directory3 = reinterpret_cast<IMAGE_RESOURCE_DIRECTORY&>(base[entry2[j].OffsetToDirectory]);
    auto entry3 = begin(directory3);
    for (std::size_t k = 0; k < directory3.NumberOfIdEntries + directory3.NumberOfNamedEntries; ++k) {
      if (entry3[k].NameIsString) {
        throw std::runtime_error{"3rd level resource directory entry name is string"};
      }

      if (entry3[k].DataIsDirectory) {
        throw std::runtime_error{"3rd level resource directory entry for string table is not data"};
      }

      auto& data = reinterpret_cast<IMAGE_RESOURCE_DATA_ENTRY&>(base[entry3[k].OffsetToData]);
      char* table = &base[data.OffsetToData - section.VirtualAddress];
      for (std::size_t l = 0, pos = 0; l < 0x10; ++l) {
        auto& string = reinterpret_cast<IMAGE_RESOURCE_DIR_STRING_U&>(table[pos]);
        if (string.Length) {
          os << "ID: " << std::setw(4) << ((entry2[j].Id - 1) * 0x10 + l) << ':' << string << '\n';
        }
        pos += sizeof(string.Length) + string.Length * sizeof(string.NameString[0]);
      }
    }
  }
  return os;
}

std::ostream& binlab::COFF::dump(std::ostream& os, IMAGE_SECTION_HEADER& section, char* base, IMAGE_RESOURCE_DIRECTORY& directory) {
  auto entry = begin(directory);
  for (std::size_t i = 0; i < directory.NumberOfIdEntries + directory.NumberOfNamedEntries; ++i) {
    if (entry[i].NameIsString) {
      throw std::runtime_error{"resource directory entry name is string"};
    }
    //os << entry[i] << '\n';

    if (entry[i].Id == 6) {  // string table
      if (!entry[i].DataIsDirectory) {
        throw std::runtime_error{"2nd level resource directory entry for string table is not directory"};
      }
      dump_rt_string(os, section, base, reinterpret_cast<IMAGE_RESOURCE_DIRECTORY&>(base[entry[i].OffsetToDirectory]));
    }
  }
  return os;
}