// tools/bindump/main.cpp: Dump Binary files

#include <cstring>
#include <fstream>
#include <iostream>
#include <vector>

#include "binlab/Config/Config.h"
#include "binlab/BinaryFormat/COFF.h"
#include "binlab/BinaryFormat/ELF.h"

using namespace binlab::COFF;
using namespace binlab::ELF;

namespace binlab {

template <typename Section>
struct section_traits;

template <>
struct section_traits<Elf64_Phdr> {
  using value_type              = Elf64_Phdr;
  using size_type               = decltype(value_type::p_memsz);
  using offset_type             = decltype(value_type::p_offset);
  using address_type            = decltype(value_type::p_vaddr);

  [[nodiscard]] static constexpr auto memory_offset(const value_type& section) {
    return section.p_vaddr;
  }
  [[nodiscard]] static constexpr auto memory_size(const value_type& section) {
    return section.p_memsz;
  }

  [[nodiscard]] static constexpr auto file_offset(const value_type& section) {
    return section.p_offset;
  }
  [[nodiscard]] static constexpr auto file_size(const value_type& section) {
    return section.p_filesz;
  }
};

template <>
struct section_traits<Elf32_Phdr> {
  using value_type              = Elf32_Phdr;
  using size_type               = decltype(value_type::p_memsz);
  using offset_type             = decltype(value_type::p_offset);
  using address_type            = decltype(value_type::p_vaddr);

  [[nodiscard]] static constexpr auto memory_offset(const value_type& section) {
    return section.p_vaddr;
  }
  [[nodiscard]] static constexpr auto memory_size(const value_type& section) {
    return section.p_memsz;
  }

  [[nodiscard]] static constexpr auto file_offset(const value_type& section) {
    return section.p_offset;
  }
  [[nodiscard]] static constexpr auto file_size(const value_type& section) {
    return section.p_filesz;
  }
};

template <>
struct section_traits<Elf64_Shdr> {
  using value_type              = Elf64_Shdr;
  using size_type               = decltype(value_type::sh_size);
  using offset_type             = decltype(value_type::sh_offset);
  using address_type            = decltype(value_type::sh_addr);

  [[nodiscard]] static constexpr auto memory_offset(const value_type& section) {
    return section.sh_addr;
  }
  [[nodiscard]] static constexpr auto memory_size(const value_type& section) {
    return section.sh_size;
  }

  [[nodiscard]] static constexpr auto file_offset(const value_type& section) {
    return section.sh_offset;
  }
  [[nodiscard]] static constexpr auto file_size(const value_type& section) {
    return section.sh_size;
  }
};

template <>
struct section_traits<Elf32_Shdr> {
  using value_type              = Elf32_Shdr;
  using size_type               = decltype(value_type::sh_size);
  using offset_type             = decltype(value_type::sh_offset);
  using address_type            = decltype(value_type::sh_addr);

  [[nodiscard]] static constexpr auto memory_offset(const value_type& section) {
    return section.sh_addr;
  }
  [[nodiscard]] static constexpr auto memory_size(const value_type& section) {
    return section.sh_size;
  }

  [[nodiscard]] static constexpr auto file_offset(const value_type& section) {
    return section.sh_offset;
  }
  [[nodiscard]] static constexpr auto file_size(const value_type& section) {
    return section.sh_size;
  }
};

template <>
struct section_traits<IMAGE_SECTION_HEADER> {
  using value_type              = IMAGE_SECTION_HEADER;
  using size_type               = decltype(value_type::SizeOfRawData);
  using offset_type             = decltype(value_type::PointerToRawData);
  using address_type            = decltype(value_type::VirtualAddress);

  [[nodiscard]] static constexpr auto memory_offset(const value_type& section) {
    return section.VirtualAddress;
  }
  [[nodiscard]] static constexpr auto memory_size(const value_type& section) {
    return section.Misc.VirtualSize;
  }

  [[nodiscard]] static constexpr auto file_offset(const value_type& section) {
    return section.PointerToRawData;
  }
  [[nodiscard]] static constexpr auto file_size(const value_type& section) {
    return section.SizeOfRawData;
  }
};

enum class address_mode { file, memory };

template <address_mode mode>
class address_policy;

template <>
class address_policy<address_mode::memory> {
 public:
  template <typename Section, typename Traits = section_traits<Section>>
  [[nodiscard]] static constexpr auto offset(const Section& header) noexcept {
    return Traits::memory_offset(header);
  }

  template <typename Section, typename Traits = section_traits<Section>>
  [[nodiscard]] static constexpr auto count(const Section& header) noexcept {
    return Traits::memory_size(header);
  }

  template <typename Section, typename Address, typename Traits = section_traits<Section>>
  [[nodiscard]] static constexpr bool contains(const Section& header, const Address& address) noexcept {
    return offset(header) <= address && address < (offset(header) + count(header));
  }
};

template <>
class address_policy<address_mode::file> {
 public:
  template <typename Section, typename Traits = section_traits<Section>>
  [[nodiscard]] static constexpr auto offset(const Section& header) noexcept {
    return Traits::file_offset(header);
  }

