// tools/bindump/pedump.cpp:

#include "pedump.h"

using namespace binlab;
using namespace binlab::COFF;

void PE::Dump(std::istream& is) {
  if (auto size = is.seekg(0, std::ios::end).tellg()) {
    std::vector<char> buff(size);
    if (is.seekg(0, std::ios::beg).read(&buff[0], size)) {
      auto& dos = reinterpret_cast<IMAGE_DOS_HEADER&>(buff[0]);
      if (dos.e_magic == IMAGE_DOS_SIGNATURE) {
        auto& nt = reinterpret_cast<IMAGE_NT_HEADERS64&>(buff[dos.e_lfanew]);
        if (nt.Signature == IMAGE_NT_SIGNATURE) {
          Accessor rva{&buff[0], IMAGE_FIRST_SECTION(&nt), nt.FileHeader.NumberOfSections};
          IMAGE_DATA_DIRECTORY* directory;

          if (nt.OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR64_MAGIC) {
            return;
          }

          std::cout << std::hex;
          directory = &nt.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
          if (directory->VirtualAddress != 0 && directory->Size != 0) {
            std::cout << "dump export directory\n";
            Dump(rva, reinterpret_cast<const IMAGE_EXPORT_DIRECTORY*>(&rva[directory->VirtualAddress]));
          }

          directory = &nt.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
          if (directory->VirtualAddress != 0 && directory->Size != 0) {
            std::cout << "dump import descriptor\n";
            Dump(rva, reinterpret_cast<const IMAGE_IMPORT_DESCRIPTOR*>(&rva[directory->VirtualAddress]));
          }

          directory = &nt.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC];
          if (directory->VirtualAddress != 0 && directory->Size != 0) {
            std::cout << "dump base relocation\n";
            Dump(rva, reinterpret_cast<const IMAGE_BASE_RELOCATION*>(&rva[directory->VirtualAddress]));
          }
        }
      }
    }
  }
}

void PE::Dump(const Accessor& base, const IMAGE_EXPORT_DIRECTORY* directory) {
  std::cout << "Name: " << directory->Name << '\n';
  std::cout << "Characteristics: " << directory->Characteristics << '\n';
  std::cout << "TimeDateStamp: " << std::ctime(reinterpret_cast<const time_t*>(&directory->TimeDateStamp)) << '\n';
  std::cout << "Version: " << directory->MajorVersion << '.' << directory->MinorVersion << '\n';
  std::cout << "Base: " << directory->Base << '\n';
  std::cout << "NumberOfFunctions: " << directory->NumberOfFunctions << '\n';
  std::cout << "NumberOfNames: " << directory->NumberOfNames << '\n';

  auto functions = reinterpret_cast<const std::uint32_t*>(&base[directory->AddressOfFunctions]);
  auto ordinals = reinterpret_cast<const std::uint16_t*>(&base[directory->AddressOfNameOrdinals]);
  auto name = reinterpret_cast<const std::uint32_t*>(&base[directory->AddressOfNames]);

  for (std::size_t i = 0; i < directory->NumberOfFunctions; ++i) {
    if (functions[i] == 0) {
      continue;  // skip over gaps in exported function ordinals
    }
    std::cout << std::setw(8) << functions[i] << std::setw(5) << i + directory->Base << ' ';
    for (std::size_t j = 0; j < directory->NumberOfNames; ++j) {
      if (ordinals[j] == i) {
        std::cout << &base[name[j]] << '\n';
      }
    }
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

void PE::Dump(const Accessor& base, const IMAGE_IMPORT_DESCRIPTOR* descriptor) {
  for (auto iter = descriptor; iter->TimeDateStamp || iter->Name; ++iter) {
    std::cout << &base[iter->Name] << '\n';
    std::cout << "OrigFirstThunk: " << std::setw(8) << iter->OriginalFirstThunk << '\n';
    std::cout << "TimeDateStamp: " << iter->TimeDateStamp << '\n';
    //auto time = std::ctime(reinterpret_cast<const time_t*>(&iter->TimeDateStamp));
    std::cout << "ForwarderChain: " << iter->ForwarderChain << '\n';
    std::cout << "FirstThunk: " << iter->FirstThunk << '\n';

    if (iter->OriginalFirstThunk || iter->FirstThunk) {
      Dump(base, reinterpret_cast<const IMAGE_THUNK_DATA64*>(&base[iter->FirstThunk]));
      std::cout << '\n';
    }
  }
  std::cout << '\n';
}

void PE::Dump(const Accessor& base, const IMAGE_BASE_RELOCATION* relocation) {
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
