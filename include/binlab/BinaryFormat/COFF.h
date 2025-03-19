// binlab/BinaryFormat/COFF.h: definitions for Windows COFF Files

#ifndef BINLAB_BINARYFORMAT_COFF_H_
#define BINLAB_BINARYFORMAT_COFF_H_

#include <cstddef>
#include <cstdint>

namespace binlab {
namespace COFF {

using CHAR        = std::int8_t;
using BYTE        = std::uint8_t;
using SHORT       = std::int16_t;
using WORD        = std::uint16_t;
using LONG        = std::int32_t;
using DWORD       = std::uint32_t;
using ULONGLONG   = std::uint64_t;
using ULONG_PTR   = std::uint64_t;
using VOID        = void;
using PVOID       = void*;

static constexpr WORD   IMAGE_DOS_SIGNATURE  = 0x5A4D;       // MZ
static constexpr DWORD  IMAGE_NT_SIGNATURE   = 0x00004550;   // PE00

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

enum {
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

enum {
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
enum {
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
enum {
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
enum {
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
enum {
//IMAGE_SCN_TYPE_REG                    = 0x00000000,  // Reserved.
//IMAGE_SCN_TYPE_DSECT                  = 0x00000001,  // Reserved.
//IMAGE_SCN_TYPE_NOLOAD                 = 0x00000002,  // Reserved.
//IMAGE_SCN_TYPE_GROUP                  = 0x00000004,  // Reserved.
  IMAGE_SCN_TYPE_NO_PAD                 = 0x00000008,  // Reserved.
//IMAGE_SCN_TYPE_COPY                   = 0x00000010,  // Reserved.

  IMAGE_SCN_CNT_CODE                    = 0x00000020,  // Section contains code.
  IMAGE_SCN_CNT_INITIALIZED_DATA        = 0x00000040,  // Section contains initialized data.
  IMAGE_SCN_CNT_UNINITIALIZED_DATA      = 0x00000080,  // Section contains uninitialized data.

  IMAGE_SCN_LNK_OTHER                   = 0x00000100,  // Reserved.
  IMAGE_SCN_LNK_INFO                    = 0x00000200,  // Section contains comments or some other type of information.
//IMAGE_SCN_TYPE_OVER                   = 0x00000400,  // Reserved.
  IMAGE_SCN_LNK_REMOVE                  = 0x00000800,  // Section contents will not become part of image.
  IMAGE_SCN_LNK_COMDAT                  = 0x00001000,  // Section contents comdat.
//                                      = 0x00002000,  // Reserved.
//IMAGE_SCN_MEM_PROTECTED - Obsolete    = 0x00004000,
  IMAGE_SCN_NO_DEFER_SPEC_EXC           = 0x00004000,  // Reset speculative exceptions handling bits in the TLB entries for this section.
  IMAGE_SCN_GPREL                       = 0x00008000,  // Section content can be accessed relative to GP
  IMAGE_SCN_MEM_FARDATA                 = 0x00008000,
//IMAGE_SCN_MEM_SYSHEAP  - Obsolete     = 0x00010000,
  IMAGE_SCN_MEM_PURGEABLE               = 0x00020000,
  IMAGE_SCN_MEM_16BIT                   = 0x00020000,
  IMAGE_SCN_MEM_LOCKED                  = 0x00040000,
  IMAGE_SCN_MEM_PRELOAD                 = 0x00080000,

  IMAGE_SCN_ALIGN_1BYTES                = 0x00100000,
  IMAGE_SCN_ALIGN_2BYTES                = 0x00200000,
  IMAGE_SCN_ALIGN_4BYTES                = 0x00300000,
  IMAGE_SCN_ALIGN_8BYTES                = 0x00400000,
  IMAGE_SCN_ALIGN_16BYTES               = 0x00500000,  // Default alignment if no others are specified.
  IMAGE_SCN_ALIGN_32BYTES               = 0x00600000,
  IMAGE_SCN_ALIGN_64BYTES               = 0x00700000,
  IMAGE_SCN_ALIGN_128BYTES              = 0x00800000,
  IMAGE_SCN_ALIGN_256BYTES              = 0x00900000,
  IMAGE_SCN_ALIGN_512BYTES              = 0x00A00000,
  IMAGE_SCN_ALIGN_1024BYTES             = 0x00B00000,
  IMAGE_SCN_ALIGN_2048BYTES             = 0x00C00000,
  IMAGE_SCN_ALIGN_4096BYTES             = 0x00D00000,
  IMAGE_SCN_ALIGN_8192BYTES             = 0x00E00000,
  // Unused                             = 0x00F00000
  IMAGE_SCN_ALIGN_MASK                  = 0x00F00000,