  template <typename Section, typename Traits = section_traits<Section>>
  [[nodiscard]] static constexpr auto count(const Section& header) noexcept {
    return Traits::file_size(header);
  }

  template <typename Section, typename Address, typename Traits = section_traits<Section>>
  [[nodiscard]] static constexpr bool contains(const Section& header, const Address& address) noexcept {
    return offset(header) <= address && address < (offset(header) + count(header));
  }
};

template <typename Header, typename Section>
struct header_traits;

template <>
struct header_traits<Elf64_Ehdr, Elf64_Phdr> {
  using traits                  = section_traits<Elf64_Phdr>;

  using header_type             = Elf64_Ehdr;
  using section_type            = typename traits::value_type;
  using size_type               = typename traits::size_type;
  using offset_type             = typename traits::offset_type;
  using address_type            = typename traits::address_type;

  [[nodiscard]] static constexpr bool check_magic(const header_type& header) noexcept {
    return std::equal(&header.e_ident[EI_MAG0], &header.e_ident[EI_MAG0] + SELFMAG, ELFMAG);
  }

  [[nodiscard]] static constexpr bool check_class(const header_type& header) noexcept {
    return header.e_ident[EI_CLASS] == ELFCLASS64;
  }

  [[nodiscard]] static constexpr auto count(const header_type& header) noexcept {
    return header.e_phnum;
  }

  [[nodiscard]] static inline auto begin(const header_type& header) noexcept {
    auto address = reinterpret_cast<std::size_t>(&header) + header.e_phoff;
    return reinterpret_cast<section_type*>(address);
  }

  [[nodiscard]] static inline auto end(const header_type& header) noexcept {
    return begin(header) + count(header);
  }
};

template <>
struct header_traits<Elf32_Ehdr, Elf32_Phdr> {
  using traits                  = section_traits<Elf32_Phdr>;

  using header_type             = Elf32_Ehdr;
  using section_type            = typename traits::value_type;
  using size_type               = typename traits::size_type;
  using offset_type             = typename traits::offset_type;
  using address_type            = typename traits::address_type;

  [[nodiscard]] static constexpr bool check_magic(const header_type& header) noexcept {
    return std::equal(&header.e_ident[EI_MAG0], &header.e_ident[EI_MAG0] + SELFMAG, ELFMAG);
  }

  [[nodiscard]] static constexpr bool check_class(const header_type& header) noexcept {
    return header.e_ident[EI_CLASS] == ELFCLASS32;
  }

  [[nodiscard]] static constexpr auto count(const header_type& header) noexcept {
    return header.e_phnum;
  }

  [[nodiscard]] static inline auto begin(const header_type& header) noexcept {
    auto address = reinterpret_cast<std::size_t>(&header) + header.e_phoff;
    return reinterpret_cast<section_type*>(address);
  }

  [[nodiscard]] static inline auto end(const header_type& header) noexcept {
    return begin(header) + count(header);
  }
};

template <>
struct header_traits<Elf64_Ehdr, Elf64_Shdr> {
  using traits                  = section_traits<Elf64_Shdr>;

  using header_type             = Elf64_Ehdr;
  using section_type            = typename traits::value_type;
  using size_type               = typename traits::size_type;
  using offset_type             = typename traits::offset_type;
  using address_type            = typename traits::address_type;

  [[nodiscard]] static constexpr bool check_magic(const header_type& header) noexcept {
    return std::equal(&header.e_ident[EI_MAG0], &header.e_ident[EI_MAG0] + SELFMAG, ELFMAG);
  }

  [[nodiscard]] static constexpr bool check_class(const header_type& header) noexcept {
    return header.e_ident[EI_CLASS] == ELFCLASS64;
  }

  [[nodiscard]] static constexpr auto count(const header_type& header) noexcept {
    return header.e_shnum;
  }

  [[nodiscard]] static inline auto begin(const header_type& header) noexcept {
    auto address = reinterpret_cast<std::size_t>(&header) + header.e_shoff;
    return reinterpret_cast<section_type*>(address);
  }

  [[nodiscard]] static inline auto end(const header_type& header) noexcept {
    return begin(header) + count(header);
  }
};

template <>
struct header_traits<Elf32_Ehdr, Elf32_Shdr> {
  using traits                  = section_traits<Elf32_Shdr>;

  using header_type             = Elf32_Ehdr;
  using section_type            = typename traits::value_type;
  using size_type               = typename traits::size_type;
  using offset_type             = typename traits::offset_type;
  using address_type            = typename traits::address_type;

  [[nodiscard]] static constexpr bool check_magic(const header_type& header) noexcept {
    return std::equal(&header.e_ident[EI_MAG0], &header.e_ident[EI_MAG0] + SELFMAG, ELFMAG);
  }

  [[nodiscard]] static constexpr bool check_class(const header_type& header) noexcept {
    return header.e_ident[EI_CLASS] == ELFCLASS32;
  }

