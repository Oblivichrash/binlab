// tools/pedump/main.cpp: Dump Binary files

#include <Windows.h>

#include <codecvt>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <locale>
#include <type_traits>
#include <vector>

#include "binlab/Config/Config.h"

template <typename Rep>
class address;

template <typename T>
class segments {
 public:
  template <typename InputIt>
  segments(InputIt first, InputIt last);

 private:
  template <typename Rep>
  friend class address;
};

template <typename Rep = std::size_t>
class address {
 public:
  constexpr address() = delete;

  constexpr explicit address(const void* base) noexcept : value_{reinterpret_cast<Rep>(base)} {}

  template <typename Rep2, std::enable_if_t<std::is_convertible_v<const Rep2&, Rep>, int> = 0>
  constexpr explicit address(const Rep2& value) noexcept : value_{value} {}

  //constexpr explicit operator Rep() const noexcept { return value_; }

  constexpr auto operator<=>(const address&) const noexcept = default;

  template <typename Type, std::enable_if_t<std::is_pointer_v<Type> || std::is_reference_v<Type>, int> = 0>
  Type object_cast() noexcept { return reinterpret_cast<Type>(value_); }

  address& operator+=(const address& rhs) noexcept {
    value_ += rhs.value_;
    return *this;
  }

 private:
  Rep value_;
};

template <typename Rep = std::size_t>
class virtual_address : public address<Rep> {
 public:
  template <typename Rep2, std::enable_if_t<std::is_convertible_v<const Rep2&, Rep>, int> = 0>
  constexpr explicit virtual_address(const Rep2& value) noexcept : address<Rep>{value} {}
};

template <typename Rep = std::size_t>
class virtual_offset : public address<Rep> {
 public:
  template <typename Rep2, std::enable_if_t<std::is_convertible_v<const Rep2&, Rep>, int> = 0>
  constexpr explicit virtual_offset(const Rep2& value) noexcept : address<Rep>{value} {}
};

template <typename Rep = std::size_t>
class file_address : public address<Rep> {
 public:
  template <typename Rep2, std::enable_if_t<std::is_convertible_v<const Rep2&, Rep>, int> = 0>
  constexpr explicit file_address(const Rep2& value) noexcept : address<Rep>{value} {}
};

template <typename Rep = std::size_t>
class file_offset : public address<Rep> {
 public:
  template <typename Rep2, std::enable_if_t<std::is_convertible_v<const Rep2&, Rep>, int> = 0>
  constexpr explicit file_offset(const Rep2& value) noexcept : address<Rep>{value} {}
};

std::ostream& memdump(std::ostream& os, void* base, std::size_t rows) {
  auto buff = static_cast<std::uint8_t*>(base);
  for (std::size_t i = 0; i < rows; ++i, buff += 16) {
    for (std::size_t j = 0; j < 16; ++j) {
      os << ' ' << std::setw(2) << std::setfill('0') << unsigned{buff[j]};
    }
    os << '\n';
  }
  return os;
}

std::ostream& operator<<(std::ostream& os, const IMAGE_EXPORT_DIRECTORY& directory) {
  os << "Name: " << std::setw(6) << directory.Name;
  os << "Characteristics: " << std::setw(4) << directory.Characteristics;
  // os << "TimeDateStamp: " << std::ctime(reinterpret_cast<const time_t*>(&directory.TimeDateStamp));
  os << "Base: " << std::setw(4) << directory.Base;
  os << "NumberOfFunctions: " << std::setw(4) << directory.NumberOfFunctions;
  os << "NumberOfNames: " << std::setw(4) << directory.NumberOfNames;
  os << "Version: " << directory.MajorVersion << '.' << directory.MinorVersion;
  return os;
}

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

