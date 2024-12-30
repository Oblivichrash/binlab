// tools/bindump/coffdump.cpp:

#include "coffdump.h"

#include <cstdio>

#include "binlab/BinaryFormat/COFF.h"

int binlab::COFF::dump_import64(const char *buff, [[maybe_unused]] std::size_t size) {
  auto& Dos = reinterpret_cast<const IMAGE_DOS_HEADER&>(buff[0]);
  if (Dos.e_magic != IMAGE_DOS_SIGNATURE) {
    return 1;
  }

  auto& Nt = reinterpret_cast<const IMAGE_NT_HEADERS64&>(buff[Dos.e_lfanew]);
  if (Nt.Signature != IMAGE_NT_SIGNATURE || Nt.OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR64_MAGIC) {
    return 1;
  }

  std::size_t vaddr = Nt.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;

  auto Sections = IMAGE_FIRST_SECTION(&Nt);
  for (std::size_t i = 0; i < Nt.FileHeader.NumberOfSections; ++i) {
    if (vaddr && (Sections[i].VirtualAddress <= vaddr) && (vaddr < Sections[i].VirtualAddress + Sections[i].Misc.VirtualSize)) {
      const char *base = &buff[Sections[i].PointerToRawData];
      auto descriptors = reinterpret_cast<const IMAGE_IMPORT_DESCRIPTOR*>(&base[vaddr - Sections[i].VirtualAddress]);
      for (std::size_t j = 0; descriptors[j].Name; ++j) {
        std::printf("%s\n", &base[descriptors[j].Name - Sections[i].VirtualAddress]);
        auto thunks = reinterpret_cast<const IMAGE_THUNK_DATA64*>(&base[descriptors->OriginalFirstThunk - Sections[i].VirtualAddress]);
        for (std::size_t k = 0; thunks[k].u1.AddressOfData; ++k) {
          if (!IMAGE_SNAP_BY_ORDINAL64(thunks[k].u1.Ordinal)) {
            auto& name = reinterpret_cast<const IMAGE_IMPORT_BY_NAME&>(base[thunks[k].u1.AddressOfData - Sections[i].VirtualAddress]);
            std::printf("\t%p: %s\n", reinterpret_cast<const void*>(name.Hint), &name.Name[0]);
          } else {
            std::printf("\t%p\n", reinterpret_cast<const void*>(IMAGE_ORDINAL64(thunks[k].u1.Ordinal)));
          }
        }
      }
      break;
    }
  }

  return 0;
}

int binlab::COFF::dump_import32(const char *buff, [[maybe_unused]] std::size_t size) {
  auto& Dos = reinterpret_cast<const IMAGE_DOS_HEADER&>(buff[0]);
  if (Dos.e_magic != IMAGE_DOS_SIGNATURE) {
    return 1;
  }

  auto& Nt = reinterpret_cast<const IMAGE_NT_HEADERS32&>(buff[Dos.e_lfanew]);
  if (Nt.Signature != IMAGE_NT_SIGNATURE || Nt.OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR32_MAGIC) {
    return 1;
  }

  std::size_t vaddr = Nt.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;

  auto Sections = IMAGE_FIRST_SECTION(&Nt);
  for (std::size_t i = 0; i < Nt.FileHeader.NumberOfSections; ++i) {
    if (vaddr && (Sections[i].VirtualAddress <= vaddr) && (vaddr < Sections[i].VirtualAddress + Sections[i].Misc.VirtualSize)) {
      const char *base = &buff[Sections[i].PointerToRawData];
      auto descriptors = reinterpret_cast<const IMAGE_IMPORT_DESCRIPTOR*>(&base[vaddr - Sections[i].VirtualAddress]);
      for (std::size_t j = 0; descriptors[j].Name; ++j) {
        std::printf("%s\n", &base[descriptors[j].Name - Sections[i].VirtualAddress]);
        auto thunks = reinterpret_cast<const IMAGE_THUNK_DATA32*>(&base[descriptors->OriginalFirstThunk - Sections[i].VirtualAddress]);
        for (std::size_t k = 0; thunks[k].u1.AddressOfData; ++k) {
          if (!IMAGE_SNAP_BY_ORDINAL32(thunks[k].u1.Ordinal)) {
            auto& name = reinterpret_cast<const IMAGE_IMPORT_BY_NAME&>(base[thunks[k].u1.AddressOfData - Sections[i].VirtualAddress]);
            std::printf("\t%p: %s\n", reinterpret_cast<const void*>(name.Hint), &name.Name[0]);
          } else {
            std::printf("\t%p\n", reinterpret_cast<const void*>(IMAGE_ORDINAL32(thunks[k].u1.Ordinal)));
          }
        }
      }
      break;
    }
  }

  return 0;
}

int binlab::COFF::dump_obj_reloc(const char* buff, [[maybe_unused]] std::size_t size) {
  auto& FileHeader = reinterpret_cast<const IMAGE_FILE_HEADER&>(buff[0]);
  auto Sections = reinterpret_cast<const IMAGE_SECTION_HEADER*>(&buff[sizeof(FileHeader) + FileHeader.SizeOfOptionalHeader]);
  for (std::size_t i = 0; i < FileHeader.NumberOfSections; ++i) {
    std::size_t offset = Sections[i].PointerToRelocations;
    if (offset) {
      std::printf("%02d %8s\n", i, Sections[i].Name);
      auto relocations = reinterpret_cast<const IMAGE_RELOCATION*>(&buff[offset]);
      for (std::size_t j = 0; j < Sections[i].NumberOfRelocations; ++j) {
        std::printf("address: %08x index: %08x type: %08x\n", relocations[j].VirtualAddress, relocations[j].SymbolTableIndex, relocations[j].Type);
      }
    }
  }
  return 0;
}

int binlab::COFF::dump_obj_sym(const char* buff, [[maybe_unused]] std::size_t size) {
  auto& FileHeader = reinterpret_cast<const IMAGE_FILE_HEADER&>(buff[0]);
  auto symbols = reinterpret_cast<const IMAGE_SYMBOL*>(&buff[FileHeader.PointerToSymbolTable]);
  auto table = reinterpret_cast<const char*>(symbols + FileHeader.NumberOfSymbols);
  for (std::size_t i = 0; i < FileHeader.NumberOfSymbols; ++i) {
    if (symbols[i].N.Name.Short) {
      std::printf("%8s\n", symbols[i].N.ShortName);
    } else {
      std::printf("%s\n", table + symbols[i].N.Name.Long);
    }
  }
  return 0;
}
