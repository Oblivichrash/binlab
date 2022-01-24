// binlab/BinaryFormat/COFF.h: definitions for Windows COFF Files

#ifndef BINLAB_BINARYFORMAT_COFF_H_
#define BINLAB_BINARYFORMAT_COFF_H_

#include <cstddef>
#include <cstdint>

namespace binlab {
namespace COFF {

using BYTE        = std::uint8_t;
using WORD        = std::uint16_t;
using LONG        = std::int32_t;
using DWORD       = std::uint32_t;
using ULONGLONG   = std::uint64_t;
using ULONG_PTR   = std::uint64_t;

static constexpr std::uint16_t IMAGE_DOS_SIGNATURE  = 0x5A4D;       // MZ
static constexpr std::uint32_t IMAGE_NT_SIGNATURE   = 0x00004550;   // PE00

struct IMAGE_DOS_HEADER {     // DOS .EXE header
  WORD      e_magic;          // Magic number
  WORD      e_cblp;           // Bytes on last page of file
  WORD      e_cp;             // Pages in file
  WORD      e_crlc;           // Relocations
  WORD      e_cparhdr;        // Size of header in paragraphs
  WORD      e_minalloc;       // Minimum extra paragraphs needed
  WORD      e_maxalloc;       // Maximum extra paragraphs needed
  WORD      e_ss;             // Initial (relative) SS value
  WORD      e_sp;             // Initial SP value
  WORD      e_csum;           // Checksum
  WORD      e_ip;             // Initial IP value
  WORD      e_cs;             // Initial (relative) CS value
  WORD      e_lfarlc;         // File address of relocation table
  WORD      e_ovno;           // Overlay number
  WORD      e_res[4];         // Reserved words
  WORD      e_oemid;          // OEM identifier (for e_oeminfo)
  WORD      e_oeminfo;        // OEM information; e_oemid specific
  WORD      e_res2[10];       // Reserved words
  LONG      e_lfanew;         // File address of new exe header
};

// File header format.
struct IMAGE_FILE_HEADER {
  WORD      Machine;
  WORD      NumberOfSections;
  DWORD     TimeDateStamp;
  DWORD     PointerToSymbolTable;
  DWORD     NumberOfSymbols;
  WORD      SizeOfOptionalHeader;
  WORD      Characteristics;
};

static constexpr std::size_t IMAGE_SIZEOF_FILE_HEADER = 20;

enum Characteristics {
  IMAGE_FILE_RELOCS_STRIPPED           = 0x0001,  // Relocation info stripped from file.
  IMAGE_FILE_EXECUTABLE_IMAGE          = 0x0002,  // File is executable  (i.e. no unresolved external references).
  IMAGE_FILE_LINE_NUMS_STRIPPED        = 0x0004,  // Line nunbers stripped from file.
  IMAGE_FILE_LOCAL_SYMS_STRIPPED       = 0x0008,  // Local symbols stripped from file.
  IMAGE_FILE_AGGRESIVE_WS_TRIM         = 0x0010,  // Aggressively trim working set
  IMAGE_FILE_LARGE_ADDRESS_AWARE       = 0x0020,  // App can handle >2gb addresses
  IMAGE_FILE_BYTES_REVERSED_LO         = 0x0080,  // Bytes of machine word are reversed.
  IMAGE_FILE_32BIT_MACHINE             = 0x0100,  // 32 bit word machine.
  IMAGE_FILE_DEBUG_STRIPPED            = 0x0200,  // Debugging info stripped from file in .DBG file
  IMAGE_FILE_REMOVABLE_RUN_FROM_SWAP   = 0x0400,  // If Image is on removable media, copy and run from the swap file.
  IMAGE_FILE_NET_RUN_FROM_SWAP         = 0x0800,  // If Image is on Net, copy and run from the swap file.
  IMAGE_FILE_SYSTEM                    = 0x1000,  // System File.
  IMAGE_FILE_DLL                       = 0x2000,  // File is a DLL.
  IMAGE_FILE_UP_SYSTEM_ONLY            = 0x4000,  // File should only be run on a UP machine
  IMAGE_FILE_BYTES_REVERSED_HI         = 0x8000   // Bytes of machine word are reversed.
};

enum MachineTypes {
  IMAGE_FILE_MACHINE_UNKNOWN           = 0,
  IMAGE_FILE_MACHINE_TARGET_HOST       = 0x0001,  // Useful for indicating we want to interact with the host and not a WoW guest.
  IMAGE_FILE_MACHINE_I386              = 0x014c,  // Intel 386.
  IMAGE_FILE_MACHINE_R3000             = 0x0162,  // MIPS little-endian, 0x160 big-endian
  IMAGE_FILE_MACHINE_R4000             = 0x0166,  // MIPS little-endian
  IMAGE_FILE_MACHINE_R10000            = 0x0168,  // MIPS little-endian
  IMAGE_FILE_MACHINE_WCEMIPSV2         = 0x0169,  // MIPS little-endian WCE v2
  IMAGE_FILE_MACHINE_ALPHA             = 0x0184,  // Alpha_AXP
  IMAGE_FILE_MACHINE_SH3               = 0x01a2,  // SH3 little-endian
  IMAGE_FILE_MACHINE_SH3DSP            = 0x01a3,
  IMAGE_FILE_MACHINE_SH3E              = 0x01a4,  // SH3E little-endian
  IMAGE_FILE_MACHINE_SH4               = 0x01a6,  // SH4 little-endian
  IMAGE_FILE_MACHINE_SH5               = 0x01a8,  // SH5
  IMAGE_FILE_MACHINE_ARM               = 0x01c0,  // ARM Little-Endian
  IMAGE_FILE_MACHINE_THUMB             = 0x01c2,  // ARM Thumb/Thumb-2 Little-Endian
  IMAGE_FILE_MACHINE_ARMNT             = 0x01c4,  // ARM Thumb-2 Little-Endian
  IMAGE_FILE_MACHINE_AM33              = 0x01d3,
  IMAGE_FILE_MACHINE_POWERPC           = 0x01F0,  // IBM PowerPC Little-Endian
  IMAGE_FILE_MACHINE_POWERPCFP         = 0x01f1,
  IMAGE_FILE_MACHINE_IA64              = 0x0200,  // Intel 64
  IMAGE_FILE_MACHINE_MIPS16            = 0x0266,  // MIPS
  IMAGE_FILE_MACHINE_ALPHA64           = 0x0284,  // ALPHA64
  IMAGE_FILE_MACHINE_MIPSFPU           = 0x0366,  // MIPS
  IMAGE_FILE_MACHINE_MIPSFPU16         = 0x0466,  // MIPS
  IMAGE_FILE_MACHINE_AXP64             = IMAGE_FILE_MACHINE_ALPHA64,
  IMAGE_FILE_MACHINE_TRICORE           = 0x0520,  // Infineon
  IMAGE_FILE_MACHINE_CEF               = 0x0CEF,
  IMAGE_FILE_MACHINE_EBC               = 0x0EBC,  // EFI Byte Code
  IMAGE_FILE_MACHINE_AMD64             = 0x8664,  // AMD64 (K8)
  IMAGE_FILE_MACHINE_M32R              = 0x9041,  // M32R little-endian
  IMAGE_FILE_MACHINE_ARM64             = 0xAA64,  // ARM64 Little-Endian
  IMAGE_FILE_MACHINE_CEE               = 0xC0EE
};

// Directory format.
struct IMAGE_DATA_DIRECTORY {
  DWORD     VirtualAddress;
  DWORD     Size;
};

static constexpr std::size_t IMAGE_NUMBEROF_DIRECTORY_ENTRIES = 16;

// Optional header format.
struct IMAGE_OPTIONAL_HEADER32 {
  // Standard fields.
  WORD      Magic;
  BYTE      MajorLinkerVersion;
  BYTE      MinorLinkerVersion;
  DWORD     SizeOfCode;
  DWORD     SizeOfInitializedData;
  DWORD     SizeOfUninitializedData;
  DWORD     AddressOfEntryPoint;
  DWORD     BaseOfCode;
  DWORD     BaseOfData;

