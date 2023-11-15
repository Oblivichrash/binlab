// resource_directory.cpp:

#include "resource_directory.h"

#include <fstream>
#include <iomanip>
#include <iostream>

using namespace binlab::COFF;

std::ostream& operator<<(std::ostream& os, const IMAGE_IMPORT_DESCRIPTOR& descriptor) {
  os << "OriginalFirstThunk: " << std::setw(8) << descriptor.OriginalFirstThunk;
  os << "TimeDateStamp: " << std::setw(8) << descriptor.TimeDateStamp;
  os << "ForwarderChain: " << std::setw(8) << descriptor.ForwarderChain;
  os << "Name: " << std::setw(8) << descriptor.Name;
  os << "FirstThunk: " << std::setw(8) << descriptor.FirstThunk;
  return os;
}

std::ostream& operator<<(std::ostream& os, const IMAGE_RESOURCE_DIRECTORY& directory) {
  os << "Characteristics: " << std::setw(8) << directory.Characteristics;
  os << "TimeDateStamp: " << std::setw(8) << directory.TimeDateStamp;
  os << "NumberOfNamedEntries: " << std::setw(8) << directory.NumberOfNamedEntries;
  os << "NumberOfIdEntries: " << std::setw(8) << directory.NumberOfIdEntries;
  os << "Version: " << directory.MajorVersion << '.' << directory.MinorVersion;
  return os;
}

std::ostream& operator<<(std::ostream& os, const IMAGE_RESOURCE_DIRECTORY_ENTRY& entry) {
  os << "NameOffset: " << std::setw(8) << entry.NameOffset;
  os << "OffsetToData: " << std::setw(9) << entry.OffsetToData;
  os << "OffsetToDirectory: " << std::setw(8) << entry.OffsetToDirectory;
  return os;
}

std::ostream& operator<<(std::ostream& os, const IMAGE_RESOURCE_DIR_STRING_U& str) {
  std::locale loc;
  std::string name(str.Length, '.');
  std::use_facet<std::ctype<std::decay_t<decltype(str.NameString[0])>>>(loc).narrow(str.NameString, str.NameString + str.Length, '.', name.data());
  os << "Name(" << str.Length << "): " << name;
  return os;
}

std::ostream& operator<<(std::ostream& os, const IMAGE_RESOURCE_DATA_ENTRY& data) {
  os << "OffsetToData: " << std::setw(8) << data.OffsetToData;
  os << "Size: " << std::setw(8) << data.Size;
  os << "CodePage: " << std::setw(8) << data.CodePage;
  return os;
}

std::ostream& operator<<(std::ostream& os, const IMAGE_BASE_RELOCATION& relocation) {
  os << "VirtualAddress: " << std::setw(8) << relocation.VirtualAddress;
  os << "SizeOfBlock: " << std::setw(8) << relocation.SizeOfBlock;
  return os;
}

std::ostream& dump(std::ostream& os, char* base, IMAGE_RESOURCE_DIRECTORY* directory, std::size_t depth) {
  os << std::setw((depth + 1) * 2) << "  " << *directory << '\n';
  auto entry = reinterpret_cast<IMAGE_RESOURCE_DIRECTORY_ENTRY*>(directory + 1);
  for (std::size_t i = 0; i < directory->NumberOfIdEntries + directory->NumberOfNamedEntries; ++i) {
    os << std::setw((depth + 1) * 2) << "  " << entry[i];
    if (entry[i].NameIsString) {
      os << reinterpret_cast<IMAGE_RESOURCE_DIR_STRING_U&>(base[entry[i].NameOffset]);
    } else {
      os << "ID: " << std::setw(4) << entry[i].Id;
    }
    os << '\n';

    if (entry[i].DataIsDirectory) {
      dump(os, base, reinterpret_cast<IMAGE_RESOURCE_DIRECTORY*>(&base[entry[i].OffsetToDirectory]), depth + 1);
    } else {
      auto& data = reinterpret_cast<IMAGE_RESOURCE_DATA_ENTRY&>(base[entry[i].OffsetToData]);
      os << std::setw((depth + 2) * 2) << "  " << data << '\n';
    }
  }
  return os;
}