// tools/pedump/main.cpp: Dump Binary files

#include <Windows.h>

#include <fstream>
#include <iomanip>
#include <iostream>
#include <span>
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

  //template <typename Type, std::enable_if_t<std::is_pointer_v<Type> || std::is_reference_v<Type>, int> = 0>
  //Type object_cast() noexcept { return reinterpret_cast<Type>(value_); }

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

constexpr auto snap_by_ordinal(const IMAGE_THUNK_DATA64& thunk) { return IMAGE_SNAP_BY_ORDINAL64(thunk.u1.Ordinal); }
constexpr auto snap_by_ordinal(const IMAGE_THUNK_DATA32& thunk) { return IMAGE_SNAP_BY_ORDINAL32(thunk.u1.Ordinal); }

constexpr auto ordinal(const IMAGE_THUNK_DATA64& thunk) { return IMAGE_ORDINAL64(thunk.u1.Ordinal); }
constexpr auto ordinal(const IMAGE_THUNK_DATA32& thunk) { return IMAGE_ORDINAL32(thunk.u1.Ordinal); }

template <typename THUNK_DATA, typename NT_HEADER>
std::ostream& import_dump(std::ostream& os, void* base, NT_HEADER& nt) {
  auto buff = static_cast<char*>(base);

  auto sections = IMAGE_FIRST_SECTION(&nt);
  for (std::size_t i = 0; i < nt.FileHeader.NumberOfSections; ++i) {
    auto virtual_address = nt.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
    if (sections[i].VirtualAddress < virtual_address && virtual_address < (sections[i].VirtualAddress + sections[i].Misc.VirtualSize)) {
      os << sections[i].Name << '\n';
      auto delta = sections[i].VirtualAddress - sections[i].PointerToRawData;

      for (auto descriptor = reinterpret_cast<IMAGE_IMPORT_DESCRIPTOR*>(&buff[virtual_address - delta]); descriptor->Name; ++descriptor) {
        std::cout << &buff[descriptor->Name - delta] << '\n';

        for (auto thunk = reinterpret_cast<THUNK_DATA*>(&buff[descriptor->OriginalFirstThunk - delta]); thunk->u1.Ordinal; ++thunk) {
          if (!snap_by_ordinal(*thunk)) {
            std::cout << '\t' << &reinterpret_cast<IMAGE_IMPORT_BY_NAME&>(buff[thunk->u1.Function - delta]).Name[0] << '\n';
          } else {
            std::cout << '\t' << ordinal(*thunk) << '\n';
          }
        }
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
      auto nt = reinterpret_cast<IMAGE_NT_HEADERS64*>(&buff[dos->e_lfanew]);
      if (nt->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC) {
        std::cout << "PE64\n";
        import_dump<IMAGE_THUNK_DATA64>(std::cout, buff.data(),reinterpret_cast<IMAGE_NT_HEADERS64&>(buff[dos->e_lfanew]));
      } else if (nt->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC) {
        std::cout << "PE32\n";
        import_dump<IMAGE_THUNK_DATA32>(std::cout, buff.data(), reinterpret_cast<IMAGE_NT_HEADERS32&>(buff[dos->e_lfanew]));
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