  IMAGE_SCN_LNK_NRELOC_OVFL             = 0x01000000,  // Section contains extended relocations.
  IMAGE_SCN_MEM_DISCARDABLE             = 0x02000000,  // Section can be discarded.
  IMAGE_SCN_MEM_NOT_CACHED              = 0x04000000,  // Section is not cachable.
  IMAGE_SCN_MEM_NOT_PAGED               = 0x08000000,  // Section is not pageable.
  IMAGE_SCN_MEM_SHARED                  = 0x10000000,  // Section is shareable.
  IMAGE_SCN_MEM_EXECUTE                 = 0x20000000,  // Section is executable.
  IMAGE_SCN_MEM_READ                    = 0x40000000,  // Section is readable.
  IMAGE_SCN_MEM_WRITE                   = 0x80000000   // Section is writeable.
};

// Symbol format.
#ifdef _WIN32
#pragma pack(push, 1)
#else
#pragma pack(1)
#endif
struct IMAGE_SYMBOL {
  union {
    BYTE ShortName[8];
    struct {
      DWORD Short;  // if 0, use LongName
      DWORD Long;   // offset into string table
    } Name;
    DWORD LongName[2];  // PBYTE [2]
  } N;
  DWORD Value;
  SHORT SectionNumber;
  WORD Type;
  BYTE StorageClass;
  BYTE NumberOfAuxSymbols;
};
#ifdef _WIN32
#pragma pack(pop)
#else
#pragma pack()
#endif

// Type (fundamental) values.
enum {
  IMAGE_SYM_TYPE_NULL                   = 0x0000,  // no type.
  IMAGE_SYM_TYPE_VOID                   = 0x0001,
  IMAGE_SYM_TYPE_CHAR                   = 0x0002,  // type character.
  IMAGE_SYM_TYPE_SHORT                  = 0x0003,  // type short integer.
  IMAGE_SYM_TYPE_INT                    = 0x0004,
  IMAGE_SYM_TYPE_LONG                   = 0x0005,
  IMAGE_SYM_TYPE_FLOAT                  = 0x0006,
  IMAGE_SYM_TYPE_DOUBLE                 = 0x0007,
  IMAGE_SYM_TYPE_STRUCT                 = 0x0008,
  IMAGE_SYM_TYPE_UNION                  = 0x0009,
  IMAGE_SYM_TYPE_ENUM                   = 0x000A,  // enumeration.
  IMAGE_SYM_TYPE_MOE                    = 0x000B,  // member of enumeration.
  IMAGE_SYM_TYPE_BYTE                   = 0x000C,
  IMAGE_SYM_TYPE_WORD                   = 0x000D,
  IMAGE_SYM_TYPE_UINT                   = 0x000E,
  IMAGE_SYM_TYPE_DWORD                  = 0x000F,
  IMAGE_SYM_TYPE_PCODE                  = 0x8000
};

// Type (derived) values.
enum {
  IMAGE_SYM_DTYPE_NULL                  = 0,  // no derived type.
  IMAGE_SYM_DTYPE_POINTER               = 1,  // pointer.
  IMAGE_SYM_DTYPE_FUNCTION              = 2,  // function.
  IMAGE_SYM_DTYPE_ARRAY                 = 3   // array.
};

// Storage classes.
enum {
  IMAGE_SYM_CLASS_END_OF_FUNCTION       = (BYTE )-1,
  IMAGE_SYM_CLASS_NULL                  = 0x0000,
  IMAGE_SYM_CLASS_AUTOMATIC             = 0x0001,
  IMAGE_SYM_CLASS_EXTERNAL              = 0x0002,
  IMAGE_SYM_CLASS_STATIC                = 0x0003,
  IMAGE_SYM_CLASS_REGISTER              = 0x0004,
  IMAGE_SYM_CLASS_EXTERNAL_DEF          = 0x0005,
  IMAGE_SYM_CLASS_LABEL                 = 0x0006,
  IMAGE_SYM_CLASS_UNDEFINED_LABEL       = 0x0007,
  IMAGE_SYM_CLASS_MEMBER_OF_STRUCT      = 0x0008,
  IMAGE_SYM_CLASS_ARGUMENT              = 0x0009,
  IMAGE_SYM_CLASS_STRUCT_TAG            = 0x000A,
  IMAGE_SYM_CLASS_MEMBER_OF_UNION       = 0x000B,
  IMAGE_SYM_CLASS_UNION_TAG             = 0x000C,
  IMAGE_SYM_CLASS_TYPE_DEFINITION       = 0x000D,
  IMAGE_SYM_CLASS_UNDEFINED_STATIC      = 0x000E,
  IMAGE_SYM_CLASS_ENUM_TAG              = 0x000F,
  IMAGE_SYM_CLASS_MEMBER_OF_ENUM        = 0x0010,
  IMAGE_SYM_CLASS_REGISTER_PARAM        = 0x0011,
  IMAGE_SYM_CLASS_BIT_FIELD             = 0x0012,