  // NT additional fields.
  DWORD     ImageBase;
  DWORD     SectionAlignment;
  DWORD     FileAlignment;
  WORD      MajorOperatingSystemVersion;
  WORD      MinorOperatingSystemVersion;
  WORD      MajorImageVersion;
  WORD      MinorImageVersion;
  WORD      MajorSubsystemVersion;
  WORD      MinorSubsystemVersion;
  DWORD     Win32VersionValue;
  DWORD     SizeOfImage;
  DWORD     SizeOfHeaders;
  DWORD     CheckSum;
  WORD      Subsystem;
  WORD      DllCharacteristics;
  DWORD     SizeOfStackReserve;
  DWORD     SizeOfStackCommit;
  DWORD     SizeOfHeapReserve;
  DWORD     SizeOfHeapCommit;
  DWORD     LoaderFlags;
  DWORD     NumberOfRvaAndSizes;
  IMAGE_DATA_DIRECTORY DataDirectory[IMAGE_NUMBEROF_DIRECTORY_ENTRIES];
};

struct IMAGE_OPTIONAL_HEADER64 {
  WORD      Magic;
  BYTE      MajorLinkerVersion;
  BYTE      MinorLinkerVersion;
  DWORD     SizeOfCode;
  DWORD     SizeOfInitializedData;
  DWORD     SizeOfUninitializedData;
  DWORD     AddressOfEntryPoint;
  DWORD     BaseOfCode;
  ULONGLONG ImageBase;
  DWORD     SectionAlignment;
  DWORD     FileAlignment;
  WORD      MajorOperatingSystemVersion;
  WORD      MinorOperatingSystemVersion;
  WORD      MajorImageVersion;
  WORD      MinorImageVersion;
  WORD      MajorSubsystemVersion;
  WORD      MinorSubsystemVersion;
  DWORD     Win32VersionValue;
  DWORD     SizeOfImage;
  DWORD     SizeOfHeaders;
  DWORD     CheckSum;
  WORD      Subsystem;
  WORD      DllCharacteristics;
  ULONGLONG SizeOfStackReserve;
  ULONGLONG SizeOfStackCommit;
  ULONGLONG SizeOfHeapReserve;
  ULONGLONG SizeOfHeapCommit;
  DWORD     LoaderFlags;
  DWORD     NumberOfRvaAndSizes;
  IMAGE_DATA_DIRECTORY DataDirectory[IMAGE_NUMBEROF_DIRECTORY_ENTRIES];
};

static constexpr std::uint16_t IMAGE_NT_OPTIONAL_HDR32_MAGIC = 0x10b;
static constexpr std::uint16_t IMAGE_NT_OPTIONAL_HDR64_MAGIC = 0x20b;

struct IMAGE_NT_HEADERS64 {
  DWORD Signature;
  IMAGE_FILE_HEADER FileHeader;
  IMAGE_OPTIONAL_HEADER64 OptionalHeader;
};

struct IMAGE_NT_HEADERS32 {
  DWORD Signature;
  IMAGE_FILE_HEADER FileHeader;
  IMAGE_OPTIONAL_HEADER32 OptionalHeader;
};

// IMAGE_FIRST_SECTION doesn't need 32/64 versions since the file header is the same either way.
#define IMAGE_FIRST_SECTION( ntheader ) ((IMAGE_SECTION_HEADER*)      \
    ((ULONG_PTR)(ntheader) +                                          \
     offsetof(IMAGE_NT_HEADERS64, OptionalHeader) +                   \
     ((ntheader))->FileHeader.SizeOfOptionalHeader   \
    ))
static_assert(offsetof(IMAGE_NT_HEADERS32, OptionalHeader) == offsetof(IMAGE_NT_HEADERS64, OptionalHeader), "");

// Subsystem Values
enum WindowsSubsystem {
  IMAGE_SUBSYSTEM_UNKNOWN                 = 0,   // Unknown subsystem.
  IMAGE_SUBSYSTEM_NATIVE                  = 1,   // Image doesn't require a subsystem.
  IMAGE_SUBSYSTEM_WINDOWS_GUI             = 2,   // Image runs in the Windows GUI subsystem.
  IMAGE_SUBSYSTEM_WINDOWS_CUI             = 3,   // Image runs in the Windows character subsystem.
  IMAGE_SUBSYSTEM_OS2_CUI                 = 5,   // image runs in the OS/2 character subsystem.
  IMAGE_SUBSYSTEM_POSIX_CUI               = 7,   // image runs in the Posix character subsystem.
  IMAGE_SUBSYSTEM_NATIVE_WINDOWS          = 8,   // image is a native Win9x driver.
  IMAGE_SUBSYSTEM_WINDOWS_CE_GUI          = 9,   // Image runs in the Windows CE subsystem.
  IMAGE_SUBSYSTEM_EFI_APPLICATION         = 10,
  IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER = 11,
  IMAGE_SUBSYSTEM_EFI_RUNTIME_DRIVER      = 12,
  IMAGE_SUBSYSTEM_EFI_ROM                 = 13,
  IMAGE_SUBSYSTEM_XBOX                    = 14,
  IMAGE_SUBSYSTEM_WINDOWS_BOOT_APPLICATION = 16,
  IMAGE_SUBSYSTEM_XBOX_CODE_CATALOG       = 17
};

// DllCharacteristics Entries
enum DLLCharacteristics {
//IMAGE_LIBRARY_PROCESS_INIT                  = 0x0001,     // Reserved.
//IMAGE_LIBRARY_PROCESS_TERM                  = 0x0002,     // Reserved.
//IMAGE_LIBRARY_THREAD_INIT                   = 0x0004,     // Reserved.
//IMAGE_LIBRARY_THREAD_TERM                   = 0x0008,     // Reserved.
  IMAGE_DLLCHARACTERISTICS_HIGH_ENTROPY_VA    = 0x0020,     // Image can handle a high entropy 64-bit virtual address space.
  IMAGE_DLLCHARACTERISTICS_DYNAMIC_BASE       = 0x0040,     // DLL can move.
  IMAGE_DLLCHARACTERISTICS_FORCE_INTEGRITY    = 0x0080,     // Code Integrity Image
  IMAGE_DLLCHARACTERISTICS_NX_COMPAT          = 0x0100,     // Image is NX compatible
  IMAGE_DLLCHARACTERISTICS_NO_ISOLATION       = 0x0200,     // Image understands isolation and doesn't want it
  IMAGE_DLLCHARACTERISTICS_NO_SEH             = 0x0400,     // Image does not use SEH.  No SE handler may reside in this image
  IMAGE_DLLCHARACTERISTICS_NO_BIND            = 0x0800,     // Do not bind this image.
  IMAGE_DLLCHARACTERISTICS_APPCONTAINER       = 0x1000,     // Image should execute in an AppContainer
  IMAGE_DLLCHARACTERISTICS_WDM_DRIVER         = 0x2000,     // Driver uses WDM model
  IMAGE_DLLCHARACTERISTICS_GUARD_CF           = 0x4000,     // Image supports Control Flow Guard.
  IMAGE_DLLCHARACTERISTICS_TERMINAL_SERVER_AWARE = 0x8000
};

// Directory Entries
enum DirectoryEntries {
  IMAGE_DIRECTORY_ENTRY_EXPORT          = 0,    // Export Directory
  IMAGE_DIRECTORY_ENTRY_IMPORT          = 1,    // Import Directory
  IMAGE_DIRECTORY_ENTRY_RESOURCE        = 2,    // Resource Directory
  IMAGE_DIRECTORY_ENTRY_EXCEPTION       = 3,    // Exception Directory
  IMAGE_DIRECTORY_ENTRY_SECURITY        = 4,    // Security Directory
  IMAGE_DIRECTORY_ENTRY_BASERELOC       = 5,    // Base Relocation Table
  IMAGE_DIRECTORY_ENTRY_DEBUG           = 6,    // Debug Directory
//IMAGE_DIRECTORY_ENTRY_COPYRIGHT       = 7,    // (X86 usage)
  IMAGE_DIRECTORY_ENTRY_ARCHITECTURE    = 7,    // Architecture Specific Data
  IMAGE_DIRECTORY_ENTRY_GLOBALPTR       = 8,    // RVA of GP
  IMAGE_DIRECTORY_ENTRY_TLS             = 9,    // TLS Directory
  IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG     = 10,   // Load Configuration Directory
  IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT    = 11,   // Bound Import Directory in headers
  IMAGE_DIRECTORY_ENTRY_IAT             = 12,   // Import Address Table
  IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT    = 13,   // Delay Load Import Descriptors
  IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR  = 14,   // COM Runtime descriptor
};

// Section header format.
static constexpr std::size_t IMAGE_SIZEOF_SHORT_NAME = 8;

struct IMAGE_SECTION_HEADER {
  BYTE    Name[IMAGE_SIZEOF_SHORT_NAME];
  union {
    DWORD   PhysicalAddress;
    DWORD   VirtualSize;
  } Misc;
  DWORD   VirtualAddress;
  DWORD   SizeOfRawData;
  DWORD   PointerToRawData;
  DWORD   PointerToRelocations;
  DWORD   PointerToLinenumbers;
  WORD    NumberOfRelocations;
  WORD    NumberOfLinenumbers;
  DWORD   Characteristics;
};

static constexpr std::size_t IMAGE_SIZEOF_SECTION_HEADER = 40;

// Section characteristics.
enum SectionCharacteristics {
//IMAGE_SCN_TYPE_REG                   = 0x00000000,  // Reserved.
//IMAGE_SCN_TYPE_DSECT                 = 0x00000001,  // Reserved.
//IMAGE_SCN_TYPE_NOLOAD                = 0x00000002,  // Reserved.
//IMAGE_SCN_TYPE_GROUP                 = 0x00000004,  // Reserved.
  IMAGE_SCN_TYPE_NO_PAD                = 0x00000008,  // Reserved.
//IMAGE_SCN_TYPE_COPY                  = 0x00000010,  // Reserved.

