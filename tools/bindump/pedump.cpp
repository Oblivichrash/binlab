// tools/bindump/pedump.cpp:

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>

#include "pedump.h"
#include "Accessor.h"

using namespace binlab;
using namespace binlab::COFF;

template <typename T>
struct thunk_data_traits;

template <>
struct thunk_data_traits<IMAGE_THUNK_DATA64> {
  using value_type        = IMAGE_THUNK_DATA64;
  using reference         = value_type&;
  using const_reference   = const value_type&;
  using iterator          = value_type*;
  using const_iterator    = const iterator;

  static constexpr auto snap_by_ordinal(const_reference thunk) { return IMAGE_SNAP_BY_ORDINAL64(thunk.u1.Ordinal); }
  static constexpr auto ordinal(const_reference thunk) { return IMAGE_ORDINAL64(thunk.u1.Ordinal); }
};

template <>
struct thunk_data_traits<IMAGE_THUNK_DATA32> {
  using value_type        = IMAGE_THUNK_DATA32;
  using reference         = value_type&;
  using const_reference   = const value_type&;
  using iterator          = value_type*;
  using const_iterator    = const iterator;

  static constexpr auto snap_by_ordinal(const_reference thunk) { return IMAGE_SNAP_BY_ORDINAL32(thunk.u1.Ordinal); }
  static constexpr auto ordinal(const_reference thunk) { return IMAGE_ORDINAL32(thunk.u1.Ordinal); }
};

using accessor = Accessor<IMAGE_SECTION_HEADER>;

std::ostream& operator<<(std::ostream& os, const IMAGE_EXPORT_DIRECTORY& directory) {
  os << "Name: " << directory.Name << '\n';
  os << "Characteristics: " << directory.Characteristics << '\n';
  //os << "TimeDateStamp: " << std::ctime(reinterpret_cast<const time_t*>(&directory.TimeDateStamp)) << '\n';
  os << "Version: " << directory.MajorVersion << '.' << directory.MinorVersion << '\n';
  os << "Base: " << directory.Base << '\n';
  os << "NumberOfFunctions: " << directory.NumberOfFunctions << '\n';
  os << "NumberOfNames: " << directory.NumberOfNames << '\n';
  return os;
}

std::ostream& operator<<(std::ostream& os, const IMAGE_IMPORT_DESCRIPTOR& descriptor) {
  os << "OrigFirstThunk: " << std::setw(8) << descriptor.OriginalFirstThunk << '\n';
  os << "TimeDateStamp: " << descriptor.TimeDateStamp << '\n';
  //auto time = std::ctime(reinterpret_cast<const time_t*>(&descriptor.TimeDateStamp));
  os << "ForwarderChain: " << descriptor.ForwarderChain << '\n';
  os << "FirstThunk: " << descriptor.FirstThunk << '\n';
  return os;
}

template <typename ThunkData>
void Dump(const accessor& base, const ThunkData* thunk) {
  using Traits = thunk_data_traits<ThunkData>;

  for (auto iter = thunk; iter->u1.AddressOfData; ++iter) {
    if (Traits::snap_by_ordinal(*iter)) {
      std::cout << std::setw(6) << Traits::ordinal(*iter);
    } else {
      auto& ordinal = reinterpret_cast<const IMAGE_IMPORT_BY_NAME&>(base[iter->u1.AddressOfData]);
      std::cout << std::setw(6) << ordinal.Hint << ": " << ordinal.Name;
    }
    std::cout << std::setw(9) << iter->u1.Function << '\n';
  }
  std::cout << '\n';
}