  IMAGE_SYM_CLASS_FAR_EXTERNAL          = 0x0044,

  IMAGE_SYM_CLASS_BLOCK                 = 0x0064,
  IMAGE_SYM_CLASS_FUNCTION              = 0x0065,
  IMAGE_SYM_CLASS_END_OF_STRUCT         = 0x0066,
  IMAGE_SYM_CLASS_FILE                  = 0x0067,
// new
  IMAGE_SYM_CLASS_SECTION               = 0x0068,
  IMAGE_SYM_CLASS_WEAK_EXTERNAL         = 0x0069,

  IMAGE_SYM_CLASS_CLR_TOKEN             = 0x006B
};

// Relocation format.
#ifdef _WIN32
#pragma pack( push, 1 )
#else
#pragma pack(1)
#endif
struct IMAGE_RELOCATION {
  union {
    DWORD   VirtualAddress;
    DWORD   RelocCount;             // Set to the real count when IMAGE_SCN_LNK_NRELOC_OVFL is set
  };
  DWORD   SymbolTableIndex;
  WORD    Type;
};
#ifdef _WIN32
#pragma pack(pop)
#else
#pragma pack()
#endif

// I386 relocation types.
enum {
  IMAGE_REL_I386_ABSOLUTE         = 0x0000,  // Reference is absolute, no relocation is necessary
  IMAGE_REL_I386_DIR16            = 0x0001,  // Direct 16-bit reference to the symbols virtual address
  IMAGE_REL_I386_REL16            = 0x0002,  // PC-relative 16-bit reference to the symbols virtual address
  IMAGE_REL_I386_DIR32            = 0x0006,  // Direct 32-bit reference to the symbols virtual address
  IMAGE_REL_I386_DIR32NB          = 0x0007,  // Direct 32-bit reference to the symbols virtual address, base not included
  IMAGE_REL_I386_SEG12            = 0x0009,  // Direct 16-bit reference to the segment-selector bits of a 32-bit virtual address
  IMAGE_REL_I386_SECTION          = 0x000A,
  IMAGE_REL_I386_SECREL           = 0x000B,
  IMAGE_REL_I386_TOKEN            = 0x000C,  // clr token
  IMAGE_REL_I386_SECREL7          = 0x000D,  // 7 bit offset from base of section containing target
  IMAGE_REL_I386_REL32            = 0x0014,  // PC-relative 32-bit reference to the symbols virtual address
};

// x64 relocations
enum {
  IMAGE_REL_AMD64_ABSOLUTE        = 0x0000,  // Reference is absolute, no relocation is necessary
  IMAGE_REL_AMD64_ADDR64          = 0x0001,  // 64-bit address (VA).
  IMAGE_REL_AMD64_ADDR32          = 0x0002,  // 32-bit address (VA).
  IMAGE_REL_AMD64_ADDR32NB        = 0x0003,  // 32-bit address w/o image base (RVA).
  IMAGE_REL_AMD64_REL32           = 0x0004,  // 32-bit relative address from byte following reloc
  IMAGE_REL_AMD64_REL32_1         = 0x0005,  // 32-bit relative address from byte distance 1 from reloc
  IMAGE_REL_AMD64_REL32_2         = 0x0006,  // 32-bit relative address from byte distance 2 from reloc
  IMAGE_REL_AMD64_REL32_3         = 0x0007,  // 32-bit relative address from byte distance 3 from reloc
  IMAGE_REL_AMD64_REL32_4         = 0x0008,  // 32-bit relative address from byte distance 4 from reloc
  IMAGE_REL_AMD64_REL32_5         = 0x0009,  // 32-bit relative address from byte distance 5 from reloc
  IMAGE_REL_AMD64_SECTION         = 0x000A,  // Section index
  IMAGE_REL_AMD64_SECREL          = 0x000B,  // 32 bit offset from base of section containing target
  IMAGE_REL_AMD64_SECREL7         = 0x000C,  // 7 bit unsigned offset from base of section containing target
  IMAGE_REL_AMD64_TOKEN           = 0x000D,  // 32 bit metadata token
  IMAGE_REL_AMD64_SREL32          = 0x000E,  // 32 bit signed span-dependent value emitted into object
  IMAGE_REL_AMD64_PAIR            = 0x000F,
  IMAGE_REL_AMD64_SSPAN32         = 0x0010,  // 32 bit signed span-dependent value applied at link time
  IMAGE_REL_AMD64_EHANDLER        = 0x0011,
  IMAGE_REL_AMD64_IMPORT_BR       = 0x0012,  // Indirect branch to an import
  IMAGE_REL_AMD64_IMPORT_CALL     = 0x0013,  // Indirect call to an import
  IMAGE_REL_AMD64_CFG_BR          = 0x0014,  // Indirect branch to a CFG check
  IMAGE_REL_AMD64_CFG_BR_REX      = 0x0015,  // Indirect branch to a CFG check, with REX.W prefix
  IMAGE_REL_AMD64_CFG_CALL        = 0x0016,  // Indirect call to a CFG check
  IMAGE_REL_AMD64_INDIR_BR        = 0x0017,  // Indirect branch to a target in RAX (no CFG)
  IMAGE_REL_AMD64_INDIR_BR_REX    = 0x0018,  // Indirect branch to a target in RAX, with REX.W prefix (no CFG)
  IMAGE_REL_AMD64_INDIR_CALL      = 0x0019,  // Indirect call to a target in RAX (no CFG)
  IMAGE_REL_AMD64_INDIR_BR_SWITCHTABLE_FIRST  = 0x0020, // Indirect branch for a switch table using Reg 0 (RAX)
  IMAGE_REL_AMD64_INDIR_BR_SWITCHTABLE_LAST   = 0x002F // Indirect branch for a switch table using Reg 15 (R15)
};

// Line number format.
struct IMAGE_LINENUMBER {
  union {
    DWORD   SymbolTableIndex;               // Symbol table index of function name if Linenumber is 0.
    DWORD   VirtualAddress;                 // Virtual address of line number.
  } Type;
  WORD    Linenumber;                         // Line number.
};

// Based relocation format.
struct IMAGE_BASE_RELOCATION {
    DWORD   VirtualAddress;
    DWORD   SizeOfBlock;
//  WORD    TypeOffset[1];
};

// Export Format
struct IMAGE_EXPORT_DIRECTORY {
  DWORD   Characteristics;
  DWORD   TimeDateStamp;
  WORD    MajorVersion;
  WORD    MinorVersion;
  DWORD   Name;
  DWORD   Base;
  DWORD   NumberOfFunctions;
  DWORD   NumberOfNames;
  DWORD   AddressOfFunctions;     // RVA from base of image
  DWORD   AddressOfNames;         // RVA from base of image
  DWORD   AddressOfNameOrdinals;  // RVA from base of image
};

// Import Format
struct IMAGE_IMPORT_BY_NAME {
  WORD    Hint;
  CHAR    Name[1];
};

struct IMAGE_THUNK_DATA64 {
  union {
    ULONGLONG ForwarderString;  // PBYTE 
    ULONGLONG Function;         // PDWORD
    ULONGLONG Ordinal;
    ULONGLONG AddressOfData;    // PIMAGE_IMPORT_BY_NAME
  } u1;
};

struct IMAGE_THUNK_DATA32 {
  union {
    DWORD ForwarderString;      // PBYTE 
    DWORD Function;             // PDWORD
    DWORD Ordinal;
    DWORD AddressOfData;        // PIMAGE_IMPORT_BY_NAME
  } u1;
};

#define IMAGE_ORDINAL_FLAG64 0x8000000000000000
#define IMAGE_ORDINAL_FLAG32 0x80000000
#define IMAGE_ORDINAL64(Ordinal) (Ordinal & 0xffff)
#define IMAGE_ORDINAL32(Ordinal) (Ordinal & 0xffff)
#define IMAGE_SNAP_BY_ORDINAL64(Ordinal) ((Ordinal & IMAGE_ORDINAL_FLAG64) != 0)
#define IMAGE_SNAP_BY_ORDINAL32(Ordinal) ((Ordinal & IMAGE_ORDINAL_FLAG32) != 0)

// Thread Local Storage
#if (_MSC_VER >= 800) || defined(_STDCALL_SUPPORTED)
#define NTAPI __stdcall
#else
#define NTAPI
#endif
typedef VOID (NTAPI *PIMAGE_TLS_CALLBACK) (PVOID DllHandle, DWORD Reason, PVOID Reserved);

struct IMAGE_TLS_DIRECTORY64 {
  ULONGLONG StartAddressOfRawData;
  ULONGLONG EndAddressOfRawData;
  ULONGLONG AddressOfIndex;         // PDWORD
  ULONGLONG AddressOfCallBacks;     // PIMAGE_TLS_CALLBACK *;
  DWORD SizeOfZeroFill;
  union {
    DWORD Characteristics;
    struct {
       DWORD Reserved0 : 20;
       DWORD Alignment : 4;
       DWORD Reserved1 : 8;
    };
  };
};

struct IMAGE_TLS_DIRECTORY32 {
  DWORD   StartAddressOfRawData;
  DWORD   EndAddressOfRawData;
  DWORD   AddressOfIndex;             // PDWORD
  DWORD   AddressOfCallBacks;         // PIMAGE_TLS_CALLBACK *
  DWORD   SizeOfZeroFill;
  union {
    DWORD Characteristics;
    struct {
      DWORD Reserved0 : 20;
      DWORD Alignment : 4;
      DWORD Reserved1 : 8;
    };
  };
};

struct IMAGE_IMPORT_DESCRIPTOR {
  union {
    DWORD   Characteristics;            // 0 for terminating null import descriptor
    DWORD   OriginalFirstThunk;         // RVA to original unbound IAT (PIMAGE_THUNK_DATA)
  };
  DWORD   TimeDateStamp;                  // 0 if not bound,
                                          // -1 if bound, and real date\time stamp
                                          //     in IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT (new BIND)
                                          // O.W. date/time stamp of DLL bound to (Old BIND)

