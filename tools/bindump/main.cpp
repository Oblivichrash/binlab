// tools/bindump/main.cpp: Dump Binary files

#include <fstream>
#include <iostream>
#include <vector>

#include "binlab/Config/Config.h"
#include "binlab/BinaryFormat/COFF.h"
#include "binlab/BinaryFormat/ELF.h"
//#include "pedump.h"
//#include "elfdump.h"

namespace binlab {

enum class address_mode { kFile, kVirtual };

template <typename Section, address_mode mode>
struct section_traits {};

template <typename Header, typename SectionTraits>
struct header_traits {};

namespace COFF {

template <typename Section, address_mode mode>
struct section_traits : private binlab::section_traits<Section, mode> {
  using value_type              = Section;
  using size_type               = decltype(Section::SizeOfRawData);
  using address_type            = decltype(Section::VirtualAddress);

  [[nodiscard]] static constexpr auto offset(const Section& header) noexcept {
    if constexpr (mode == address_mode::kFile) {
      return header.PointerToRawData;
    } else {
      return header.VirtualAddress;
    }
  }

  [[nodiscard]] static constexpr auto size(const Section& header) noexcept {
    if constexpr (mode == address_mode::kFile) {
      return header.Misc.VirtualSize;
    } else {
      return header.SizeOfRawData;
    }
  }

  [[nodiscard]] static constexpr bool contains(const Section& header, const address_type& address) noexcept {
    return offset(header) <= address && address < offset(header) + size(header);
  }

  template <typename Header>
  [[nodiscard]] static constexpr auto first(const Header& header) noexcept {
    auto address = reinterpret_cast<std::size_t>(&header) + offsetof(Header, OptionalHeader) + header.FileHeader.SizeOfOptionalHeader;
    return reinterpret_cast<Section*>(address);
  }

  template <typename Header>
  [[nodiscard]] static constexpr auto count(const Header& header) noexcept {
    return header.FileHeader.NumberOfSections;
  }
};

template <typename Header, typename SectionTraits>
struct header_traits : private binlab::header_traits<Header, SectionTraits> {
  using size_type               = decltype(decltype(Header::FileHeader)::NumberOfSections);

  using section_iterator        = typename SectionTraits::value_type*;

  [[nodiscard]] static constexpr bool check_magic(const Header& header) noexcept {
    return header.Signature == IMAGE_NT_SIGNATURE;
  }

  [[nodiscard]] static constexpr auto begin(const Header& header) noexcept {
    return SectionTraits::first(header);
  }

  [[nodiscard]] static constexpr auto size(const Header& header) noexcept {
    return SectionTraits::count(header);
  }
};

}  // namespace COFF

namespace ELF {

template <typename Section, address_mode mode>
struct section_traits : private binlab::section_traits<Section, mode> {
  using value_type              = Section;
  using size_type               = decltype(Section::sh_size);
  using address_type            = decltype(Section::sh_addr);
  using offset_type             = decltype(Section::sh_offset);

  [[nodiscard]] static constexpr auto offset(const Section& header) noexcept {
    if constexpr (mode == address_mode::kFile) {
      return header.sh_offset;
    } else {
      return header.sh_addr;
    }
  }

  [[nodiscard]] static constexpr auto size(const Section& header) noexcept {
    return header.sh_size;
  }

  [[nodiscard]] static constexpr bool contains(const Section& header, const address_type& address) noexcept {
    return offset(header) <= address && address < offset(header) + size(header);
  }

  template <typename Header>
  [[nodiscard]] static constexpr auto first(const Header& header) noexcept {
    auto address = reinterpret_cast<std::size_t>(&header) + header.e_shoff;
    return reinterpret_cast<Section*>(address);
  }

  template <typename Header>
  [[nodiscard]] static constexpr auto count(const Header& header) noexcept {
    return header.e_shnum;
  }
};

template <typename Section, address_mode mode>
struct segment_traits : private binlab::section_traits<Section, mode> {
  using value_type              = Section;
  using size_type               = decltype(Section::p_memsz);
  using address_type            = decltype(Section::p_vaddr);
  using offset_type             = decltype(Section::p_offset);

