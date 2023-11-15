// tools/bindump/pedump.h:

#ifndef BINLAB_BINDUMP_PEDUMP_H_
#define BINLAB_BINDUMP_PEDUMP_H_

#include <vector>

#include "binlab/BinaryFormat/COFF.h"
#include "binlab/Traits/pe_traits.h"

namespace binlab {

namespace COFF {

void Dump(std::vector<char>& buff);

}  // namespace PE
}  // namespace binlab

#endif  // !BINLAB_BINDUMP_PEDUMP_H_