void Dump64(const accessor& base, const IMAGE_EXPORT_DIRECTORY* directory) {
  std::cout << *directory << '\n';

  auto functions = reinterpret_cast<const std::uint32_t*>(&base[directory->AddressOfFunctions]);
  auto ordinals = reinterpret_cast<const std::uint16_t*>(&base[directory->AddressOfNameOrdinals]);
  auto names = reinterpret_cast<const std::uint32_t*>(&base[directory->AddressOfNames]);

  for (std::size_t i = 0; i < directory->NumberOfFunctions; ++i) {
    if (functions[i] == 0) {
      continue;  // skip over gaps in exported function ordinals
    }
    std::cout << std::setw(8) << functions[i] << std::setw(5) << i + directory->Base << ' ';
    for (std::size_t j = 0; j < directory->NumberOfNames; ++j) {
      if (ordinals[j] == i) {
        std::cout << &base[names[j]] << '\n';
      }
    }
  }
  std::cout << '\n';
}

void Dump32(const accessor& base, const IMAGE_EXPORT_DIRECTORY* directory) {
  std::cout << *directory << '\n';

  auto functions = reinterpret_cast<const std::uint32_t*>(&base[directory->AddressOfFunctions]);
  auto ordinals = reinterpret_cast<const std::uint16_t*>(&base[directory->AddressOfNameOrdinals]);
  auto names = reinterpret_cast<const std::uint32_t*>(&base[directory->AddressOfNames]);

  for (std::size_t i = 0; i < directory->NumberOfFunctions; ++i) {
    if (functions[i] == 0) {
      continue;  // skip over gaps in exported function ordinals
    }
    std::cout << std::setw(8) << functions[i] << std::setw(5) << i + directory->Base << ' ';
    for (std::size_t j = 0; j < directory->NumberOfNames; ++j) {
      if (ordinals[j] == i) {
        std::cout << &base[names[j]] << '\n';
      }
    }
  }
  std::cout << '\n';
}

void Dump64(const accessor& base, const IMAGE_IMPORT_DESCRIPTOR* descriptor) {
  for (auto iter = descriptor; iter->TimeDateStamp || iter->Name; ++iter) {
    std::cout << &base[iter->Name] << '\n';
    std::cout << *iter << '\n';

    if (iter->OriginalFirstThunk || iter->FirstThunk) {
      Dump(base, reinterpret_cast<const IMAGE_THUNK_DATA64*>(&base[iter->FirstThunk]));
      std::cout << '\n';
    }
  }
  std::cout << '\n';
}

void Dump32(const accessor& base, const IMAGE_IMPORT_DESCRIPTOR* descriptor) {
  for (auto iter = descriptor; iter->TimeDateStamp || iter->Name; ++iter) {
    std::cout << &base[iter->Name] << '\n';
    std::cout << *iter << '\n';

    if (iter->OriginalFirstThunk || iter->FirstThunk) {
      Dump(base, reinterpret_cast<const IMAGE_THUNK_DATA32*>(&base[iter->FirstThunk]));
      std::cout << '\n';
    }
  }
  std::cout << '\n';
}

void Dump32(const IMAGE_BASE_RELOCATION* relocation) {
  for (auto* iter = reinterpret_cast<const char*>(relocation);; iter += relocation->SizeOfBlock) {
    relocation = reinterpret_cast<const IMAGE_BASE_RELOCATION*>(iter);
    if (relocation->SizeOfBlock == 0 || relocation->VirtualAddress == 0) {
      break;
    }

    if (relocation->SizeOfBlock < sizeof(*relocation)) {
      break;
    }

    std::cout << "Virtual Address: " << relocation->VirtualAddress << " Size: " << relocation->SizeOfBlock << '\n';
    auto entry = reinterpret_cast<const WORD*>(relocation + 1);
    for (std::size_t j = 0; j < (relocation->SizeOfBlock - sizeof(*relocation)) / sizeof(*entry); ++j) {
      std::cout << entry[j] << '\n';
    }
    std::cout << '\n';
  }
  std::cout << '\n';
}

