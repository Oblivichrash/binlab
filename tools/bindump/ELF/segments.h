// segments.h:

#ifndef BINLAB_ELF_SEGMENTS_H_
#define BINLAB_ELF_SEGMENTS_H_

#include <algorithm>
#include <cstdint>
#include <iterator>
#include <type_traits>

#include "ELF/elf_types.h"

namespace binlab {
namespace ELF {

#define EI_NIDENT 16

struct Elf32_Ehdr {
  unsigned char e_ident[EI_NIDENT];  // Magic number and other info
  Elf32_Half    e_type;              // Object file type
  Elf32_Half    e_machine;           // Architecture
  Elf32_Word    e_version;           // Object file version
  Elf32_Addr    e_entry;             // Entry point virtual address
  Elf32_Off     e_phoff;             // Program header table file offset
  Elf32_Off     e_shoff;             // Section header table file offset
  Elf32_Word    e_flags;             // Processor-specific flags
  Elf32_Half    e_ehsize;            // ELF header size in bytes
  Elf32_Half    e_phentsize;         // Program header table entry size
  Elf32_Half    e_phnum;             // Program header table entry count
  Elf32_Half    e_shentsize;         // Section header table entry size
  Elf32_Half    e_shnum;             // Section header table entry count
  Elf32_Half    e_shstrndx;          // Section header string table index
};

struct Elf64_Ehdr {
  unsigned char e_ident[EI_NIDENT];  // Magic number and other info
  Elf64_Half    e_type;              // Object file type
  Elf64_Half    e_machine;           // Architecture
  Elf64_Word    e_version;           // Object file version
  Elf64_Addr    e_entry;             // Entry point virtual address
  Elf64_Off     e_phoff;             // Program header table file offset
  Elf64_Off     e_shoff;             // Section header table file offset
  Elf64_Word    e_flags;             // Processor-specific flags
  Elf64_Half    e_ehsize;            // ELF header size in bytes
  Elf64_Half    e_phentsize;         // Program header table entry size
  Elf64_Half    e_phnum;             // Program header table entry count
  Elf64_Half    e_shentsize;         // Section header table entry size
  Elf64_Half    e_shnum;             // Section header table entry count
  Elf64_Half    e_shstrndx;          // Section header string table index
};

struct Elf32_Phdr {
  Elf32_Word  p_type;     // Segment type
  Elf32_Off   p_offset;   // Segment file offset
  Elf32_Addr  p_vaddr;    // Segment virtual address
  Elf32_Addr  p_paddr;    // Segment physical address
  Elf32_Word  p_filesz;   // Segment size in file
  Elf32_Word  p_memsz;    // Segment size in memory
  Elf32_Word  p_flags;    // Segment flags
  Elf32_Word  p_align;    // Segment alignment
};

struct Elf64_Phdr {
  Elf64_Word  p_type;     // Segment type
  Elf64_Word  p_flags;    // Segment flags
  Elf64_Off   p_offset;   // Segment file offset
  Elf64_Addr  p_vaddr;    // Segment virtual address
  Elf64_Addr  p_paddr;    // Segment physical address
  Elf64_Xword p_filesz;   // Segment size in file
  Elf64_Xword p_memsz;    // Segment size in memory
  Elf64_Xword p_align;    // Segment alignment
};

class segments {
 public:
  using value_type          = Elf64_Off;  // address
  using size_type           = std::size_t;
  using difference_type     = std::ptrdiff_t;
  using const_iterator      = const value_type*;

  segments(Elf64_Phdr* first, Elf64_Phdr* last) : first_{first}, last_{last} {}
  segments(Elf64_Phdr* first, size_type size) : segments{first, first + size} {}

  // Bucket interface
  const_iterator begin(size_type n) const noexcept {
    return first_[n].p_offset;
  }
  const_iterator end(size_type n) const noexcept {
    return first_[n].p_offset + first_[n].p_filesz;
  }
  size_type segment_count() const noexcept {
    return last_ - first_;
  }
  size_type segment_size(size_type n) const noexcept {
    return first_[n].p_filesz;
  }

 private:
  Elf64_Phdr first_;
  Elf64_Phdr last_;
};

template <typename SegmentIt, typename AddressType>
auto virtual_offset_to_file_offset(SegmentIt first, SegmentIt last, AddressType address) -> AddressType {
  using Traits          = traits::segment_traits<typename std::iterator_traits<SegmentIt>::value_type>;

  using difference_type = typename Traits::difference_type;
  using address_type    = typename Traits::address_type;

  static_assert(std::is_convertible_v<AddressType, address_type>, "address type error");

  address_type result = 0;
  for (auto iter = first; iter != last; ++iter) {
    address_type base = Traits::virtual_offset(*iter);
    if (std::clamp(address, base, base + Traits::virtual_size(*iter)) == address) {
      difference_type offset = address - base;
      result = Traits::file_offset(*iter) + offset;
    }
  }
  return result;
}

template <typename SegmentIt, typename AddressType>
auto file_offset_to_virtual_offset(SegmentIt first, SegmentIt last, AddressType address) -> AddressType {
  using Traits          = traits::segment_traits<typename std::iterator_traits<SegmentIt>::value_type>;

  using difference_type = typename Traits::difference_type;
  using address_type    = typename Traits::address_type;

  static_assert(std::is_convertible_v<AddressType, address_type>, "address type error");

  address_type result = 0;
  for (auto iter = first; iter != last; ++iter) {
    address_type base = Traits::file_offset(*iter);
    if (std::clamp(address, base, base + Traits::file_size(*iter)) == address) {
      difference_type offset = address - base;
      result = Traits::virtual_offset(*iter) + offset;
    }
  }
  return result;
}

}  // namespace ELF
}  // namespace binlab

#endif  // !BINLAB_ELF_SEGMENTS_H_
