// tools/bindump/elfdump.h:

#ifndef BINLAB_BINDUMP_ELF_DUMP_H_
#define BINLAB_BINDUMP_ELF_DUMP_H_

#include <cstddef>

namespace binlab {
namespace ELF {

int dump_sym64(char *buff, std::size_t size);

}  // namespace ELF
}  // namespace binlab

#endif  // !BINLAB_BINDUMP_ELF_DUMP_H_
