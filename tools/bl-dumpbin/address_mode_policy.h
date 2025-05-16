// address_mode_policy.h

#ifndef BINLAB_ADDRESS_MODE_POLICY_H_
#define BINLAB_ADDRESS_MODE_POLICY_H_

#include "binlab/Config.h"
#include "binlab/BinaryFormat/COFF.h"
#include "binlab/BinaryFormat/ELF.h"

template <typename Section>
struct section_traits;

template <>
struct section_traits<binlab::COFF::IMAGE_SECTION_HEADER> {
  using address_type = decltype(binlab::COFF::IMAGE_SECTION_HEADER::VirtualAddress);

  static inline constexpr auto address(const binlab::COFF::IMAGE_SECTION_HEADER& section) {
    return section.PointerToRawData;
  }
  static inline constexpr auto size(const binlab::COFF::IMAGE_SECTION_HEADER& section) {
    return section.SizeOfRawData;
  }

  static inline constexpr auto vaddress(const binlab::COFF::IMAGE_SECTION_HEADER& section) {
    return section.VirtualAddress;
  }
  static inline constexpr auto vsize(const binlab::COFF::IMAGE_SECTION_HEADER& section) {
    return section.Misc.VirtualSize;
  }
};

template <typename Section, typename Traits = section_traits<Section>>
class file_offset_policy {
 public:
  static constexpr bool in_section(const Section::address_type address, const Section& section) {
    return (Traits::address(section) <= address) && (address < Traits::address(section) + Traits::size(section));
  }

  static constexpr auto cast(const Traits::address_type address, const Section& section) {
    return (address - Traits::address(section) + Traits::vaddress(section));
  }
};

template <typename Section, typename Traits = section_traits<Section>>
class relative_virtual_address_policy {
 public:
  static constexpr bool in_section(const Traits::address_type address, const Section& section) {
    return (Traits::vaddress(section) <= address) && (address < Traits::vaddress(section) + Traits::vsize(section));
  }

  static constexpr auto cast(const Traits::address_type address, const Section& section) {
    return (address - Traits::vaddress(section) + Traits::address(section));
  }
};

#endif  // BINLAB_ADDRESS_MODE_POLICY_H_
