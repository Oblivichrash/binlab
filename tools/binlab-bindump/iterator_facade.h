// iterator_facade.h:

#ifndef BINLAB_ITERATOR_FACADE_H_
#define BINLAB_ITERATOR_FACADE_H_

#include <cstddef>
#include <memory>
#include <type_traits>

template <typename Derived, typename Value, typename CategoryOrTraversal, typename Reference = Value&, typename Difference = std::ptrdiff_t>
class iterator_facade {
 public:
  using iterator_category = CategoryOrTraversal;

  using value_type        = std::remove_const_t<Value>;
  using reference         = Reference;
  using pointer           = std::add_pointer_t<std::remove_reference_t<Reference>>;
  using difference_type   = Difference;

  // Input iterator interface:
  [[nodiscard]] constexpr reference operator*() const noexcept { return derived().dereference(); }
  [[nodiscard]] constexpr pointer operator->() const noexcept { return std::pointer_traits<pointer>::pointer_to(**this); }

  bool operator==(const Derived& rhs) noexcept { return derived().equals(rhs); }
  bool operator!=(const Derived& rhs) noexcept { return !(*this == rhs); }

  constexpr Derived& operator++() noexcept {
    derived().increment();
    return derived();
  }

  [[nodiscard]] constexpr Derived operator++(int) noexcept {
    Derived tmp{derived()};
    ++*this;
    return tmp;
  }

 protected:
  [[nodiscard]] constexpr Derived& derived() noexcept { return static_cast<Derived&>(*this); }
  [[nodiscard]] constexpr const Derived& derived() const noexcept { return static_cast<const Derived&>(*this); }
};

#endif  // !BINLAB_ITERATOR_FACADE_H_
