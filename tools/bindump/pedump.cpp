// tools/bindump/pedump.cpp:

#include "pedump.h"

using namespace binlab;
using namespace binlab::COFF;

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

void PE::Dump(std::istream& is) {
  if (auto size = is.seekg(0, std::ios::end).tellg()) {
    std::vector<char> buff(size);
    if (is.seekg(0, std::ios::beg).read(&buff[0], size)) {
      auto& dos = reinterpret_cast<IMAGE_DOS_HEADER&>(buff[0]);
      if (dos.e_magic == IMAGE_DOS_SIGNATURE) {
        auto& nt = reinterpret_cast<IMAGE_NT_HEADERS64&>(buff[dos.e_lfanew]);
        if (nt.Signature == IMAGE_NT_SIGNATURE) {
          Accessor rva{&buff[0], IMAGE_FIRST_SECTION(&nt), nt.FileHeader.NumberOfSections};
          switch (nt.OptionalHeader.Magic) {
            case IMAGE_NT_OPTIONAL_HDR64_MAGIC:
              Dump(buff, nt, rva);
              break;
            case IMAGE_NT_OPTIONAL_HDR32_MAGIC:
              Dump(buff, reinterpret_cast<IMAGE_NT_HEADERS32&>(buff[dos.e_lfanew]), rva);
              break;
            default:
              std::cerr << "invalid optional magic\n";
              break;
          }
        }
      }
    }
  }
}

void PE::Dump(const std::vector<char>& buff, const IMAGE_NT_HEADERS32& nt, const Accessor& rva) {
  const IMAGE_DATA_DIRECTORY* directory;
  
  std::cout << std::hex;
  directory = &nt.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
  if (directory->VirtualAddress != 0 && directory->Size != 0) {
    std::cout << "dump export directory\n";
    Dump32(rva, reinterpret_cast<const IMAGE_EXPORT_DIRECTORY*>(&rva[directory->VirtualAddress]));
  }

  directory = &nt.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
  if (directory->VirtualAddress != 0 && directory->Size != 0) {
    std::cout << "dump import descriptor\n";
    Dump32(rva, reinterpret_cast<const IMAGE_IMPORT_DESCRIPTOR*>(&rva[directory->VirtualAddress]));
  }

  directory = &nt.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC];
  if (directory->VirtualAddress != 0 && directory->Size != 0) {
    std::cout << "dump base relocation\n";
    Dump32(rva, reinterpret_cast<const IMAGE_BASE_RELOCATION*>(&rva[directory->VirtualAddress]));
  }
}

void PE::Dump(const std::vector<char>& buff, const IMAGE_NT_HEADERS64& nt, const Accessor& rva) {
  const IMAGE_DATA_DIRECTORY* directory;
  
  std::cout << std::hex;
  directory = &nt.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
  if (directory->VirtualAddress != 0 && directory->Size != 0) {
    std::cout << "dump export directory\n";
    Dump64(rva, reinterpret_cast<const IMAGE_EXPORT_DIRECTORY*>(&rva[directory->VirtualAddress]));
  }

  directory = &nt.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
  if (directory->VirtualAddress != 0 && directory->Size != 0) {
    std::cout << "dump import descriptor\n";
    Dump64(rva, reinterpret_cast<const IMAGE_IMPORT_DESCRIPTOR*>(&rva[directory->VirtualAddress]));
  }

  directory = &nt.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC];
  if (directory->VirtualAddress != 0 && directory->Size != 0) {
    std::cout << "dump base relocation\n";
    Dump64(rva, reinterpret_cast<const IMAGE_BASE_RELOCATION*>(&rva[directory->VirtualAddress]));
  }
}

void PE::Dump32(const Accessor& base, const IMAGE_EXPORT_DIRECTORY* directory) {
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

void PE::Dump32(const Accessor& base, const IMAGE_IMPORT_DESCRIPTOR* descriptor) {
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

void PE::Dump32(const Accessor& base, const IMAGE_BASE_RELOCATION* relocation) {
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

void PE::Dump64(const Accessor& base, const IMAGE_EXPORT_DIRECTORY* directory) {
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

void PE::Dump64(const Accessor& base, const IMAGE_IMPORT_DESCRIPTOR* descriptor) {
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

void PE::Dump64(const Accessor& base, const IMAGE_BASE_RELOCATION* relocation) {
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

void PE::Dump(const Accessor& base, const IMAGE_THUNK_DATA32* thunk) {
  for (auto iter = thunk; iter->u1.AddressOfData; ++iter) {
    if (IMAGE_SNAP_BY_ORDINAL32(iter->u1.Ordinal)) {
      std::cout << std::setw(6) << IMAGE_ORDINAL32(iter->u1.Ordinal);
    } else {
      auto& ordinal = reinterpret_cast<const IMAGE_IMPORT_BY_NAME&>(base[iter->u1.AddressOfData]);
      std::cout << std::setw(6) << ordinal.Hint << ": " << ordinal.Name;
    }
    std::cout << std::setw(9) << iter->u1.Function << '\n';
  }
  std::cout << '\n';
}

void PE::Dump(const Accessor& base, const IMAGE_THUNK_DATA64* thunk) {
  for (auto iter = thunk; iter->u1.AddressOfData; ++iter) {
    if (IMAGE_SNAP_BY_ORDINAL64(iter->u1.Ordinal)) {
      std::cout << std::setw(6) << IMAGE_ORDINAL64(iter->u1.Ordinal);
    } else {
      auto& ordinal = reinterpret_cast<const IMAGE_IMPORT_BY_NAME&>(base[iter->u1.AddressOfData]);
      std::cout << std::setw(6) << ordinal.Hint << ": " << ordinal.Name;
    }
    std::cout << std::setw(9) << iter->u1.Function << '\n';
  }
  std::cout << '\n';
}