void Dump64(const IMAGE_BASE_RELOCATION* relocation) {
  for (auto* iter = reinterpret_cast<const char*>(relocation);; iter += relocation->SizeOfBlock) {
    relocation = reinterpret_cast<const IMAGE_BASE_RELOCATION*>(iter);
    if (relocation->SizeOfBlock == 0 || relocation->VirtualAddress == 0) {
      break;
    }

    if (relocation->SizeOfBlock < sizeof(*relocation)) {
      break;
    }

    std::cout << "Virtual Address: " << relocation->VirtualAddress << " Size: " << relocation->SizeOfBlock << '\n';
    auto entry = reinterpret_cast<const WORD*>(relocation + 1);
    for (std::size_t j = 0; j < (relocation->SizeOfBlock - sizeof(*relocation)) / sizeof(*entry); ++j) {
      std::cout << entry[j] << '\n';
    }
    std::cout << '\n';
  }
  std::cout << '\n';
}

void DumpNt(const accessor& base, const IMAGE_NT_HEADERS32& nt) {
  const IMAGE_DATA_DIRECTORY* directory;
  
  std::cout << std::hex;
  directory = &nt.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
  if (directory->VirtualAddress != 0 && directory->Size != 0) {
    std::cout << "dump export directory\n";
    Dump32(base, reinterpret_cast<const IMAGE_EXPORT_DIRECTORY*>(&base[directory->VirtualAddress]));
  }

  directory = &nt.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
  if (directory->VirtualAddress != 0 && directory->Size != 0) {
    std::cout << "dump import descriptor\n";
    Dump32(base, reinterpret_cast<const IMAGE_IMPORT_DESCRIPTOR*>(&base[directory->VirtualAddress]));
  }

  directory = &nt.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC];
  if (directory->VirtualAddress != 0 && directory->Size != 0) {
    std::cout << "dump base relocation\n";
    Dump32(reinterpret_cast<const IMAGE_BASE_RELOCATION*>(&base[directory->VirtualAddress]));
  }
}

void DumpNt(const accessor& base, const IMAGE_NT_HEADERS64& nt) {
  const IMAGE_DATA_DIRECTORY* directory;
  
  std::cout << std::hex;
  directory = &nt.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
  if (directory->VirtualAddress != 0 && directory->Size != 0) {
    std::cout << "dump export directory\n";
    Dump64(base, reinterpret_cast<const IMAGE_EXPORT_DIRECTORY*>(&base[directory->VirtualAddress]));
  }

  directory = &nt.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
  if (directory->VirtualAddress != 0 && directory->Size != 0) {
    std::cout << "dump import descriptor\n";
    Dump64(base, reinterpret_cast<const IMAGE_IMPORT_DESCRIPTOR*>(&base[directory->VirtualAddress]));
  }

  directory = &nt.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC];
  if (directory->VirtualAddress != 0 && directory->Size != 0) {
    std::cout << "dump base relocation\n";
    Dump64(reinterpret_cast<const IMAGE_BASE_RELOCATION*>(&base[directory->VirtualAddress]));
  }
}

void COFF::Dump(std::vector<char>& buff) {
  auto& dos = reinterpret_cast<IMAGE_DOS_HEADER&>(buff[0]);
  if (dos.e_magic == IMAGE_DOS_SIGNATURE) {
    auto& nt = reinterpret_cast<IMAGE_NT_HEADERS64&>(buff[dos.e_lfanew]);
    if (nt.Signature == IMAGE_NT_SIGNATURE) {
      Accessor<IMAGE_SECTION_HEADER> base{static_cast<void*>(&buff[0]), IMAGE_FIRST_SECTION(&nt), nt.FileHeader.NumberOfSections};
      switch (nt.OptionalHeader.Magic) {
        case IMAGE_NT_OPTIONAL_HDR64_MAGIC:
          std::cout << "dump PE64\n";
          DumpNt(base, nt);
          break;
        case IMAGE_NT_OPTIONAL_HDR32_MAGIC:
          std::cout << "dump PE32\n";
          DumpNt(base, reinterpret_cast<IMAGE_NT_HEADERS32&>(buff[dos.e_lfanew]));
          break;
        default:
          std::cerr << "invalid optional magic\n";
          break;
      }
    }
  }
}
