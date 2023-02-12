// iterator_facade.h:

#ifndef BINLAB_ITERATOR_FACADE_H_
#define BINLAB_ITERATOR_FACADE_H_

#include <cstddef>
#include <memory>
#include <type_traits>

class iterator_facade_access {
 private:
  // Required of all iterators:
  template <typename Facade>
  static typename Facade::reference dereference(const Facade& f) { return f.dereference(); }

  template <typename Facade>
  static void increment(Facade& f) { f.increment(); }

  template <typename Facade>
  static bool equals(const Facade& lhs, const Facade& rhs) { return lhs.equals(rhs); }

  // Required of bidirectional iterators:
  template <typename Facade>
  static void decrement(Facade& f) { f.decrement(); }

  // Required of random-access iterators:
  template <typename Facade>
  static void advance(Facade& f, typename Facade::difference_type n) { f.advance(n); }

  template <typename Facade>
  static typename Facade::difference_type distance_from(const Facade& lhs, const Facade& rhs) { return lhs.distance_to(rhs); }

  // Only iterator_facade can use these definitions.
  template <typename Derived, typename Value, typename CategoryOrTraversal, typename Reference, typename Difference>
  friend class iterator_facade;
};

template <typename Derived, typename Value, typename CategoryOrTraversal, typename Reference = Value&, typename Difference = std::ptrdiff_t>
class iterator_facade {
 public:
  using iterator_category = CategoryOrTraversal;

  using value_type        = typename std::remove_const<Value>::type;
  using reference         = Reference;
  using pointer           = std::add_pointer_t<std::remove_reference_t<Reference>>;
  using difference_type   = Difference;

  // Input iterator interface:
  reference operator*() const noexcept { return iterator_facade_access::dereference(this->derived()); }
  pointer operator->() const noexcept { return std::pointer_traits<pointer>::pointer_to(**this); }

  Derived& operator++() noexcept {
    iterator_facade_access::increment(this->derived());
    return this->derived();
  }

  Derived& operator++(int) noexcept {
    Derived result{derived()};
    ++*this;
    return result;
  }

  bool operator==(const Derived& rhs) noexcept { return iterator_facade_access::equals(this->derived(), rhs); }
  bool operator!=(const Derived& rhs) noexcept { return !(*this == rhs); }

  // Bidirectional iterator interface:
  Derived& operator--() noexcept {
    iterator_facade_access::decrement(this->derived());
    return this->derived();
  }
  Derived operator--(int) noexcept {
    Derived result{derived()};
    --*this;
    return result;
  }

  // Random access iterator interface:
  Derived& operator+=(difference_type n) noexcept {
    iterator_facade_access::advance(this->derived(), n);
    return this->derived();
  }

  Derived operator+(difference_type x) const noexcept {
    Derived result(this->derived());
    result += x;
    return result;
  }

  Derived& operator-=(difference_type n) noexcept {
    return *this += -n;
  }

  Derived operator-(difference_type x) const noexcept {
    Derived result(this->derived());
    result -= x;
    return result;
  }

  reference operator[](difference_type n) const noexcept { return *(*this + n); }

 protected:
  // Curiously Recurring Template interface.
  Derived& derived() { return *static_cast<Derived*>(this); }
  const Derived& derived() const { return *static_cast<const Derived*>(this); }
};

#endif  // !BINLAB_ITERATOR_FACADE_H_