  [[nodiscard]] static constexpr auto count(const header_type& header) noexcept {
    return header.e_shnum;
  }

  [[nodiscard]] static inline auto begin(const header_type& header) noexcept {
    auto address = reinterpret_cast<std::size_t>(&header) + header.e_shoff;
    return reinterpret_cast<section_type*>(address);
  }

  [[nodiscard]] static inline auto end(const header_type& header) noexcept {
    return begin(header) + count(header);
  }
};

template <>
struct header_traits<IMAGE_NT_HEADERS64, IMAGE_SECTION_HEADER> {
  using traits                  = section_traits<IMAGE_SECTION_HEADER>;

  using header_type             = IMAGE_NT_HEADERS64;
  using section_type            = typename traits::value_type;
  using size_type               = typename traits::size_type;
  using offset_type             = typename traits::offset_type;
  using address_type            = typename traits::address_type;

  [[nodiscard]] static constexpr bool check_magic(const header_type& header) noexcept {
    return header.Signature == IMAGE_NT_SIGNATURE;
  }

  [[nodiscard]] static constexpr bool check_class(const header_type& header) noexcept {
    return header.OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC;
  }

  [[nodiscard]] static constexpr auto count(const header_type& header) noexcept {
    return header.FileHeader.NumberOfSections;
  }

  [[nodiscard]] static inline auto begin(const header_type& header) noexcept {
    auto address = reinterpret_cast<std::size_t>(&header) + offsetof(header_type, OptionalHeader) + header.FileHeader.SizeOfOptionalHeader;
    return reinterpret_cast<section_type*>(address);
  }

  [[nodiscard]] static inline auto end(const header_type& header) noexcept {
    return begin(header) + count(header);
  }
};

template <>
struct header_traits<IMAGE_NT_HEADERS32, IMAGE_SECTION_HEADER> {
  using traits                  = section_traits<IMAGE_SECTION_HEADER>;

  using header_type             = IMAGE_NT_HEADERS32;
  using section_type            = typename traits::value_type;
  using size_type               = typename traits::size_type;
  using offset_type             = typename traits::offset_type;
  using address_type            = typename traits::address_type;

  [[nodiscard]] static constexpr bool check_magic(const header_type& header) noexcept {
    return header.Signature == IMAGE_NT_SIGNATURE;
  }

  [[nodiscard]] static constexpr bool check_class(const header_type& header) noexcept {
    return header.OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC;
  }

  [[nodiscard]] static constexpr auto count(const header_type& header) noexcept {
    return header.FileHeader.NumberOfSections;
  }

  [[nodiscard]] static inline auto begin(const header_type& header) noexcept {
    auto address = reinterpret_cast<std::size_t>(&header) + offsetof(header_type, OptionalHeader) + header.FileHeader.SizeOfOptionalHeader;
    return reinterpret_cast<section_type*>(address);
  }

  [[nodiscard]] static inline auto end(const header_type& header) noexcept {
    return begin(header) + count(header);
  }
};

}  // namespace binlab

using namespace binlab;

std::ostream& coff_dump(std::ostream& os, char* buff) {
  auto& Dos = reinterpret_cast<IMAGE_DOS_HEADER&>(buff[0]);
  if (Dos.e_magic != IMAGE_DOS_SIGNATURE) {
    return os << "invalid DOS magic\n";
  }

  using traits = header_traits<IMAGE_NT_HEADERS64, IMAGE_SECTION_HEADER>;
  auto& Nt = reinterpret_cast<IMAGE_NT_HEADERS64&>(buff[Dos.e_lfanew]);
  if (!traits::check_magic(Nt) || !traits::check_class(Nt)) {
    return os << "invalid NT type\n";
  }

  auto first = traits::begin(Nt), last = traits::end(Nt);
  os << std::hex;

  for (auto iter = first; iter != last; ++iter) {
    if (iter->Characteristics & IMAGE_SCN_MEM_EXECUTE) {
      os << std::string{reinterpret_cast<char*>(iter->Name), sizeof(iter->Name)} << '\t';
      os << "offset: " << iter->PointerToRawData << '\t';
      os << "size: " << iter->SizeOfRawData << '\t';
      os << "Characteristics: " << iter->Characteristics << '\n';
    }
  }
  
  return os;
}

std::ostream& elf_dump(std::ostream& os, char* buff) {
  using traits = header_traits<Elf64_Ehdr, Elf64_Shdr>;

  auto& ehdr = reinterpret_cast<Elf64_Ehdr&>(buff[0]);
  if (!traits::check_magic(ehdr) || !traits::check_class(ehdr)) {
    return os << "invalid ELF type\n";
  }

  auto first = traits::begin(ehdr), last = traits::end(ehdr);
  auto shstr = &buff[first[ehdr.e_shstrndx].sh_offset];

  for (auto iter = first; iter != last; ++iter) {
    os << &shstr[iter->sh_name] << '\n';
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
    const auto count = static_cast<std::size_t>(is.tellg());
    std::vector<char> buff(count);
    if (is.seekg(0, std::ios::beg).read(&buff[0], count)) {
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