std::ostream& dump(std::ostream& os, char* buff, std::uint64_t delta, IMAGE_EXPORT_DIRECTORY* directory) {
  os << &buff[directory->Name - delta] << "\n  " << std::left << *directory << std::right << '\n';

  auto functions = reinterpret_cast<const std::uint32_t*>(&buff[directory->AddressOfFunctions - delta]);
  auto ordinals = reinterpret_cast<const std::uint16_t*>(&buff[directory->AddressOfNameOrdinals - delta]);
  auto names = reinterpret_cast<const std::uint32_t*>(&buff[directory->AddressOfNames - delta]);

  for (std::size_t j = 0; j < directory->NumberOfNames; ++j) {
    if (functions[ordinals[j]]) {
      os << std::setw(6) << ordinals[j] + directory->Base << std::setw(8) << functions[ordinals[j]] << '\t' << &buff[names[j] - delta] << '\n';
    }
  }

  //for (std::size_t j = 0; j < directory->NumberOfFunctions; ++j) {
  //  if (functions[j]) {
  //    os << std::setw(6) << j + directory->Base << std::setw(8) << functions[j] << '\t';
  //    for (std::size_t k = 0; k < directory->NumberOfNames; ++k) {
  //      if (ordinals[k] == j) {
  //        os << &buff[names[k] - delta] << '\n';
  //      }
  //    }
  //  }
  //}
  return os;
}

constexpr auto snap_by_ordinal(const IMAGE_THUNK_DATA64& thunk) { return IMAGE_SNAP_BY_ORDINAL64(thunk.u1.Ordinal); }
constexpr auto snap_by_ordinal(const IMAGE_THUNK_DATA32& thunk) { return IMAGE_SNAP_BY_ORDINAL32(thunk.u1.Ordinal); }

constexpr auto ordinal(const IMAGE_THUNK_DATA64& thunk) { return IMAGE_ORDINAL64(thunk.u1.Ordinal); }
constexpr auto ordinal(const IMAGE_THUNK_DATA32& thunk) { return IMAGE_ORDINAL32(thunk.u1.Ordinal); }

template <typename THUNK>
std::ostream& dump(std::ostream& os, char* buff, std::uint64_t delta, IMAGE_IMPORT_DESCRIPTOR* descriptor) {
  for (; descriptor->Characteristics; ++descriptor) {
    os << &buff[descriptor->Name - delta] << "\n  " << std::left << *descriptor << std::right << '\n';
       
    for (auto thunk = reinterpret_cast<THUNK*>(&buff[descriptor->OriginalFirstThunk - delta]); thunk->u1.AddressOfData; ++thunk) {
      os << std::setw(16 + 2) << thunk->u1.Ordinal;
      if (!snap_by_ordinal(*thunk)) {
        auto& name = reinterpret_cast<IMAGE_IMPORT_BY_NAME&>(buff[thunk->u1.AddressOfData - delta]);
        os << std::setw(6) << name.Hint << ": " << &name.Name[0] << '\n';
       } else {
        os << std::setw(6) << ordinal(*thunk) << '\n';
      }
    }
    os << '\n';
  }
  return os;
}

std::ostream& dump(std::ostream& os, char* resource, IMAGE_RESOURCE_DIRECTORY* directory, std::size_t depth) {
  os << std::left << std::setw((depth + 1) * 2) << "  " << *directory << '\n';
  auto entry = reinterpret_cast<IMAGE_RESOURCE_DIRECTORY_ENTRY*>(directory + 1);
  for (std::size_t i = 0; i < directory->NumberOfIdEntries + directory->NumberOfNamedEntries; ++i) {
    os << std::setw((depth + 1) * 2) << "  " << entry[i];
    if (entry[i].NameIsString) {
      auto name = reinterpret_cast<IMAGE_RESOURCE_DIR_STRING_U*>(&resource[entry[i].NameOffset]);
      os << "Name: ";
      //os.write(reinterpret_cast<char*>(name->NameString), (name->Length * sizeof(name->NameString[0])));
    } else {
      os << "ID: " << std::setw(4) << entry[i].Id;
    }
    os << '\n';

    if (entry[i].DataIsDirectory) {
      dump(os, resource, reinterpret_cast<IMAGE_RESOURCE_DIRECTORY*>(&resource[entry[i].OffsetToDirectory]), depth + 1);
    } else {
      auto& data = reinterpret_cast<IMAGE_RESOURCE_DATA_ENTRY&>(resource[entry[i].OffsetToData]);
      os << std::setw((depth + 2) * 2) << "  " << data << '\n';
    }
  }
  return os << std::right;
}

