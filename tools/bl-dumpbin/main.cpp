//

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <locale>
#include <vector>

#include "binlab/Config.h"
#include "binlab/BinaryFormat/COFF.h"
#include "binlab/BinaryFormat/ELF.h"

using namespace binlab::COFF;
using namespace binlab::ELF;

int dump(const char* base, const std::size_t off, const std::size_t size) {
  auto data = base + off;
  constexpr std::size_t block = 16;
  const char* fmt = "%-48s  %-s\n";  // block * 3 = 48

  for (std::size_t i = 0; i < size; i += block) {
    char hex[3 * block + 1] = {0}, str[1 * block + 1] = {0};

    for (std::size_t j = 0, pos = 0; (j < block) && ((i + j) < size); ++j) {
      auto count = std::snprintf(&hex[pos], sizeof(hex) - pos, " %02x", static_cast<std::uint8_t>(data[i + j]));
      if (count < 0 || (sizeof(hex) - pos - 1) < count) {
        return -1;
      }
      pos += count;
    }

    for (std::size_t j = 0, pos = 0; (j < block) && ((i + j) < size); ++j) {
      auto count = std::snprintf(&str[pos], sizeof(str) - pos, "%c", (std::isprint(static_cast<std::uint8_t>(data[i + j])) ? data[i + j] : ' '));
      if (count < 0 || (sizeof(hex) - pos - 1) < count) {
        return -1;
      }
      pos += count;
    }

    std::printf(fmt, hex, str);
  }
  return 0;
}

int dump64(const char* base, const std::size_t off, const std::size_t va, const IMAGE_IMPORT_DESCRIPTOR* descriptoies) {
  const std::ptrdiff_t delta = reinterpret_cast<std::size_t>(base) + off - va;
  auto print_thunks = [delta](const IMAGE_IMPORT_DESCRIPTOR& descriptor) {
    for (auto thunks = reinterpret_cast<const IMAGE_THUNK_DATA64*>(descriptor.OriginalFirstThunk + delta); thunks->u1.AddressOfData; ++thunks) {
      if (!IMAGE_SNAP_BY_ORDINAL64(thunks->u1.Ordinal)) {
        auto name = reinterpret_cast<const IMAGE_IMPORT_BY_NAME*>(thunks->u1.AddressOfData + delta);
        std::printf("\t%04x: %s\n", name->Hint, &name->Name[0]);
      } else {
        std::printf("\t%p\n", reinterpret_cast<const void*>(IMAGE_ORDINAL64(thunks->u1.Ordinal)));
      }
    }
  };

  for (auto iter = descriptoies; iter->Name; ++iter) {
    std::printf("%s\n", reinterpret_cast<char*>(iter->Name + delta));
    print_thunks(*iter);
  }
  return 0;
}

int dump64(const char* base, const std::size_t off, const std::size_t va, const IMAGE_EXPORT_DIRECTORY* directories) {
  const std::ptrdiff_t delta = reinterpret_cast<std::size_t>(base) + off - va;
  std::printf("Name: %s\n", reinterpret_cast<const char*>(directories->Name + delta));
  std::printf("Base: %08x\n", directories->Base);

  auto functions = reinterpret_cast<const DWORD*>(directories->AddressOfFunctions + delta);
  auto names = reinterpret_cast<const DWORD*>(directories->AddressOfNames + delta);
  auto ordinals = reinterpret_cast<const WORD*>(directories->AddressOfNameOrdinals + delta);
  for (std::size_t i = 0; i < directories->NumberOfFunctions; ++i, ++functions, ++names, ++ordinals) {
    std::printf("\t%08x", *functions);
    std::printf("\t%04x", *ordinals);
    std::printf("\t%s\n", reinterpret_cast<const char*>(*names + delta));
  }
  return 0;
}

