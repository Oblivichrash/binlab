//

#ifndef BINLAB_BINARYFORMAT_COFF_H_
#define BINLAB_BINARYFORMAT_COFF_H_

#include <cstdint>

namespace binlab {
namespace COFF {

//
static constexpr std::uint16_t IMAGE_DOS_SIGNATURE  = 0x5A4D;       // MZ

struct IMAGE_DOS_HEADER {              // DOS .EXE header
  std::uint16_t     e_magic;          // Magic number
  std::uint16_t     e_cblp;           // Bytes on last page of file
  std::uint16_t     e_cp;             // Pages in file
  std::uint16_t     e_crlc;           // Relocations
  std::uint16_t     e_cparhdr;        // Size of header in paragraphs
  std::uint16_t     e_minalloc;       // Minimum extra paragraphs needed
  std::uint16_t     e_maxalloc;       // Maximum extra paragraphs needed
  std::uint16_t     e_ss;             // Initial (relative) SS value
  std::uint16_t     e_sp;             // Initial SP value
  std::uint16_t     e_csum;           // Checksum
  std::uint16_t     e_ip;             // Initial IP value
  std::uint16_t     e_cs;             // Initial (relative) CS value
  std::uint16_t     e_lfarlc;         // File address of relocation table
  std::uint16_t     e_ovno;           // Overlay number
  std::uint16_t     e_res[4];         // Reserved words
  std::uint16_t     e_oemid;          // OEM identifier (for e_oeminfo)
  std::uint16_t     e_oeminfo;        // OEM information; e_oemid specific
  std::uint16_t     e_res2[10];       // Reserved words
  std::uint32_t     e_lfanew;         // File address of new exe header
};

struct IMAGE_FILE_HEADER {
  std::uint16_t     Machine;
  std::uint16_t     NumberOfSections;
  std::uint32_t     TimeDateStamp;
  std::uint32_t     PointerToSymbolTable;
  std::uint32_t     NumberOfSymbols;
  std::uint16_t     SizeOfOptionalHeader;
  std::uint16_t     Characteristics;
};

struct IMAGE_DATA_DIRECTORY {
  std::uint32_t     VirtualAddress;
  std::uint32_t     Size;
};

// Optional header format.
static constexpr std::uint16_t IMAGE_NT_OPTIONAL_HDR32_MAGIC = 0x10b;
static constexpr std::uint16_t IMAGE_NT_OPTIONAL_HDR64_MAGIC = 0x20b;

static constexpr std::size_t IMAGE_NUMBEROF_DIRECTORY_ENTRIES = 16;

struct IMAGE_OPTIONAL_HEADER32 {
  // Standard fields.
  std::uint16_t     Magic;
  std::uint8_t      MajorLinkerVersion;
  std::uint8_t      MinorLinkerVersion;
  std::uint32_t     SizeOfCode;
  std::uint32_t     SizeOfInitializedData;
  std::uint32_t     SizeOfUninitializedData;
  std::uint32_t     AddressOfEntryPoint;
  std::uint32_t     BaseOfCode;
  std::uint32_t     BaseOfData;