  IMAGE_SCN_CNT_CODE                   = 0x00000020,  // Section contains code.
  IMAGE_SCN_CNT_INITIALIZED_DATA       = 0x00000040,  // Section contains initialized data.
  IMAGE_SCN_CNT_UNINITIALIZED_DATA     = 0x00000080,  // Section contains uninitialized data.

  IMAGE_SCN_LNK_OTHER                  = 0x00000100,  // Reserved.
  IMAGE_SCN_LNK_INFO                   = 0x00000200,  // Section contains comments or some other type of information.
//IMAGE_SCN_TYPE_OVER                  = 0x00000400,  // Reserved.
  IMAGE_SCN_LNK_REMOVE                 = 0x00000800,  // Section contents will not become part of image.
  IMAGE_SCN_LNK_COMDAT                 = 0x00001000,  // Section contents comdat.
//                                     = 0x00002000,  // Reserved.
//IMAGE_SCN_MEM_PROTECTED - Obsolete   = 0x00004000,
  IMAGE_SCN_NO_DEFER_SPEC_EXC          = 0x00004000,  // Reset speculative exceptions handling bits in the TLB entries for this section.
  IMAGE_SCN_GPREL                      = 0x00008000,  // Section content can be accessed relative to GP
  IMAGE_SCN_MEM_FARDATA                = 0x00008000,
//IMAGE_SCN_MEM_SYSHEAP  - Obsolete    = 0x00010000,
  IMAGE_SCN_MEM_PURGEABLE              = 0x00020000,
  IMAGE_SCN_MEM_16BIT                  = 0x00020000,
  IMAGE_SCN_MEM_LOCKED                 = 0x00040000,
  IMAGE_SCN_MEM_PRELOAD                = 0x00080000,

