// binlab/BinaryFormat/ELF.h: definitions for Linux ELF Files

#ifndef BINLAB_BINARYFORMAT_ELF_H_
#define BINLAB_BINARYFORMAT_ELF_H_

#include <cstdint>

namespace binlab {
namespace ELF {

using Elf32_Addr = std::uint32_t; // Program address
using Elf32_Off = std::uint32_t;  // File offset
using Elf32_Half = std::uint16_t;
using Elf32_Word = std::uint32_t;
using Elf32_Sword = std::int32_t;

using Elf64_Addr = std::uint64_t;
using Elf64_Off = std::uint64_t;
using Elf64_Half = std::uint16_t;
using Elf64_Word = std::uint32_t;
using Elf64_Sword = std::int32_t;
using Elf64_Xword = std::uint64_t;
using Elf64_Sxword = std::int64_t;

// The ELF file header.  This appears at the start of every ELF file.
static constexpr std::size_t EI_NIDENT = 16;

struct Elf32_Ehdr {
  unsigned char e_ident[EI_NIDENT];  // Magic number and other info
  Elf32_Half    e_type;              // Object file type
  Elf32_Half    e_machine;           // Architecture
  Elf32_Word    e_version;           // Object file version
  Elf32_Addr    e_entry;             // Entry point virtual address
  Elf32_Off     e_phoff;             // Program header table file offset
  Elf32_Off     e_shoff;             // Section header table file offset
  Elf32_Word    e_flags;             // Processor-specific flags
  Elf32_Half    e_ehsize;            // ELF header size in bytes
  Elf32_Half    e_phentsize;         // Program header table entry size
  Elf32_Half    e_phnum;             // Program header table entry count
  Elf32_Half    e_shentsize;         // Section header table entry size
  Elf32_Half    e_shnum;             // Section header table entry count
  Elf32_Half    e_shstrndx;          // Section header string table index
};

struct Elf64_Ehdr {
  unsigned char e_ident[EI_NIDENT];  // Magic number and other info
  Elf64_Half    e_type;              // Object file type
  Elf64_Half    e_machine;           // Architecture
  Elf64_Word    e_version;           // Object file version
  Elf64_Addr    e_entry;             // Entry point virtual address
  Elf64_Off     e_phoff;             // Program header table file offset
  Elf64_Off     e_shoff;             // Section header table file offset
  Elf64_Word    e_flags;             // Processor-specific flags
  Elf64_Half    e_ehsize;            // ELF header size in bytes
  Elf64_Half    e_phentsize;         // Program header table entry size
  Elf64_Half    e_phnum;             // Program header table entry count
  Elf64_Half    e_shentsize;         // Section header table entry size
  Elf64_Half    e_shnum;             // Section header table entry count
  Elf64_Half    e_shstrndx;          // Section header string table index
};

// Fields in the e_ident array
// The EI_* macros are indices into the array.  The macros under each EI_* macro are the values the byte may have.

static constexpr std::size_t EI_MAG0 = 0;     // File identification byte 0 index
static constexpr std::uint8_t ELFMAG0 = 0x7f; // Magic number byte 0

static constexpr std::size_t EI_MAG1 = 1;     // File identification byte 1 index
static constexpr std::uint8_t ELFMAG1 = 'E';  // Magic number byte 1

static constexpr std::size_t EI_MAG2 = 2;     // File identification byte 2 index
static constexpr std::uint8_t ELFMAG2 = 'L';  // Magic number byte 2

static constexpr std::size_t EI_MAG3 = 3;     // File identification byte 3 index
static constexpr std::uint8_t ELFMAG3 = 'F';  // Magic number byte 3

// Conglomeration of the identification bytes, for easy testing as a word.
static constexpr std::uint8_t ELFMAG[] = "\177ELF";
static constexpr std::size_t SELFMAG = sizeof(ELFMAG) - 1;

static constexpr std::size_t EI_CLASS = 4;    // File class byte index
enum {
  ELFCLASSNONE = 0, // Invalid class
  ELFCLASS32 = 1,   // 32-bit object file
  ELFCLASS64 = 2    // 64-bit object file
};
static constexpr std::size_t ELFCLASSNUM = 3;

static constexpr std::size_t EI_DATA = 5;     // Data encoding byte index
enum {
  ELFDATANONE = 0,  // Invalid data encoding
  ELFDATA2LSB = 1,  // 2's complement, little endian
  ELFDATA2MSB = 2   // 2's complement, big endian
};
static constexpr std::size_t ELFDATANUM = 3;

static constexpr std::size_t EI_VERSION = 6;  // File version byte index, value must be EV_CURRENT

static constexpr std::size_t EI_OSABI = 7;    // OS ABI identification
enum {
  ELFOSABI_NONE = 0,          // UNIX System V ABI
  ELFOSABI_SYSV = 0,          // Alias
  ELFOSABI_HPUX = 1,          // HP-UX
  ELFOSABI_NETBSD = 2,        // NetBSD
  ELFOSABI_GNU = 3,           // Object uses GNU ELF extensions
  ELFOSABI_LINUX = 3,         // Compatibility alias
  ELFOSABI_HURD = 4,          // GNU/Hurd
  ELFOSABI_SOLARIS = 6,       // Sun Solaris
  ELFOSABI_AIX = 7,           // IBM AIX
  ELFOSABI_IRIX = 8,          // SGI Irix
  ELFOSABI_FREEBSD = 9,       // FreeBSD
  ELFOSABI_TRU64 = 10,        // Compaq TRU64 UNIX
  ELFOSABI_MODESTO = 11,      // Novell Modesto
  ELFOSABI_OPENBSD = 12,      // OpenBSD
  ELFOSABI_OPENVMS = 13,      // OpenVMS
  ELFOSABI_NSK = 14,          // Hewlett-Packard Non-Stop Kernel
  ELFOSABI_AROS = 15,         // AROS
  ELFOSABI_FENIXOS = 16,      // FenixOS
  ELFOSABI_CLOUDABI = 17,     // Nuxi CloudABI
  ELFOSABI_ARM_AEABI = 64,    // ARM EABI
  ELFOSABI_ARM = 97,          // ARM
  ELFOSABI_STANDALONE = 255,  // Standalone (embedded) application
};

static constexpr std::size_t EI_ABIVERSION = 8; // ABI version

static constexpr std::size_t EI_PAD = 9;      // Byte index of padding bytes

// Legal values for e_type (object file type).
enum {
  ET_NONE = 0,        // No file type
  ET_REL = 1,         // Relocatable file
  ET_EXEC = 2,        // Executable file
  ET_DYN = 3,         // Shared object file
  ET_CORE = 4,        // Core file
  ET_NUM = 5,         // Number of defined types
  ET_LOOS = 0xfe00,   // OS-specific range start
  ET_HIOS = 0xfeff,   // OS-specific range end
  ET_LOPROC = 0xff00, // Processor-specific range start
  ET_HIPROC = 0xffff  // Processor-specific range end
};

// Legal values for e_machine (architecture).
enum {
  EM_NONE = 0,            // No machine
  EM_M32 = 1,             // AT&T WE 32100
  EM_SPARC = 2,           // SUN SPARC
  EM_386 = 3,             // Intel 80386
  EM_68K = 4,             // Motorola m68k family
  EM_88K = 5,             // Motorola m88k family
  EM_IAMCU = 6,           // Intel MCU
  EM_860 = 7,             // Intel 80860
  EM_MIPS = 8,            // MIPS R3000 big-endian
  EM_S370 = 9,            // IBM System/370
  EM_MIPS_RS3_LE = 10,    // MIPS R3000 little-endian
                          // reserved 11-14
  EM_PARISC = 15,         // HPPA
                          // reserved 16
  EM_VPP500 = 17,         // Fujitsu VPP500
  EM_SPARC32PLUS = 18,    // Sun's "v8plus"
  EM_960 = 19,            // Intel 80960
  EM_PPC = 20,            // PowerPC
  EM_PPC64 = 21,          // PowerPC 64-bit
  EM_S390 = 22,           // IBM S390
  EM_SPU = 23,            // IBM SPU/SPC
                          // reserved 24-35
  EM_V800 = 36,           // NEC V800 series
  EM_FR20 = 37,           // Fujitsu FR20
  EM_RH32 = 38,           // TRW RH-32
  EM_RCE = 39,            // Motorola RCE
  EM_ARM = 40,            // ARM
  EM_FAKE_ALPHA = 41,     // Digital Alpha
  EM_SH = 42,             // Hitachi SH
  EM_SPARCV9 = 43,        // SPARC v9 64-bit
  EM_TRICORE = 44,        // Siemens Tricore
  EM_ARC = 45,            // Argonaut RISC Core
  EM_H8_300 = 46,         // Hitachi H8/300
  EM_H8_300H = 47,        // Hitachi H8/300H
  EM_H8S = 48,            // Hitachi H8S
  EM_H8_500 = 49,         // Hitachi H8/500
  EM_IA_64 = 50,          // Intel Merced
  EM_MIPS_X = 51,         // Stanford MIPS-X
  EM_COLDFIRE = 52,       // Motorola Coldfire
  EM_68HC12 = 53,         // Motorola M68HC12
  EM_MMA = 54,            // Fujitsu MMA Multimedia Accelerator
  EM_PCP = 55,            // Siemens PCP
  EM_NCPU = 56,           // Sony nCPU embeeded RISC
  EM_NDR1 = 57,           // Denso NDR1 microprocessor
  EM_STARCORE = 58,       // Motorola Start*Core processor
  EM_ME16 = 59,           // Toyota ME16 processor
  EM_ST100 = 60,          // STMicroelectronic ST100 processor
  EM_TINYJ = 61,          // Advanced Logic Corp. Tinyj emb.fam
  EM_X86_64 = 62,         // AMD x86-64 architecture
  EM_PDSP = 63,           // Sony DSP Processor
  EM_PDP10 = 64,          // Digital PDP-10
  EM_PDP11 = 65,          // Digital PDP-11
  EM_FX66 = 66,           // Siemens FX66 microcontroller
  EM_ST9PLUS = 67,        // STMicroelectronics ST9+ 8/16 mc
  EM_ST7 = 68,            // STmicroelectronics ST7 8 bit mc
  EM_68HC16 = 69,         // Motorola MC68HC16 microcontroller
  EM_68HC11 = 70,         // Motorola MC68HC11 microcontroller
  EM_68HC08 = 71,         // Motorola MC68HC08 microcontroller
  EM_68HC05 = 72,         // Motorola MC68HC05 microcontroller
  EM_SVX = 73,            // Silicon Graphics SVx
  EM_ST19 = 74,           // STMicroelectronics ST19 8 bit mc
  EM_VAX = 75,            // Digital VAX
  EM_CRIS = 76,           // Axis Communications 32-bit emb.proc
  EM_JAVELIN = 77,        // Infineon Technologies 32-bit emb.proc
  EM_FIREPATH = 78,       // Element 14 64-bit DSP Processor
  EM_ZSP = 79,            // LSI Logic 16-bit DSP Processor
  EM_MMIX = 80,           // Donald Knuth's educational 64-bit proc
  EM_HUANY = 81,          // Harvard University machine-independent object files
  EM_PRISM = 82,          // SiTera Prism
  EM_AVR = 83,            // Atmel AVR 8-bit microcontroller
  EM_FR30 = 84,           // Fujitsu FR30
  EM_D10V = 85,           // Mitsubishi D10V
  EM_D30V = 86,           // Mitsubishi D30V
  EM_V850 = 87,           // NEC v850
  EM_M32R = 88,           // Mitsubishi M32R
  EM_MN10300 = 89,        // Matsushita MN10300
  EM_MN10200 = 90,        // Matsushita MN10200
  EM_PJ = 91,             // picoJava
  EM_OPENRISC = 92,       // OpenRISC 32-bit embedded processor
  EM_ARC_COMPACT = 93,    // ARC International ARCompact
  EM_XTENSA = 94,         // Tensilica Xtensa Architecture
  EM_VIDEOCORE = 95,      // Alphamosaic VideoCore
  EM_TMM_GPP = 96,        // Thompson Multimedia General Purpose Proc
  EM_NS32K = 97,          // National Semi. 32000
  EM_TPC = 98,            // Tenor Network TPC
  EM_SNP1K = 99,          // Trebia SNP 1000
  EM_ST200 = 100,         // STMicroelectronics ST200
  EM_IP2K = 101,          // Ubicom IP2xxx
  EM_MAX = 102,           // MAX processor
  EM_CR = 103,            // National Semi. CompactRISC
  EM_F2MC16 = 104,        // Fujitsu F2MC16
  EM_MSP430 = 105,        // Texas Instruments msp430
  EM_BLACKFIN = 106,      // Analog Devices Blackfin DSP
  EM_SE_C33 = 107,        // Seiko Epson S1C33 family
  EM_SEP = 108,           // Sharp embedded microprocessor
  EM_ARCA = 109,          // Arca RISC
  EM_UNICORE = 110,       // PKU-Unity & MPRC Peking Uni. mc series
  EM_EXCESS = 111,        // eXcess configurable cpu
  EM_DXP = 112,           // Icera Semi. Deep Execution Processor
  EM_ALTERA_NIOS2 = 113,  // Altera Nios II
  EM_CRX = 114,           // National Semi. CompactRISC CRX
  EM_XGATE = 115,         // Motorola XGATE
  EM_C166 = 116,          // Infineon C16x/XC16x
  EM_M16C = 117,          // Renesas M16C
  EM_DSPIC30F = 118,      // Microchip Technology dsPIC30F
  EM_CE = 119,            // Freescale Communication Engine RISC
  EM_M32C = 120,          // Renesas M32C
                          // reserved 121-130
  EM_TSK3000 = 131,       // Altium TSK3000
  EM_RS08 = 132,          // Freescale RS08
  EM_SHARC = 133,         // Analog Devices SHARC family
  EM_ECOG2 = 134,         // Cyan Technology eCOG2
  EM_SCORE7 = 135,        // Sunplus S+core7 RISC
  EM_DSP24 = 136,         // New Japan Radio (NJR) 24-bit DSP
  EM_VIDEOCORE3 = 137,    // Broadcom VideoCore III
  EM_LATTICEMICO32 = 138,  // RISC for Lattice FPGA
  EM_SE_C17 = 139,         // Seiko Epson C17
  EM_TI_C6000 = 140,       // Texas Instruments TMS320C6000 DSP
  EM_TI_C2000 = 141,       // Texas Instruments TMS320C2000 DSP
  EM_TI_C5500 = 142,       // Texas Instruments TMS320C55x DSP
  EM_TI_ARP32 = 143,       // Texas Instruments App. Specific RISC
  EM_TI_PRU = 144,         // Texas Instruments Prog. Realtime Unit
                           // reserved 145-159
  EM_MMDSP_PLUS = 160,     // STMicroelectronics 64bit VLIW DSP
  EM_CYPRESS_M8C = 161,    // Cypress M8C
  EM_R32C = 162,           // Renesas R32C
  EM_TRIMEDIA = 163,       // NXP Semi. TriMedia
  EM_QDSP6 = 164,          // QUALCOMM DSP6
  EM_8051 = 165,           // Intel 8051 and variants
  EM_STXP7X = 166,         // STMicroelectronics STxP7x
  EM_NDS32 = 167,          // Andes Tech. compact code emb. RISC
  EM_ECOG1X = 168,         // Cyan Technology eCOG1X
  EM_MAXQ30 = 169,         // Dallas Semi. MAXQ30 mc
  EM_XIMO16 = 170,         // New Japan Radio (NJR) 16-bit DSP
  EM_MANIK = 171,          // M2000 Reconfigurable RISC
  EM_CRAYNV2 = 172,        // Cray NV2 vector architecture
  EM_RX = 173,             // Renesas RX
  EM_METAG = 174,          // Imagination Tech. META
  EM_MCST_ELBRUS = 175,    // MCST Elbrus
  EM_ECOG16 = 176,         // Cyan Technology eCOG16
  EM_CR16 = 177,           // National Semi. CompactRISC CR16
  EM_ETPU = 178,           // Freescale Extended Time Processing Unit
  EM_SLE9X = 179,          // Infineon Tech. SLE9X
  EM_L10M = 180,           // Intel L10M
  EM_K10M = 181,           // Intel K10M
                           // reserved 182
  EM_AARCH64 = 183,        // ARM AARCH64
                           // reserved 184
  EM_AVR32 = 185,          // Amtel 32-bit microprocessor
  EM_STM8 = 186,           // STMicroelectronics STM8
  EM_TILE64 = 187,         // Tileta TILE64
  EM_TILEPRO = 188,        // Tilera TILEPro
  EM_MICROBLAZE = 189,     // Xilinx MicroBlaze
  EM_CUDA = 190,           // NVIDIA CUDA
  EM_TILEGX = 191,         // Tilera TILE-Gx
  EM_CLOUDSHIELD = 192,    // CloudShield
  EM_COREA_1ST = 193,      // KIPO-KAIST Core-A 1st gen.
  EM_COREA_2ND = 194,      // KIPO-KAIST Core-A 2nd gen.
  EM_ARC_COMPACT2 = 195,   // Synopsys ARCompact V2
  EM_OPEN8 = 196,          // Open8 RISC
  EM_RL78 = 197,           // Renesas RL78
  EM_VIDEOCORE5 = 198,     // Broadcom VideoCore V
  EM_78KOR = 199,          // Renesas 78KOR
  EM_56800EX = 200,        // Freescale 56800EX DSC
  EM_BA1 = 201,            // Beyond BA1
  EM_BA2 = 202,            // Beyond BA2
  EM_XCORE = 203,          // XMOS xCORE
  EM_MCHP_PIC = 204,       // Microchip 8-bit PIC(r)
                           // reserved 205-209
  EM_KM32 = 210,           // KM211 KM32
  EM_KMX32 = 211,          // KM211 KMX32
  EM_EMX16 = 212,          // KM211 KMX16
  EM_EMX8 = 213,           // KM211 KMX8
  EM_KVARC = 214,          // KM211 KVARC
  EM_CDP = 215,            // Paneve CDP
  EM_COGE = 216,           // Cognitive Smart Memory Processor
  EM_COOL = 217,           // Bluechip CoolEngine
  EM_NORC = 218,           // Nanoradio Optimized RISC
  EM_CSR_KALIMBA = 219,    // CSR Kalimba
  EM_Z80 = 220,            // Zilog Z80
  EM_VISIUM = 221,         // Controls and Data Services VISIUMcore
  EM_FT32 = 222,           // FTDI Chip FT32
  EM_MOXIE = 223,          // Moxie processor
  EM_AMDGPU = 224,         // AMD GPU
                           // reserved 225-242
  EM_RISCV = 243,          // RISC-V