std::ostream& dump(std::ostream& os, char* buff, IMAGE_BASE_RELOCATION* relocation) {
  for (IMAGE_BASE_RELOCATION* relocation_next; relocation->SizeOfBlock && relocation->VirtualAddress; relocation = relocation_next) {
    relocation_next = reinterpret_cast<IMAGE_BASE_RELOCATION*>(reinterpret_cast<std::size_t>(relocation) + relocation->SizeOfBlock);

    os << std::left << *relocation << std::right << '\n';
    for (auto entry = reinterpret_cast<WORD*>(relocation + 1); entry != reinterpret_cast<WORD*>(relocation_next); ++entry) {
      os << "  RVA: " << std::setw(8) << (relocation->VirtualAddress + (*entry & 0x0fff)) << "\ttype: " << (*entry >> 12) << '\n';
    }
    os << '\n';
  }
  return os;
}

template <typename NT>
std::ostream& dump(std::ostream& os, char* buff, NT& nt) {
  auto sections = IMAGE_FIRST_SECTION(&nt);

  for (std::size_t i = 0; i < IMAGE_NUMBEROF_DIRECTORY_ENTRIES; ++i) {
    auto virtual_address = nt.OptionalHeader.DataDirectory[i].VirtualAddress;

    for (std::size_t j = 0; j < nt.FileHeader.NumberOfSections; ++j) {
      if (sections[j].VirtualAddress <= virtual_address && virtual_address < (sections[j].VirtualAddress + sections[j].Misc.VirtualSize)) {
        std::uint64_t delta = sections[j].VirtualAddress - sections[j].PointerToRawData;
        auto file_offset = virtual_address - delta;

        switch (i) {
          case IMAGE_DIRECTORY_ENTRY_EXPORT:
            os << "export directory:\n\n";
            dump(os, buff, delta, reinterpret_cast<IMAGE_EXPORT_DIRECTORY*>(&buff[file_offset]));
            break;
          case IMAGE_DIRECTORY_ENTRY_IMPORT:
            os << "import descriptor:\n\n";
            dump<std::conditional_t<std::is_same_v<decltype(nt), IMAGE_NT_HEADERS64>, IMAGE_THUNK_DATA64, IMAGE_THUNK_DATA32>>(os, buff, delta, reinterpret_cast<IMAGE_IMPORT_DESCRIPTOR*>(&buff[file_offset]));
            break;
          case IMAGE_DIRECTORY_ENTRY_RESOURCE:
            os << "resource directory:\n\n";
            dump(os, &buff[file_offset], reinterpret_cast<IMAGE_RESOURCE_DIRECTORY*>(&buff[file_offset]), 0);
            break;
          case IMAGE_DIRECTORY_ENTRY_BASERELOC:
            os << "base relocation:\n\n";
            dump(os, buff, reinterpret_cast<IMAGE_BASE_RELOCATION*>(&buff[file_offset]));
            break;
          default:
            break;
        }
        os << '\n';
      }
    }
  }
  return os;
}

int main(int argc, char* argv[]) try {
  if (argc < 2) {
    std::cerr << argv[0] << " Version " << BINLAB_VERSION_MAJOR << '.' << BINLAB_VERSION_MINOR << '\n';
    std::cout << "Usage: " << argv[0] << " <file>\n";
    return 1;
  }

  if (std::ifstream is{argv[1], std::ios::binary | std::ios::ate}) {
    const auto size = is.tellg();
    std::vector<char> buff(size);
    if (is.seekg(0).read(&buff[0], size)) {
      auto dos = reinterpret_cast<IMAGE_DOS_HEADER*>(&buff[0]);
      auto ptr = reinterpret_cast<IMAGE_NT_HEADERS64*>(&buff[dos->e_lfanew]);
      
      std::cout << std::hex;
      if (ptr->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC) {
        std::cout << "PE64\n";
        dump(std::cout, buff.data(), reinterpret_cast<IMAGE_NT_HEADERS64&>(buff[dos->e_lfanew]));
      } else if (ptr->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC) {
        std::cout << "PE32\n";
        dump(std::cout, buff.data(), reinterpret_cast<IMAGE_NT_HEADERS32&>(buff[dos->e_lfanew]));
      } else {
        std::cerr << "the optional header magic is invalid\n";
      }
    } else {
      std::cerr << "read failed\n";
    }
  } else {
    std::cerr << "open failed\n";
  }

  return 0;
} catch (const std::exception& e) {
  std::cerr << "error: " << e.what() << '\n';
  return 1;
}

