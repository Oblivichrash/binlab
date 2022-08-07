//

#include "elf_stream.h"

#include <iomanip>
#include <ostream>

std::ostream& operator<<(std::ostream& os, const Elf32_Sym& sym) {
  os << "name: " << std::setw(8) << sym.st_name;
  os << "info: " << std::setw(4) << unsigned{sym.st_info};
  os << "other: " << std::setw(4) << unsigned{sym.st_other};
  os << "shndx: " << std::setw(8) << sym.st_shndx;
  os << "value: " << std::setw(8) << sym.st_value;
  os << "size: " << std::setw(8) << sym.st_size;
  return os;
}

std::ostream& operator<<(std::ostream& os, const Elf64_Sym& sym) {
  os << "name: " << std::setw(8) << sym.st_name;
  os << "info: " << std::setw(4) << unsigned{sym.st_info};
  os << "other: " << std::setw(4) << unsigned{sym.st_other};
  os << "shndx: " << std::setw(8) << sym.st_shndx;
  os << "value: " << std::setw(8) << sym.st_value;
  os << "size: " << std::setw(8) << sym.st_size;
  return os;
}

std::ostream& operator<<(std::ostream& os, const Elf32_Verdef& verdef) {
  os << "version: " << std::setw(4) << verdef.vd_version;
  os << "flags: " << std::setw(4) << verdef.vd_flags;
  os << "ndx: " << std::setw(4) << verdef.vd_ndx;
  os << "cnt: " << std::setw(4) << verdef.vd_cnt;
  os << "hash: " << std::setw(8) << verdef.vd_hash;
  os << "aux: " << std::setw(4) << verdef.vd_aux;
  os << "next: " << std::setw(4) << verdef.vd_next;
  return os;
}

std::ostream& operator<<(std::ostream& os, const Elf64_Verdef& verdef) {
  os << "version: " << std::setw(4) << verdef.vd_version;
  os << "flags: " << std::setw(4) << verdef.vd_flags;
  os << "ndx: " << std::setw(4) << verdef.vd_ndx;
  os << "cnt: " << std::setw(4) << verdef.vd_cnt;
  os << "hash: " << std::setw(8) << verdef.vd_hash;
  os << "aux: " << std::setw(4) << verdef.vd_aux;
  os << "next: " << std::setw(4) << verdef.vd_next;
  return os;
}

std::ostream& operator<<(std::ostream& os, const Elf32_Verneed& verneed) {
  os << "version: " << std::setw(4) << verneed.vn_version;
  os << "cnt: " << std::setw(4) << verneed.vn_cnt;
  os << "file: " << std::setw(6) << verneed.vn_file;
  os << "aux: " << std::setw(4) << verneed.vn_aux;
  os << "next: " << std::setw(4) << verneed.vn_next;
  return os;
}

std::ostream& operator<<(std::ostream& os, const Elf64_Verneed& verneed) {
  os << "version: " << std::setw(4) << verneed.vn_version;
  os << "cnt: " << std::setw(4) << verneed.vn_cnt;
  os << "file: " << std::setw(6) << verneed.vn_file;
  os << "aux: " << std::setw(4) << verneed.vn_aux;
  os << "next: " << std::setw(4) << verneed.vn_next;
  return os;
}

std::ostream& operator<<(std::ostream& os, const Elf32_Vernaux& vernaux) {
  os << "hash: " << std::setw(8) << vernaux.vna_hash;
  os << "flags: " << std::setw(4) << vernaux.vna_flags;
  os << "other: " << std::setw(4) << vernaux.vna_other;
  os << "name: " << std::setw(6) << vernaux.vna_name;
  os << "next: " << std::setw(4) << vernaux.vna_next;
  return os;
}

std::ostream& operator<<(std::ostream& os, const Elf64_Vernaux& vernaux) {
  os << "hash: " << std::setw(8) << vernaux.vna_hash;
  os << "flags: " << std::setw(4) << vernaux.vna_flags;
  os << "other: " << std::setw(4) << vernaux.vna_other;
  os << "name: " << std::setw(6) << vernaux.vna_name;
  os << "next: " << std::setw(4) << vernaux.vna_next;
  return os;
}