  EM_BPF = 247,   // Linux BPF -- in-kernel virtual machine
  EM_CSKY = 252,  // C-SKY

  EM_NUM = 253,

  // Old spellings/synonyms.
  EM_ARC_A5 = EM_ARC_COMPACT,

  // If it is necessary to assign new unofficial EM_* values, please pick large
  // random numbers (0x8523, 0xa7f2, etc.) to minimize the chances of collision
  // with official or non-GNU unofficial values.
  EM_ALPHA = 0x9026
};

// Legal values for e_version (version).
enum {
  EV_NONE = 0,    // Invalid ELF version
  EV_CURRENT = 1  // Current version
};
static constexpr std::size_t EV_NUM = 2;

// Program segment header.
struct Elf32_Phdr {
  Elf32_Word  p_type;     // Segment type
  Elf32_Off   p_offset;   // Segment file offset
  Elf32_Addr  p_vaddr;    // Segment virtual address
  Elf32_Addr  p_paddr;    // Segment physical address
  Elf32_Word  p_filesz;   // Segment size in file
  Elf32_Word  p_memsz;    // Segment size in memory
  Elf32_Word  p_flags;    // Segment flags
  Elf32_Word  p_align;    // Segment alignment
};

struct Elf64_Phdr {
  Elf64_Word  p_type;     // Segment type
  Elf64_Word  p_flags;    // Segment flags
  Elf64_Off   p_offset;   // Segment file offset
  Elf64_Addr  p_vaddr;    // Segment virtual address
  Elf64_Addr  p_paddr;    // Segment physical address
  Elf64_Xword p_filesz;   // Segment size in file
  Elf64_Xword p_memsz;    // Segment size in memory
  Elf64_Xword p_align;    // Segment alignment
};

// Special value for e_phnum.
// This indicates that the real number of program headers is too large to fit into e_phnum.
// Instead the real value is in the field sh_info of section 0.

static constexpr std::size_t PN_XNUM = 0xffff;

// Legal values for p_type (segment type).
enum {
  PT_NULL = 0,                   // Program header table entry unused
  PT_LOAD = 1,                   // Loadable program segment
  PT_DYNAMIC = 2,                // Dynamic linking information
  PT_INTERP = 3,                 // Program interpreter
  PT_NOTE = 4,                   // Auxiliary information
  PT_SHLIB = 5,                  // Reserved
  PT_PHDR = 6,                   // Entry for header table itself
  PT_TLS = 7,                    // Thread-local storage segment
  PT_NUM = 8,                    // Number of defined types
  PT_LOOS = 0x60000000,          // Start of OS-specific
  PT_GNU_EH_FRAME = 0x6474e550,  // GCC .eh_frame_hdr segment
  PT_GNU_STACK = 0x6474e551,     // Indicates stack executability
  PT_GNU_RELRO = 0x6474e552,     // Read-only after relocation
  PT_LOSUNW = 0x6ffffffa,
  PT_SUNWBSS = 0x6ffffffa,       // Sun Specific segment
  PT_SUNWSTACK = 0x6ffffffb,     // Stack segment
  PT_HISUNW = 0x6fffffff,
  PT_HIOS = 0x6fffffff,          // End of OS-specific
  PT_LOPROC = 0x70000000,        // Start of processor-specific
  PT_HIPROC = 0x7fffffff         // End of processor-specific
};

// Legal values for p_flags (segment flags).

#define PF_X (1 << 0)          // Segment is executable
#define PF_W (1 << 1)          // Segment is writable
#define PF_R (1 << 2)          // Segment is readable
#define PF_MASKOS 0x0ff00000   // OS-specific
#define PF_MASKPROC 0xf0000000 // Processor-specific

// Legal values for note segment descriptor types for core files.
enum {
  NT_PRSTATUS = 1,            // Contains copy of prstatus struct
  NT_PRFPREG = 2,             // Contains copy of fpregset struct.
  NT_FPREGSET = 2,            // Contains copy of fpregset struct
  NT_PRPSINFO = 3,            // Contains copy of prpsinfo struct
  NT_PRXREG = 4,              // Contains copy of prxregset struct
  NT_TASKSTRUCT = 4,          // Contains copy of task structure
  NT_PLATFORM = 5,            // String from sysinfo(SI_PLATFORM)
  NT_AUXV = 6,                // Contains copy of auxv array
  NT_GWINDOWS = 7,            // Contains copy of gwindows struct
  NT_ASRS = 8,                // Contains copy of asrset struct
  NT_PSTATUS = 10,            // Contains copy of pstatus struct
  NT_PSINFO = 13,             // Contains copy of psinfo struct
  NT_PRCRED = 14,             // Contains copy of prcred struct
  NT_UTSNAME = 15,            // Contains copy of utsname struct
  NT_LWPSTATUS = 16,          // Contains copy of lwpstatus struct
  NT_LWPSINFO = 17,           // Contains copy of lwpinfo struct
  NT_PRFPXREG = 20,           // Contains copy of fprxregset struct
  NT_SIGINFO = 0x53494749,    // Contains copy of siginfo_t, size might increase
  NT_FILE = 0x46494c45,       // Contains information about mapped files
  NT_PRXFPREG = 0x46e62b7f,   // Contains copy of user_fxsr_struct
  NT_PPC_VMX = 0x100,         // PowerPC Altivec/VMX registers
  NT_PPC_SPE = 0x101,         // PowerPC SPE/EVR registers
  NT_PPC_VSX = 0x102,         // PowerPC VSX registers
  NT_PPC_TAR = 0x103,         // Target Address Register
  NT_PPC_PPR = 0x104,         // Program Priority Register
  NT_PPC_DSCR = 0x105,        // Data Stream Control Register
  NT_PPC_EBB = 0x106,         // Event Based Branch Registers
  NT_PPC_PMU = 0x107,         // Performance Monitor Registers
  NT_PPC_TM_CGPR = 0x108,     // TM checkpointed GPR Registers
  NT_PPC_TM_CFPR = 0x109,     // TM checkpointed FPR Registers
  NT_PPC_TM_CVMX = 0x10a,     // TM checkpointed VMX Registers
  NT_PPC_TM_CVSX = 0x10b,     // TM checkpointed VSX Registers
  NT_PPC_TM_SPR = 0x10c,      // TM Special Purpose Registers
  NT_PPC_TM_CTAR = 0x10d,     // TM checkpointed Target Address Register
  NT_PPC_TM_CPPR = 0x10e,     // TM checkpointed Program Priority Register
  NT_PPC_TM_CDSCR = 0x10f,    // TM checkpointed Data Stream Control Register
  NT_PPC_PKEY = 0x110,        // Memory Protection Keys registers.
  NT_386_TLS = 0x200,         // i386 TLS slots (struct user_desc)
  NT_386_IOPERM = 0x201,      // x86 io permission bitmap (1=deny)
  NT_X86_XSTATE = 0x202,      // x86 extended state using xsave
  NT_S390_HIGH_GPRS = 0x300,  // s390 upper register halves
  NT_S390_TIMER = 0x301,      // s390 timer register
  NT_S390_TODCMP = 0x302,     // s390 TOD clock comparator register
  NT_S390_TODPREG = 0x303,    // s390 TOD programmable register
  NT_S390_CTRS = 0x304,       // s390 control registers
  NT_S390_PREFIX = 0x305,     // s390 prefix register
  NT_S390_LAST_BREAK = 0x306,   // s390 breaking event address
  NT_S390_SYSTEM_CALL = 0x307,  // s390 system call restart data
  NT_S390_TDB = 0x308,          // s390 transaction diagnostic block
  NT_S390_VXRS_LOW = 0x309,     // s390 vector registers 0-15 upper half.
  NT_S390_VXRS_HIGH = 0x30a,    // s390 vector registers 16-31.
  NT_S390_GS_CB = 0x30b,        // s390 guarded storage registers.
  NT_S390_GS_BC = 0x30c,        // s390 guarded storage broadcast control block.
  NT_S390_RI_CB = 0x30d,        // s390 runtime instrumentation.
  NT_ARM_VFP = 0x400,           // ARM VFP/NEON registers
  NT_ARM_TLS = 0x401,           // ARM TLS register
  NT_ARM_HW_BREAK = 0x402,      // ARM hardware breakpoint registers
  NT_ARM_HW_WATCH = 0x403,      // ARM hardware watchpoint registers
  NT_ARM_SYSTEM_CALL = 0x404,   // ARM system call number
  NT_ARM_SVE = 0x405,           // ARM Scalable Vector Extension registers
  NT_ARM_PAC_MASK = 0x406,      // ARM pointer authentication code masks.
  NT_ARM_PACA_KEYS = 0x407,     // ARM pointer authentication address keys.
  NT_ARM_PACG_KEYS = 0x408,     // ARM pointer authentication generic key.
  NT_VMCOREDD = 0x700,          // Vmcore Device Dump Note.
  NT_MIPS_DSP = 0x800,          // MIPS DSP ASE registers.
  NT_MIPS_FP_MODE = 0x801,      // MIPS floating-point mode.
  NT_MIPS_MSA = 0x802,          // MIPS SIMD registers.
};

// Legal values for the note segment descriptor types for object files.
static constexpr std::uint32_t NT_VERSION = 1;  // Contains a version string.

// Dynamic section entry.
struct Elf32_Dyn {
  Elf32_Sword d_tag;    // Dynamic entry type
  union {
    Elf32_Word d_val;   // Integer value
    Elf32_Addr d_ptr;   // Address value
  } d_un;
};

struct Elf64_Dyn {
  Elf64_Sxword d_tag;   // Dynamic entry type
  union {
    Elf64_Xword d_val;  // Integer value
    Elf64_Addr d_ptr;   // Address value
  } d_un;
};

// Legal values for d_tag (dynamic entry type).
enum {
  DT_NULL = 0,              // Marks end of dynamic section
  DT_NEEDED = 1,            // Name of needed library
  DT_PLTRELSZ = 2,          // Size in bytes of PLT relocs
  DT_PLTGOT = 3,            // Processor defined value
  DT_HASH = 4,              // Address of symbol hash table
  DT_STRTAB = 5,            // Address of string table
  DT_SYMTAB = 6,            // Address of symbol table
  DT_RELA = 7,              // Address of Rela relocs
  DT_RELASZ = 8,            // Total size of Rela relocs
  DT_RELAENT = 9,           // Size of one Rela reloc
  DT_STRSZ = 10,            // Size of string table
  DT_SYMENT = 11,           // Size of one symbol table entry
  DT_INIT = 12,             // Address of init function
  DT_FINI = 13,             // Address of termination function
  DT_SONAME = 14,           // Name of shared object
  DT_RPATH = 15,            // Library search path (deprecated)
  DT_SYMBOLIC = 16,         // Start symbol search here
  DT_REL = 17,              // Address of Rel relocs
  DT_RELSZ = 18,            // Total size of Rel relocs
  DT_RELENT = 19,           // Size of one Rel reloc
  DT_PLTREL = 20,           // Type of reloc in PLT
  DT_DEBUG = 21,            // For debugging; unspecified
  DT_TEXTREL = 22,          // Reloc might modify .text
  DT_JMPREL = 23,           // Address of PLT relocs
  DT_BIND_NOW = 24,         // Process relocations of object
  DT_INIT_ARRAY = 25,       // Array with addresses of init fct
  DT_FINI_ARRAY = 26,       // Array with addresses of fini fct
  DT_INIT_ARRAYSZ = 27,     // Size in bytes of DT_INIT_ARRAY
  DT_FINI_ARRAYSZ = 28,     // Size in bytes of DT_FINI_ARRAY
  DT_RUNPATH = 29,          // Library search path
  DT_FLAGS = 30,            // Flags for the object being loaded
  DT_ENCODING = 32,         // Start of encoded range
  DT_PREINIT_ARRAY = 32,    // Array with addresses of preinit fct
  DT_PREINIT_ARRAYSZ = 33,  // size in bytes of DT_PREINIT_ARRAY
  DT_SYMTAB_SHNDX = 34,     // Address of SYMTAB_SHNDX section
  DT_NUM = 35,              // Number used
  DT_LOOS = 0x6000000d,     // Start of OS-specific
  DT_HIOS = 0x6ffff000,     // End of OS-specific
  DT_LOPROC = 0x70000000,   // Start of processor-specific
  DT_HIPROC = 0x7fffffff,   // End of processor-specific
  //DT_PROCNUM = DT_MIPS_NUM, // Most used by any processor
};

// DT_* entries which fall between DT_VALRNGHI & DT_VALRNGLO use the
// Dyn.d_un.d_val field of the Elf*_Dyn structure.  This follows Sun's approach.
static constexpr std::uint32_t DT_VALRNGLO = 0x6ffffd00;
static constexpr std::uint32_t DT_GNU_PRELINKED = 0x6ffffdf5;   // Prelinking timestamp
static constexpr std::uint32_t DT_GNU_CONFLICTSZ = 0x6ffffdf6;  // Size of conflict section
static constexpr std::uint32_t DT_GNU_LIBLISTSZ = 0x6ffffdf7;   // Size of library list
static constexpr std::uint32_t DT_CHECKSUM = 0x6ffffdf8;
static constexpr std::uint32_t DT_PLTPADSZ = 0x6ffffdf9;
static constexpr std::uint32_t DT_MOVEENT = 0x6ffffdfa;
static constexpr std::uint32_t DT_MOVESZ = 0x6ffffdfb;
static constexpr std::uint32_t DT_FEATURE_1 = 0x6ffffdfc;       // Feature selection (DTF_*).
static constexpr std::uint32_t DT_POSFLAG_1 = 0x6ffffdfd;       // Flags for DT_* entries, effecting the following DT_* entry.
static constexpr std::uint32_t DT_SYMINSZ = 0x6ffffdfe;         // Size of syminfo table (in bytes)
static constexpr std::uint32_t DT_SYMINENT = 0x6ffffdff;        // Entry size of syminfo
static constexpr std::uint32_t DT_VALRNGHI = 0x6ffffdff;
#define DT_VALTAGIDX(tag) (DT_VALRNGHI - (tag)) // Reverse order!
static constexpr std::uint32_t DT_VALNUM = 12;

// DT_* entries which fall between DT_ADDRRNGHI & DT_ADDRRNGLO use the Dyn.d_un.d_ptr field of the Elf*_Dyn structure.
// If any adjustment is made to the ELF object after it has been built these entries will need to be adjusted.
static constexpr std::uint32_t DT_ADDRRNGLO = 0x6ffffe00;
static constexpr std::uint32_t DT_GNU_HASH = 0x6ffffef5;        // GNU-style hash table.
static constexpr std::uint32_t DT_TLSDESC_PLT = 0x6ffffef6;
static constexpr std::uint32_t DT_TLSDESC_GOT = 0x6ffffef7;
static constexpr std::uint32_t DT_GNU_CONFLICT = 0x6ffffef8;    // Start of conflict section
static constexpr std::uint32_t DT_GNU_LIBLIST = 0x6ffffef9;     // Library list
static constexpr std::uint32_t DT_CONFIG = 0x6ffffefa;          // Configuration information.
static constexpr std::uint32_t DT_DEPAUDIT = 0x6ffffefb;        // Dependency auditing.
static constexpr std::uint32_t DT_AUDIT = 0x6ffffefc;           // Object auditing.
static constexpr std::uint32_t DT_PLTPAD = 0x6ffffefd;          // PLT padding.
static constexpr std::uint32_t DT_MOVETAB = 0x6ffffefe;         // Move table.
static constexpr std::uint32_t DT_SYMINFO = 0x6ffffeff;         // Syminfo table.
static constexpr std::uint32_t DT_ADDRRNGHI = 0x6ffffeff;
#define DT_ADDRTAGIDX(tag) (DT_ADDRRNGHI - (tag))               // Reverse order!
static constexpr std::uint32_t DT_ADDRNUM = 11;

// The versioning entry types.  The next are defined as part of the GNU extension.
static constexpr std::uint32_t DT_VERSYM = 0x6ffffff0;

static constexpr std::uint32_t DT_RELACOUNT = 0x6ffffff9;
static constexpr std::uint32_t DT_RELCOUNT = 0x6ffffffa;

// These were chosen by Sun.
static constexpr std::uint32_t DT_FLAGS_1 = 0x6ffffffb;         // State flags, see DF_1_* below.
static constexpr std::uint32_t DT_VERDEF = 0x6ffffffc;          // Address of version definition table
static constexpr std::uint32_t DT_VERDEFNUM = 0x6ffffffd;       // Number of version definitions
static constexpr std::uint32_t DT_VERNEED = 0x6ffffffe;         // Address of table with needed versions
static constexpr std::uint32_t DT_VERNEEDNUM = 0x6fffffff;      // Number of needed versions
#define DT_VERSIONTAGIDX(tag) (DT_VERNEEDNUM - (tag))           // Reverse order!
static constexpr std::uint32_t DT_VERSIONTAGNUM = 16;

// Sun added these machine-independent extensions in the "processor-specific" range.  Be compatible.
static constexpr std::uint32_t DT_AUXILIARY = 0x7ffffffd;       // Shared object to load before self
static constexpr std::uint32_t DT_FILTER = 0x7fffffff;          // Shared object to get values from
#define DT_EXTRATAGIDX(tag) ((Elf32_Word) - ((Elf32_Sword)(tag) << 1 >> 1) - 1)
static constexpr std::uint32_t DT_EXTRANUM = 3;

// Values of `d_un.d_val' in the DT_FLAGS entry.
enum {
  DF_ORIGIN = 0x00000001,     // Object may use DF_ORIGIN
  DF_SYMBOLIC = 0x00000002,   // Symbol resolutions starts here
  DF_TEXTREL = 0x00000004,    // Object contains text relocations
  DF_BIND_NOW = 0x00000008,   // No lazy binding for this object
  DF_STATIC_TLS = 0x00000010  // Module uses the static TLS model
};

// State flags selectable in the `d_un.d_val' element of the DT_FLAGS_1 entry in the dynamic section.
enum {
  DF_1_NOW = 0x00000001,         // Set RTLD_NOW for this object.
  DF_1_GLOBAL = 0x00000002,      // Set RTLD_GLOBAL for this object.
  DF_1_GROUP = 0x00000004,       // Set RTLD_GROUP for this object.
  DF_1_NODELETE = 0x00000008,    // Set RTLD_NODELETE for this object.*/
  DF_1_LOADFLTR = 0x00000010,    // Trigger filtee loading at runtime.*/
  DF_1_INITFIRST = 0x00000020,   // Set RTLD_INITFIRST for this object*/
  DF_1_NOOPEN = 0x00000040,      // Set RTLD_NOOPEN for this object.
  DF_1_ORIGIN = 0x00000080,      //$ORIGIN must be handled.
  DF_1_DIRECT = 0x00000100,      // Direct binding enabled.
  DF_1_TRANS = 0x00000200,
  DF_1_INTERPOSE = 0x00000400,   // Object is used to interpose.
  DF_1_NODEFLIB = 0x00000800,    // Ignore default lib search path.
  DF_1_NODUMP = 0x00001000,      // Object can't be dldump'ed.
  DF_1_CONFALT = 0x00002000,     // Configuration alternative created.*/
  DF_1_ENDFILTEE = 0x00004000,   // Filtee terminates filters search.
  DF_1_DISPRELDNE = 0x00008000,  // Disp reloc applied at build time.
  DF_1_DISPRELPND = 0x00010000,  // Disp reloc applied at run-time.
  DF_1_NODIRECT = 0x00020000,    // Object has no-direct binding.
  DF_1_IGNMULDEF = 0x00040000,
  DF_1_NOKSYMS = 0x00080000,
  DF_1_NOHDR = 0x00100000,
  DF_1_EDITED = 0x00200000,      // Object is modified after built.
  DF_1_NORELOC = 0x00400000,
  DF_1_SYMINTPOSE = 0x00800000,  // Object has individual interposers.
  DF_1_GLOBAUDIT = 0x01000000,   // Global auditing required.
  DF_1_SINGLETON = 0x02000000,   // Singleton symbols are used.
  DF_1_STUB = 0x04000000,
  DF_1_PIE = 0x08000000,
  DF_1_KMOD = 0x10000000,
  DF_1_WEAKFILTER = 0x20000000,
  DF_1_NOCOMMON = 0x40000000
};

// Flags for the feature selection in DT_FEATURE_1.
static constexpr std::uint32_t DTF_1_PARINIT = 0x00000001;
static constexpr std::uint32_t DTF_1_CONFEXP = 0x00000002;

// Flags in the DT_POSFLAG_1 entry effecting only the next DT_* entry.
static constexpr std::uint32_t DF_P1_LAZYLOAD = 0x00000001;   // Lazyload following object.
static constexpr std::uint32_t DF_P1_GROUPPERM = 0x00000002;  // Symbols from next object are not generally available.

}  // namespace ELF
}  // namespace binlab

#endif  // !BINLAB_BINARYFORMAT_ELF_H_