  // NT additional fields.
  std::uint32_t     ImageBase;
  std::uint32_t     SectionAlignment;
  std::uint32_t     FileAlignment;
  std::uint16_t     MajorOperatingSystemVersion;
  std::uint16_t     MinorOperatingSystemVersion;
  std::uint16_t     MajorImageVersion;
  std::uint16_t     MinorImageVersion;
  std::uint16_t     MajorSubsystemVersion;
  std::uint16_t     MinorSubsystemVersion;
  std::uint32_t     Win32VersionValue;
  std::uint32_t     SizeOfImage;
  std::uint32_t     SizeOfHeaders;
  std::uint32_t     CheckSum;
  std::uint16_t     Subsystem;
  std::uint16_t     DllCharacteristics;
  std::uint32_t     SizeOfStackReserve;
  std::uint32_t     SizeOfStackCommit;
  std::uint32_t     SizeOfHeapReserve;
  std::uint32_t     SizeOfHeapCommit;
  std::uint32_t     LoaderFlags;
  std::uint32_t     NumberOfRvaAndSizes;
  IMAGE_DATA_DIRECTORY DataDirectory[IMAGE_NUMBEROF_DIRECTORY_ENTRIES];
};

struct IMAGE_OPTIONAL_HEADER64 {
  std::uint16_t     Magic;
  std::uint8_t      MajorLinkerVersion;
  std::uint8_t      MinorLinkerVersion;
  std::uint32_t     SizeOfCode;
  std::uint32_t     SizeOfInitializedData;
  std::uint32_t     SizeOfUninitializedData;
  std::uint32_t     AddressOfEntryPoint;
  std::uint32_t     BaseOfCode;
  std::uint64_t     ImageBase;
  std::uint32_t     SectionAlignment;
  std::uint32_t     FileAlignment;
  std::uint16_t     MajorOperatingSystemVersion;
  std::uint16_t     MinorOperatingSystemVersion;
  std::uint16_t     MajorImageVersion;
  std::uint16_t     MinorImageVersion;
  std::uint16_t     MajorSubsystemVersion;
  std::uint16_t     MinorSubsystemVersion;
  std::uint32_t     Win32VersionValue;
  std::uint32_t     SizeOfImage;
  std::uint32_t     SizeOfHeaders;
  std::uint32_t     CheckSum;
  std::uint16_t     Subsystem;
  std::uint16_t     DllCharacteristics;
  std::uint64_t     SizeOfStackReserve;
  std::uint64_t     SizeOfStackCommit;
  std::uint64_t     SizeOfHeapReserve;
  std::uint64_t     SizeOfHeapCommit;
  std::uint32_t     LoaderFlags;
  std::uint32_t     NumberOfRvaAndSizes;
  IMAGE_DATA_DIRECTORY DataDirectory[IMAGE_NUMBEROF_DIRECTORY_ENTRIES];
};

//
static constexpr std::uint32_t IMAGE_NT_SIGNATURE   = 0x00004550;   // PE00

struct IMAGE_NT_HEADERS64 {
  std::uint32_t           Signature;
  IMAGE_FILE_HEADER       FileHeader;
  IMAGE_OPTIONAL_HEADER64 OptionalHeader;
};

struct IMAGE_NT_HEADERS32 {
  std::uint32_t           Signature;
  IMAGE_FILE_HEADER       FileHeader;
  IMAGE_OPTIONAL_HEADER32 OptionalHeader;
};

// Section header format.
static constexpr std::size_t IMAGE_SIZEOF_SHORT_NAME = 8;

struct IMAGE_SECTION_HEADER {
  std::uint8_t      Name[IMAGE_SIZEOF_SHORT_NAME];
  union {
    std::uint32_t   PhysicalAddress;
    std::uint32_t   VirtualSize;
  } Misc;
  std::uint32_t     VirtualAddress;
  std::uint32_t     SizeOfRawData;
  std::uint32_t     PointerToRawData;
  std::uint32_t     PointerToRelocations;
  std::uint32_t     PointerToLinenumbers;
  std::uint16_t     NumberOfRelocations;
  std::uint16_t     NumberOfLinenumbers;
  std::uint32_t     Characteristics;
};

#define IMAGE_FIRST_SECTION( ntheader ) ((IMAGE_SECTION_HEADER*)    \
  ((std::uint64_t)(ntheader) +                                      \
    offsetof(IMAGE_NT_HEADERS64, OptionalHeader) +                  \
    ((ntheader))->FileHeader.SizeOfOptionalHeader                   \
  ))

}  // namespace COFF
}  // namespace binlab

#endif  // BINLAB_BINARYFORMAT_COFF_H_
