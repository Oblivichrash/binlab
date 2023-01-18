// elf_types.h:

#ifndef BINLAB_ELF_TYPES_H_
#define BINLAB_ELF_TYPES_H_

#include <cstdint>

namespace binlab {
namespace ELF {

using Elf32_Addr    = std::uint32_t; // Program address
using Elf32_Off     = std::uint32_t;  // File offset
using Elf32_Half    = std::uint16_t;
using Elf32_Word    = std::uint32_t;
using Elf32_Sword   = std::int32_t;
using Elf32_Section = std::uint16_t;

using Elf64_Addr    = std::uint64_t;
using Elf64_Off     = std::uint64_t;
using Elf64_Half    = std::uint16_t;
using Elf64_Word    = std::uint32_t;
using Elf64_Sword   = std::int32_t;
using Elf64_Xword   = std::uint64_t;
using Elf64_Sxword  = std::int64_t;
using Elf64_Section = std::uint16_t;

}  // namespace ELF
}  // namespace binlab

#endif  // !BINLAB_ELF_TYPES_H_