  DWORD   ForwarderChain;                 // -1 if no forwarders
  DWORD   Name;
  DWORD   FirstThunk;                     // RVA to IAT (if bound this IAT has actual addresses)
};

// New format import descriptors pointed to by DataDirectory[ IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT ]
struct IMAGE_BOUND_IMPORT_DESCRIPTOR {
  DWORD   TimeDateStamp;
  WORD    OffsetModuleName;
  WORD    NumberOfModuleForwarderRefs;
// Array of zero or more IMAGE_BOUND_FORWARDER_REF follows
};

struct IMAGE_BOUND_FORWARDER_REF {
  DWORD   TimeDateStamp;
  WORD    OffsetModuleName;
  WORD    Reserved;
};

struct IMAGE_DELAYLOAD_DESCRIPTOR {
  union {
    DWORD AllAttributes;
    struct {
      DWORD RvaBased : 1;                 // Delay load version 2
      DWORD ReservedAttributes : 31;
    } u;
  } Attributes;

  DWORD DllNameRVA;                       // RVA to the name of the target library (NULL-terminate ASCII string)
  DWORD ModuleHandleRVA;                  // RVA to the HMODULE caching location (PHMODULE)
  DWORD ImportAddressTableRVA;            // RVA to the start of the IAT (PIMAGE_THUNK_DATA)
  DWORD ImportNameTableRVA;               // RVA to the start of the name table (PIMAGE_THUNK_DATA::AddressOfData)
  DWORD BoundImportAddressTableRVA;       // RVA to an optional bound IAT
  DWORD UnloadInformationTableRVA;        // RVA to an optional unload info table
  DWORD TimeDateStamp;                    // 0 if not bound,
                                          // Otherwise, date/time of the target DLL
};

// Resource Format.

struct IMAGE_RESOURCE_DIRECTORY {
  DWORD   Characteristics;
  DWORD   TimeDateStamp;
  WORD    MajorVersion;
  WORD    MinorVersion;
  WORD    NumberOfNamedEntries;
  WORD    NumberOfIdEntries;
//IMAGE_RESOURCE_DIRECTORY_ENTRY DirectoryEntries[];
};

struct IMAGE_RESOURCE_DIRECTORY_ENTRY {
  union {
    struct {
      DWORD NameOffset:31;
      DWORD NameIsString:1;
    };
    DWORD   Name;
    WORD    Id;
  };
  union {
    DWORD   OffsetToData;
    struct {
      DWORD   OffsetToDirectory:31;
      DWORD   DataIsDirectory:1;
    };
  };
};

struct IMAGE_RESOURCE_DATA_ENTRY {
  DWORD   OffsetToData;
  DWORD   Size;
  DWORD   CodePage;
  DWORD   Reserved;
};

}  // namespace COFF
}  // namespace binlab

#endif  // !BINLAB_BINARYFORMAT_COFF_H_
