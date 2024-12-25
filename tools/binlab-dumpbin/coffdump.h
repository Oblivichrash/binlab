// tools/bindump/coffdump.h:

#ifndef BINLAB_BINDUMP_COFF_DUMP_H_
#define BINLAB_BINDUMP_COFF_DUMP_H_

#include <cstddef>

namespace binlab {
namespace COFF {

int dump_import64(const char *buff, std::size_t size);
int dump_import32(const char *buff, std::size_t size);

int dump_obj(const char *buff, std::size_t size);

}  // namespace COFF
}  // namespace binlab

#endif  // !BINLAB_BINDUMP_COFF_DUMP_H_