  IMAGE_SCN_ALIGN_1BYTES               = 0x00100000,  //
  IMAGE_SCN_ALIGN_2BYTES               = 0x00200000,  //
  IMAGE_SCN_ALIGN_4BYTES               = 0x00300000,  //
  IMAGE_SCN_ALIGN_8BYTES               = 0x00400000,  //
  IMAGE_SCN_ALIGN_16BYTES              = 0x00500000,  // Default alignment if no others are specified.
  IMAGE_SCN_ALIGN_32BYTES              = 0x00600000,  //
  IMAGE_SCN_ALIGN_64BYTES              = 0x00700000,  //
  IMAGE_SCN_ALIGN_128BYTES             = 0x00800000,  //
  IMAGE_SCN_ALIGN_256BYTES             = 0x00900000,  //
  IMAGE_SCN_ALIGN_512BYTES             = 0x00A00000,  //
  IMAGE_SCN_ALIGN_1024BYTES            = 0x00B00000,  //
  IMAGE_SCN_ALIGN_2048BYTES            = 0x00C00000,  //
  IMAGE_SCN_ALIGN_4096BYTES            = 0x00D00000,  //
  IMAGE_SCN_ALIGN_8192BYTES            = 0x00E00000,  //
  // Unused                            = 0x00F00000
  IMAGE_SCN_ALIGN_MASK                 = 0x00F00000,

  IMAGE_SCN_LNK_NRELOC_OVFL            = 0x01000000,  // Section contains extended relocations.
  IMAGE_SCN_MEM_DISCARDABLE            = 0x02000000,  // Section can be discarded.
  IMAGE_SCN_MEM_NOT_CACHED             = 0x04000000,  // Section is not cachable.
  IMAGE_SCN_MEM_NOT_PAGED              = 0x08000000,  // Section is not pageable.
  IMAGE_SCN_MEM_SHARED                 = 0x10000000,  // Section is shareable.
  IMAGE_SCN_MEM_EXECUTE                = 0x20000000,  // Section is executable.
  IMAGE_SCN_MEM_READ                   = 0x40000000,  // Section is readable.
  IMAGE_SCN_MEM_WRITE                  = 0x80000000   // Section is writeable.
};

}  // namespace COFF
}  // namespace binlab

#endif  // !BINLAB_BINARYFORMAT_COFF_H_
