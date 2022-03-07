// tools/bindump/elfdump.h:

#ifndef BINLAB_BINDUMP_ELFDUMP_H_
#define BINLAB_BINDUMP_ELFDUMP_H_

#include <algorithm>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <type_traits>
#include <vector>

#include "binlab/BinaryFormat/ELF.h"
#include "segments.h"

namespace binlab {

template <>
struct segment_traits<ELF::Elf64_Phdr> {
  using value_type = ELF::Elf64_Phdr;
  using address_type = std::uint64_t;
  using size_type = std::uint64_t;

  static constexpr address_type virtual_address(const value_type& segment) { return segment.p_vaddr; }
  static constexpr address_type file_offset(const value_type& segment) { return segment.p_offset; }

  static constexpr size_type file_size(const value_type& segment) { return segment.p_filesz; }
  static constexpr size_type memory_size(const value_type& segment) { return segment.p_memsz; }
};

template <>
struct segment_traits<ELF::Elf32_Phdr> {
  using value_type = ELF::Elf32_Phdr;
  using address_type = std::uint64_t;
  using size_type = std::uint64_t;

  static constexpr address_type virtual_address(const value_type& segment) { return segment.p_vaddr; }
  static constexpr address_type file_offset(const value_type& segment) { return segment.p_offset; }

  static constexpr size_type file_size(const value_type& segment) { return segment.p_filesz; }
  static constexpr size_type memory_size(const value_type& segment) { return segment.p_memsz; }
};

namespace ELF {

class Accessor : segments<ELF::Elf64_Phdr> {
 public:
  Accessor(void* base, const_iterator first, size_type num) : segments(first, num), base_{static_cast<char*>(base)} {}

  const char& operator[](address_type offset) const { return base_[offset]; }

 private:
  char* base_;
};

void Dump(std::vector<char>& buff);
void Dump64LE(std::vector<char>& buff);

void Dump(const Accessor& base, const Elf64_Dyn* dyn);

}  // namespace ELF
}  // namespace binlab

#endif  // !BINLAB_BINDUMP_ELFDUMP_H_