int dump_pe64(const char* buff) {
  auto& Dos = reinterpret_cast<const IMAGE_DOS_HEADER&>(buff[0]);
  if (Dos.e_magic == IMAGE_DOS_SIGNATURE) {
    auto& Nt = reinterpret_cast<const IMAGE_NT_HEADERS64&>(buff[Dos.e_lfanew]);
    if (Nt.Signature == IMAGE_NT_SIGNATURE && Nt.OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC) {
      auto Sections = IMAGE_FIRST_SECTION(&Nt);
      std::size_t va0 = Nt.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
      if (va0) {
        for (std::size_t i = 0; i < Nt.FileHeader.NumberOfSections; ++i) {
          if ((Sections[i].VirtualAddress <= va0) && (va0 < Sections[i].VirtualAddress + Sections[i].Misc.VirtualSize)) {
            dump64(buff, Sections[i].PointerToRawData, Sections[i].VirtualAddress, reinterpret_cast<const IMAGE_EXPORT_DIRECTORY*>(&buff[Sections[i].PointerToRawData + va0 - Sections[i].VirtualAddress]));
            break;
          }
        }
      }

      std::size_t va1 = Nt.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
      if (va1) {
        for (std::size_t i = 0; i < Nt.FileHeader.NumberOfSections; ++i) {
          if ((Sections[i].VirtualAddress <= va1) && (va1 < Sections[i].VirtualAddress + Sections[i].Misc.VirtualSize)) {
            const std::ptrdiff_t delta = reinterpret_cast<std::size_t>(buff) + Sections[i].PointerToRawData - Sections[i].VirtualAddress;
            dump64(buff, Sections[i].PointerToRawData, Sections[i].VirtualAddress, reinterpret_cast<const IMAGE_IMPORT_DESCRIPTOR*>(&buff[Sections[i].PointerToRawData + va1 - Sections[i].VirtualAddress]));
            break;
          }
        }
      }
    }
  }
  return 0;
}

#ifdef _WIN32
using WCHAR       = wchar_t;
#else  // __GNU__
using WCHAR       = char16_t;
#endif  // _WIN32

struct IMAGE_RESOURCE_DIRECTORY_STRING {
  WORD    Length;
  CHAR    NameString[1];
};

struct IMAGE_RESOURCE_DIR_STRING_U {
  WORD    Length;
  WCHAR   NameString[1];
};

#if defined(unix) || defined(__unix__) || defined(__unix)
#include <unistd.h>

#if defined(_POSIX_VERSION) && (_POSIX_VERSION >= 200809L)
#include <iconv.h>

int dump(const IMAGE_RESOURCE_DIR_STRING_U& string) {
  int result = 0;
  auto cd = iconv_open("UTF-8", "UTF-16LE");
  if (cd != reinterpret_cast<iconv_t>(-1)) {
    char buff[1024] = {0};
    auto inbuf = const_cast<char*>(reinterpret_cast<const char*>(string.NameString));
    std::size_t inbytesleft = string.Length * sizeof(string.NameString[0]);
    auto outbuf = buff;
    std::size_t outbytesleft = sizeof(buff);
    if (iconv(cd, &inbuf, &inbytesleft, &outbuf, &outbytesleft) != static_cast<std::size_t>(-1)) {
      std::printf("%s\n", buff);
    }
    result = iconv_close(cd);
  }
  return result;
}
#endif  // !_POSIX_VERSION
#else
int dump(const IMAGE_RESOURCE_DIR_STRING_U& string) {
  using char_type = std::decay_t<decltype(string.NameString[0])>;
  std::locale loc;
  std::string name;
  std::use_facet<std::ctype<char_type>>(loc).narrow(string.NameString, string.NameString + string.Length, '.', name.data());

  std::printf("%s\n", name.c_str());
  return 0;
}
#endif  // !unix

int dump(const char* base, const std::size_t off, const std::size_t va, const IMAGE_RESOURCE_DIRECTORY* directories) {
  int result = 0;
  auto entries = reinterpret_cast<const IMAGE_RESOURCE_DIRECTORY_ENTRY*>(directories + 1);
  for (std::size_t i = 0; i < directories->NumberOfIdEntries + directories->NumberOfNamedEntries; ++i) {
    if (entries[i].NameIsString) {
      if (result = dump(reinterpret_cast<const IMAGE_RESOURCE_DIR_STRING_U&>(base[off + entries[i].NameOffset]))) {
        break;
      }
    } else {
      std::printf("%d\n", entries[i].Id);
    }

    if (entries[i].DataIsDirectory) {
      if (result = dump(base, off, va, reinterpret_cast<const IMAGE_RESOURCE_DIRECTORY*>(&base[off + entries[i].OffsetToDirectory]))) {
        break;
      }
    } else {
      auto& data = reinterpret_cast<const IMAGE_RESOURCE_DATA_ENTRY&>(base[off + entries[i].OffsetToData]);
      std::printf("[%p, %p), offset: %8x, size: %8x, code page: %8x, reserved: %8x\n", reinterpret_cast<void*>(off + data.OffsetToData - va), reinterpret_cast<void*>(off + data.OffsetToData - va + data.Size), data.OffsetToData, data.Size, data.CodePage, data.Reserved);
    }
  }
  return result;
}

