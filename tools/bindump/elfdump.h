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

namespace binlab {
namespace ELF {

class Accessor {
 public:
  using Section = ELF::Elf64_Phdr;
  using Address = std::uint64_t;

  Accessor(void* base, const Section* first, const Address num)
      : base_{static_cast<char*>(base)}, first_{first}, last_{first + num} {}

  const char& operator[](Address offset) const { return base_[offset]; }

 private:
  char* base_;
  const Section* first_;
  const Section* last_;
};

void Dump(std::vector<char>& buff);
void Dump64LE(std::vector<char>& buff);

void Dump(const Accessor& base, const ELF::Elf64_Dyn* dyns);

}  // namespace ELF
}  // namespace binlab

#endif  // !BINLAB_BINDUMP_ELFDUMP_H_
