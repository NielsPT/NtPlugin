#pragma once

#include <cmath>
#include <cstddef>

namespace NtFx {

template <typename signal_t>
struct Stereo {
  signal_t l { 0 };
  signal_t r { 0 };
  Stereo() : l(0.0f), r(0.0f) { }
  Stereo(signal_t x) : l(x), r(x) { }
  Stereo(signal_t left, signal_t right) : l(left), r(right) { }
  Stereo<signal_t>& operator=(const Stereo<signal_t>& x) noexcept {
    this->l = x.l;
    this->r = x.r;
    return *this;
  }
  bool operator==(const Stereo<signal_t>& x) const noexcept {
    return (this->l == x.l) && (this->r == x.r);
  }
  bool operator<(const Stereo<signal_t>& x) const noexcept {
    return this->absMin() < x.absMin();
  }
  bool operator>(const Stereo<signal_t>& x) const noexcept {
    return this->absMax() > x.absMax();
  }
  bool operator<=(const Stereo<signal_t>& x) const noexcept {
    return this->absMin() <= x.absMin();
  }
  bool operator>=(const Stereo<signal_t>& x) const noexcept {
    return this->absMax() >= x.absMax();
  }
  bool operator<(const signal_t& x) const noexcept {
    return this->absMin() < x;
  }
  bool operator>(const signal_t& x) const noexcept {
    return this->absMax() > x;
  }
  bool operator<=(const signal_t& x) const noexcept {
    return this->absMin() <= x;
  }
  bool operator>=(const signal_t& x) const noexcept {
    return this->absMax() >= x;
  }

  Stereo<signal_t>& operator*=(const signal_t x) noexcept {
    this->l = this->l * x;
    this->r = this->r * x;
    return *this;
  }
  Stereo<signal_t>& operator*=(const Stereo<signal_t> x) noexcept {
    this->l = this->l * x.l;
    this->r = this->r * x.r;
    return *this;
  }
  Stereo<signal_t>& operator+=(const signal_t x) noexcept {
    this->l = this->l + x;
    this->r = this->r + x;
    return *this;
  }
  Stereo<signal_t>& operator+=(const Stereo<signal_t> x) noexcept {
    this->l = this->l + x.l;
    this->r = this->r + x.r;
    return *this;
  }
  signal_t avgSquared() const noexcept {
    return (this->l * this->l + this->r * this->r) * 0.5f;
  }
  signal_t absMax() const noexcept {
    return std::abs(this->l) > std::abs(this->r) ? this->l : this->r;
  }
  signal_t absMin() const noexcept {
    return std::abs(this->l) < std::abs(this->r) ? this->l : this->r;
  }
};
template <typename signal_t>
static inline Stereo<signal_t> operator+(
    const Stereo<signal_t>& y, const signal_t& x) noexcept {
  return { (y.l + x), (y.r + x) };
}
template <typename signal_t>
static inline Stereo<signal_t> operator-(
    const Stereo<signal_t>& y, const signal_t& x) noexcept {
  return { (y.l - x), (y.r - x) };
}
template <typename signal_t>
static inline Stereo<signal_t> operator*(
    const Stereo<signal_t>& y, const signal_t& x) noexcept {
  return { (y.l * x), (y.r * x) };
}
template <typename signal_t>
static inline Stereo<signal_t> operator/(
    const Stereo<signal_t>& y, const signal_t& x) noexcept {
  return { (y.l / x), (y.r / x) };
}
template <typename signal_t>
static inline Stereo<signal_t> operator+(
    const Stereo<signal_t>& y, const Stereo<signal_t>& x) noexcept {
  return { (y.l + x.l), (y.r + x.r) };
}
template <typename signal_t>
static inline Stereo<signal_t> operator-(
    const Stereo<signal_t>& y, const Stereo<signal_t>& x) noexcept {
  return { (y.l - x.l), (y.r - x.r) };
}
template <typename signal_t>
static inline Stereo<signal_t> operator*(
    const Stereo<signal_t>& y, const Stereo<signal_t>& x) noexcept {
  return { (y.l * x.l), (y.r * x.r) };
}
template <typename signal_t>
static inline Stereo<signal_t> operator/(
    const Stereo<signal_t>& y, const Stereo<signal_t>& x) noexcept {
  return { (y.l / x.l), (y.r / x.r) };
}
template <typename signal_t>
static inline Stereo<signal_t> operator+(
    const signal_t& x, const Stereo<signal_t>& y) noexcept {
  return { (x + y.l), (x + y.r) };
}
template <typename signal_t>
static inline Stereo<signal_t> operator-(
    const signal_t& x, const Stereo<signal_t>& y) noexcept {
  return { (x - y.l), (x - y.r) };
}
template <typename signal_t>
static inline Stereo<signal_t> operator*(
    const signal_t& x, const Stereo<signal_t>& y) noexcept {
  return { (x * y.l), (x * y.r) };
}
template <typename signal_t>
static inline Stereo<signal_t> operator/(
    const signal_t& x, const Stereo<signal_t>& y) noexcept {
  return { (x / y.l), (x / y.r) };
}
template <typename signal_t>
static inline Stereo<signal_t> operator*(
    const Stereo<signal_t>& y, const size_t& x) noexcept {
  return { (y.l * static_cast<signal_t>(x)), (y.r * static_cast<signal_t>(x)) };
}
template <typename signal_t>
static inline Stereo<signal_t> operator*(
    const size_t x, const Stereo<signal_t>& y) noexcept {
  return { (y.l * static_cast<signal_t>(x)), (y.r * static_cast<signal_t>(x)) };
}
template <typename signal_t>
static inline Stereo<signal_t> ensureFinite(
    Stereo<signal_t> x, signal_t def = static_cast<signal_t>(0)) noexcept {
  Stereo<signal_t> y(def, def);
  if (x.l == x.l) { y.l = x.l; };
  if (x.r == x.r) { y.r = x.r; }
  return y;
}
} // namespace NtFx