int dump_pe32(const char* buff) {
  auto& Dos = reinterpret_cast<const IMAGE_DOS_HEADER&>(buff[0]);
  if (Dos.e_magic == IMAGE_DOS_SIGNATURE) {
    auto& Nt = reinterpret_cast<const IMAGE_NT_HEADERS32&>(buff[Dos.e_lfanew]);
    if (Nt.Signature == IMAGE_NT_SIGNATURE && Nt.OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC) {
      auto Sections = IMAGE_FIRST_SECTION(&Nt);
      std::size_t va2 = Nt.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress;
      if (va2) {
        for (std::size_t i = 0; i < Nt.FileHeader.NumberOfSections; ++i) {
          if ((Sections[i].VirtualAddress <= va2) && (va2 < Sections[i].VirtualAddress + Sections[i].Misc.VirtualSize)) {
            std::printf("pointer to raw data: %x\n", Sections[i].PointerToRawData);
            const std::ptrdiff_t delta = reinterpret_cast<std::size_t>(buff) + Sections[i].PointerToRawData - Sections[i].VirtualAddress;
            dump(buff, Sections[i].PointerToRawData, Sections[i].VirtualAddress, reinterpret_cast<const IMAGE_RESOURCE_DIRECTORY*>(&buff[Sections[i].PointerToRawData + va2 - Sections[i].VirtualAddress]));
            break;
          }
        }
      }
    }
  }
  return 0;
}

int dump_obj64(const char* buff) {
  auto& FileHeader = reinterpret_cast<const IMAGE_FILE_HEADER&>(buff[0]);
  std::printf("NumberOfSections: %d\n", FileHeader.NumberOfSections);
  auto Sections = reinterpret_cast<const IMAGE_SECTION_HEADER*>(&buff[sizeof(FileHeader) + FileHeader.SizeOfOptionalHeader]);
  for (std::size_t i = 0; i < FileHeader.NumberOfSections; ++i) {
    std::printf("%8s\n", Sections[i].Name);
  }
  return 0;
}

int dump_obj_sym(const char* buff) {
  std::size_t addr = reinterpret_cast<std::size_t>(buff);

  auto& FileHeader = reinterpret_cast<const IMAGE_FILE_HEADER&>(buff[0]);
  auto symbols = reinterpret_cast<const IMAGE_SYMBOL*>(addr + FileHeader.PointerToSymbolTable);
  auto table = reinterpret_cast<const char*>(symbols + FileHeader.NumberOfSymbols);
  for (std::size_t i = 0; i < FileHeader.NumberOfSymbols; ++i) {
    std::printf("%-08x %-04x %-04x %-02x", symbols[i].Value, static_cast<std::uint16_t>(symbols[i].SectionNumber), symbols[i].Type, symbols[i].StorageClass);
    if (symbols[i].N.Name.Short) {
      char name[sizeof(IMAGE_SYMBOL::N.ShortName) + 1] = {0};
      std::strncpy(name, reinterpret_cast<const char*>(symbols[i].N.ShortName), sizeof(IMAGE_SYMBOL::N.ShortName));
      std::printf("%8s\n", name);
    } else {
      std::printf("%s\n", table + symbols[i].N.Name.Long);
    }
  }
  return 0;
}

int dump_elf64le(const char* buff) {
  if (!std::memcmp(buff, ELFMAG, SELFMAG) && buff[EI_CLASS] == ELFCLASS64) {
    auto& ehdr = reinterpret_cast<const Elf64_Ehdr&>(buff[0]);
    auto shdr = reinterpret_cast<const Elf64_Shdr*>(&buff[ehdr.e_shoff]);

    auto shstr = &buff[shdr[ehdr.e_shstrndx].sh_offset];
    for (auto iter = shdr; iter != shdr + ehdr.e_shnum; ++iter) {
      std::printf("%s\n", &shstr[iter->sh_name]);
    }
  }
  return 0;
}

int main(int argc, char* argv[]) {
  if (argc < 2) {
    std::printf("%s ver: %d.%d\n", argv[0], BINLAB_VERSION_MAJOR, BINLAB_VERSION_MINOR);
    return 0;
  }

  std::ifstream is{argv[1], std::ios::binary | std::ios::ate};
  if (is) {
    std::printf("dump %s\n", argv[1]);
    const auto& count = is.tellg();
    if (count) {
      std::vector<char> buff(count);
      if (is.seekg(0, std::ios::beg).read(&buff[0], count)) {
        dump_pe64(&buff[0]);
        dump_pe32(&buff[0]);
        dump_elf64le(&buff[0]);
      }
    }
  }
  return 0;
}
