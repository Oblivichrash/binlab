//

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <vector>

#include "binlab/Config.h"
#include "binlab/BinaryFormat/COFF.h"

using namespace binlab::COFF;

int dump_data(const std::size_t va, const std::size_t size, const std::ptrdiff_t delta) {
  auto data = reinterpret_cast<const char*>(va + delta);
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
      auto count = std::snprintf(&str[pos], sizeof(str) - pos, "%c", (std::isprint(data[i + j]) ? data[i + j] : ' '));
      if (count < 0 || (sizeof(hex) - pos - 1) < count) {
        return -1;
      }
      pos += count;
    }

    std::printf(fmt, hex, str);
  }
  return 0;
}

int dump_import64(const std::size_t va , const std::size_t size, const std::ptrdiff_t delta) {
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

  for (auto descriptors = reinterpret_cast<const IMAGE_IMPORT_DESCRIPTOR*>(va + delta); descriptors->Name; ++descriptors) {
    std::printf("%s\n", reinterpret_cast<char*>(descriptors->Name + delta));
    print_thunks(*descriptors);
  }
  return 0;
}

int dump_export(const std::size_t va , const std::size_t size, const std::ptrdiff_t delta) {
  auto& directory = *reinterpret_cast<const IMAGE_EXPORT_DIRECTORY*>(va + delta);
  std::printf("Name: %s\n", reinterpret_cast<const char*>(directory.Name + delta));
  std::printf("Base: %08x\n", directory.Base);

  auto functions = reinterpret_cast<const DWORD*>(directory.AddressOfFunctions + delta); 
  auto names = reinterpret_cast<const DWORD*>(directory.AddressOfNames + delta); 
  auto ordinals = reinterpret_cast<const WORD*>(directory.AddressOfNameOrdinals + delta); 
  for (std::size_t i = 0; i < directory.NumberOfFunctions; ++i, ++functions, ++names, ++ordinals) {
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
      //std::size_t va = Nt.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
      std::size_t va = Nt.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
      for (std::size_t i = 0; i < Nt.FileHeader.NumberOfSections; ++i) {
        if (va && (Sections[i].VirtualAddress <= va) && (va < Sections[i].VirtualAddress + Sections[i].Misc.VirtualSize)) {
          const std::ptrdiff_t delta = reinterpret_cast<std::size_t>(buff) + Sections[i].PointerToRawData - Sections[i].VirtualAddress;
          //dump_import64(va, Nt.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size, delta);
          dump_export(va, Nt.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size, delta);
          break;
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
        dump_obj_sym(&buff[0]);
      }
    }
  }
  return 0;
}
