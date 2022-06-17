// tools/bindump/elfdump.h:

#ifndef BINLAB_BINDUMP_ELFDUMP_H_
#define BINLAB_BINDUMP_ELFDUMP_H_

#include <vector>

#include "binlab/BinaryFormat/ELF.h"
#include "binlab/Traits/elf_traits.h"

namespace binlab {

namespace ELF {

void Dump(std::vector<char>& buff);

}  // namespace ELF
}  // namespace binlab

#endif  // !BINLAB_BINDUMP_ELFDUMP_H_