  [[nodiscard]] static constexpr auto offset(const Section& header) noexcept {
    if constexpr (mode == address_mode::kFile) {
      return header.p_offset;
    } else {
      return header.p_vaddr;
    }
  }

  [[nodiscard]] static constexpr auto size(const Section& header) noexcept {
    if constexpr (mode == address_mode::kFile) {
      return header.p_filesz;
    } else {
      return header.p_memsz;
    }
  }

  [[nodiscard]] static constexpr bool contains(const Section& header, const address_type& address) noexcept {
    return offset(header) <= address && address < offset(header) + size(header);
  }

  template <typename Header>
  [[nodiscard]] static constexpr auto first(const Header& header) noexcept {
    auto address = reinterpret_cast<std::size_t>(&header) + header.e_phoff;
    return reinterpret_cast<Section*>(address);
  }

  template <typename Header>
  [[nodiscard]] static constexpr auto count(const Header& header) noexcept {
    return header.e_phnum;
  }
};

template <typename Header, typename SectionTraits>
struct header_traits : private binlab::header_traits<Header, SectionTraits> {
  using size_type               = decltype(Header::e_shnum);

  [[nodiscard]] static constexpr bool check_magic(const Header& header) noexcept {
    return !std::memcmp(&header.e_ident[EI_MAG0], ELFMAG, SELFMAG);
  }

  [[nodiscard]] static constexpr auto begin(const Header& header) noexcept {
    return SectionTraits::first(header);
  }

  [[nodiscard]] static constexpr auto size(const Header& header) noexcept {
    return SectionTraits::count(header);
  }
};

}  // namespace ELF
}  // namespace binlab

using namespace binlab;
using namespace COFF;
using namespace ELF;

std::ostream& coff_dump(std::ostream& os, char* buff) {
  using traits = COFF::header_traits<IMAGE_NT_HEADERS64, COFF::section_traits<IMAGE_SECTION_HEADER, address_mode::kFile>>;

  auto& Dos = reinterpret_cast<IMAGE_DOS_HEADER&>(buff[0]);
  if (Dos.e_magic != IMAGE_DOS_SIGNATURE) {
    return os << "invalid DOS magic\n";
  }

  auto& Nt = reinterpret_cast<IMAGE_NT_HEADERS64&>(buff[Dos.e_lfanew]);
  if (!traits::check_magic(Nt)) {
    return os << "invalid NT magic\n";
  }

  auto iter = traits::begin(Nt);
  for (std::size_t i = 0; i < traits::size(Nt); ++i) {
    std::cout << iter[i].Name << '\n';
  }
  return os;
}

std::ostream& elf_dump(std::ostream& os, char* buff) {
  using traits = ELF::header_traits<Elf64_Ehdr, ELF::section_traits<Elf64_Shdr, address_mode::kFile>>;

  auto& ehdr = reinterpret_cast<Elf64_Ehdr&>(buff[0]);
  if (!traits::check_magic(ehdr)) {
    return os << "invalid ELF magic\n";
  }

  auto sections = traits::begin(ehdr);
  auto shstr = &buff[sections[ehdr.e_shstrndx].sh_offset];
  for (std::size_t i = 0; i < traits::size(ehdr); ++i) {
    std::cout << &shstr[sections[i].sh_name] << '\n';
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
    const auto size = static_cast<std::size_t>(is.tellg());
    std::vector<char> buff(size);
    if (is.seekg(0, std::ios::beg).read(&buff[0], size)) {
      switch (buff[0]) {
        case 'M':
          coff_dump(std::cout, &buff[0]);
          break;
        case 0x7f:  // ELFMAG0
          elf_dump(std::cout, &buff[0]);
          break;
        default:
          break;
      }
    }
  }

  return 0;
} catch (const std::exception& e) {
  std::cerr << "error: " << e.what() << '\n';
  return 1;
}
