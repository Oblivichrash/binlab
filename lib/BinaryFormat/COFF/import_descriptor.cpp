// import_descriptor.cpp:

#include "import_descriptor.h"

#include <iomanip>

using namespace binlab::COFF;

std::ostream& binlab::COFF::operator<<(std::ostream& os, const IMAGE_IMPORT_DESCRIPTOR& descriptor) {
  os << "OriginalFirstThunk: " << std::setw(8) << descriptor.OriginalFirstThunk;
  os << "TimeDateStamp: " << std::setw(8) << descriptor.TimeDateStamp;
  os << "ForwarderChain: " << std::setw(8) << descriptor.ForwarderChain;
  os << "Name: " << std::setw(8) << descriptor.Name;
  os << "FirstThunk: " << std::setw(8) << descriptor.FirstThunk;
  return os;
}

std::ostream& binlab::COFF::dump_import64(std::ostream& os, binlab::COFF::section_ref& section, std::size_t vaddr) {
  import_table64_ref table(vaddr, section);
  for (auto& descriptor : table) {
    os << &section[descriptor.Name] << '\n';
    for (auto thunk = table.begin(descriptor); thunk != std::default_sentinel; ++thunk) {
      if (!IMAGE_SNAP_BY_ORDINAL64(thunk->u1.Ordinal)) {
        auto& name = reinterpret_cast<IMAGE_IMPORT_BY_NAME&>(section[thunk->u1.AddressOfData]);
        os << std::setw(6) << name.Hint << ": " << &name.Name[0] << '\n';
      } else {
        os << std::setw(6) << IMAGE_ORDINAL64(thunk->u1.Ordinal) << '\n';
      }
    }
  }
  os << '\n';
  return os;
}