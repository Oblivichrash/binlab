// binlab/Traits/elf_traits.h:

#ifndef BINLAB_TRAITS_ELF_TRAITS_H_
#define BINLAB_TRAITS_ELF_TRAITS_H_

#include <cstdint>

#include "binlab/BinaryFormat/ELF.h"
#include "binlab/Traits/bin_traits.h"

namespace binlab {
namespace traits {

template <>
struct segment_traits<ELF::Elf64_Phdr> {
  using value_type        = ELF::Elf64_Phdr;
  using reference         = value_type&;
  using const_reference   = const value_type&;
  using size_type         = std::uint64_t;
  using difference_type   = std::ptrdiff_t;
  using iterator          = value_type*;
  using const_iterator    = const iterator;

  using address_type      = std::uint64_t;

  static constexpr address_type virtual_offset(const_reference segment) { return segment.p_vaddr; }
  static constexpr size_type virtual_size(const_reference segment) { return segment.p_memsz; }

  static constexpr address_type file_offset(const_reference segment) { return segment.p_offset; }
  static constexpr size_type file_size(const_reference segment) { return segment.p_filesz; }
};

template <>
struct segment_traits<ELF::Elf32_Phdr> {
  using value_type        = ELF::Elf32_Phdr;
  using reference         = value_type&;
  using const_reference   = const value_type&;
  using size_type         = std::uint64_t;
  using difference_type   = std::ptrdiff_t;
  using iterator          = value_type*;
  using const_iterator    = const iterator;

  using address_type      = std::uint64_t;

  static constexpr address_type virtual_offset(const_reference segment) { return segment.p_vaddr; }
  static constexpr size_type virtual_size(const_reference segment) { return segment.p_memsz; }

  static constexpr address_type file_offset(const_reference segment) { return segment.p_offset; }
  static constexpr size_type file_size(const_reference segment) { return segment.p_filesz; }
};

}  // namespace traits
}  // namespace binlab

#endif  // !BINLAB_TRAITS_ELF_TRAITS_H_
