//

#include <elf.h>

#include <iosfwd>

std::ostream& operator<<(std::ostream& os, const Elf32_Sym& sym);
std::ostream& operator<<(std::ostream& os, const Elf64_Sym& sym);

std::ostream& operator<<(std::ostream& os, const Elf32_Verdef& verdef);
std::ostream& operator<<(std::ostream& os, const Elf64_Verdef& verdef);

std::ostream& operator<<(std::ostream& os, const Elf32_Verneed& verneed);
std::ostream& operator<<(std::ostream& os, const Elf64_Verneed& verneed);

std::ostream& operator<<(std::ostream& os, const Elf32_Vernaux& vernaux);
std::ostream& operator<<(std::ostream& os, const Elf64_Vernaux& vernaux);
