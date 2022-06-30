// tools/pedump/main.cpp: Dump Binary files

#include <fstream>
#include <iomanip>
#include <iostream>
#include <span>
#include <type_traits>
#include <vector>

#include "binlab/Config/Config.h"

template <typename Rep>
class address;

template <typename T>
class segments {
 public:
  template <typename InputIt>
  segments(InputIt first, InputIt last);

 private:
  template <typename Rep>
  friend class address;
};

template <typename Rep = std::size_t>
class address {
 public:
  constexpr address() = delete;

  constexpr explicit address(const void* base) noexcept : value_{reinterpret_cast<Rep>(base)} {}

  template <typename Rep2, std::enable_if_t<std::is_convertible_v<const Rep2&, Rep>, int> = 0>
  constexpr explicit address(const Rep2& value) noexcept : value_{value} {}

  //constexpr explicit operator Rep() const noexcept { return value_; }

  constexpr auto operator<=>(const address&) const noexcept = default;

  //template <typename Type, std::enable_if_t<std::is_pointer_v<Type> || std::is_reference_v<Type>, int> = 0>
  //Type object_cast() noexcept { return reinterpret_cast<Type>(value_); }

  address& operator+=(const address& rhs) noexcept {
    value_ += rhs.value_;
    return *this;
  }

 private:
  Rep value_;
};

template <typename Rep = std::size_t>
class virtual_address : public address<Rep> {
 public:
  template <typename Rep2, std::enable_if_t<std::is_convertible_v<const Rep2&, Rep>, int> = 0>
  constexpr explicit virtual_address(const Rep2& value) noexcept : address<Rep>{value} {}
};

template <typename Rep = std::size_t>
class virtual_offset : public address<Rep> {
 public:
  template <typename Rep2, std::enable_if_t<std::is_convertible_v<const Rep2&, Rep>, int> = 0>
  constexpr explicit virtual_offset(const Rep2& value) noexcept : address<Rep>{value} {}
};

template <typename Rep = std::size_t>
class file_address : public address<Rep> {
 public:
  template <typename Rep2, std::enable_if_t<std::is_convertible_v<const Rep2&, Rep>, int> = 0>
  constexpr explicit file_address(const Rep2& value) noexcept : address<Rep>{value} {}
};

template <typename Rep = std::size_t>
class file_offset : public address<Rep> {
 public:
  template <typename Rep2, std::enable_if_t<std::is_convertible_v<const Rep2&, Rep>, int> = 0>
  constexpr explicit file_offset(const Rep2& value) noexcept : address<Rep>{value} {}
};

std::ostream& memdump(std::ostream& os, void* base, std::size_t rows) {
  auto buff = static_cast<std::uint8_t*>(base);
  for (std::size_t i = 0; i < rows; ++i, buff += 16) {
    for (std::size_t j = 0; j < 16; ++j) {
      os << ' ' << std::setw(2) << std::setfill('0') << unsigned{buff[j]};
    }
    os << '\n';
  }
  return os;
}

int main(int argc, char* argv[]) try {
  if (argc < 2) {
    std::cerr << argv[0] << " Version " << BINLAB_VERSION_MAJOR << '.' << BINLAB_VERSION_MINOR << '\n';
    std::cout << "Usage: " << argv[0] << " <file>\n";
    return 1;
  }

  if (std::ifstream is{argv[1], std::ios::binary | std::ios::ate}) {
    const auto size = is.tellg();
    std::vector<char> buff(size);
    if (is.seekg(0).read(&buff[0], size)) {
      std::cout << std::hex;
      memdump(std::cout, buff.data(), 16);
    } else {
      std::cerr << "read failed\n";
    }
  } else {
    std::cerr << "open failed\n";
  }

  return 0;
} catch (const std::exception& e) {
  std::cerr << "error: " << e.what() << '\n';
  return 1;
}
