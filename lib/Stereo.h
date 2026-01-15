#pragma once

#include "lib/utils.h"

namespace NtFx {

template <typename signal_t>
struct Stereo {
  signal_t l { 0 };
  signal_t r { 0 };
  Stereo() : l(0.0f), r(0.0f) { }
  Stereo(signal_t x) : l(x), r(x) { }
  Stereo(signal_t left, signal_t right) : l(left), r(right) { }
  NTFX_INLINE_MEMBER Stereo<signal_t>& operator=(const Stereo<signal_t>& x) noexcept {
    this->l = x.l;
    this->r = x.r;
    return *this;
  }
  NTFX_INLINE_MEMBER bool operator==(const Stereo<signal_t>& x) const noexcept {
    return (this->l == x.l) && (this->r == x.r);
  }
  NTFX_INLINE_MEMBER bool operator<(const Stereo<signal_t>& x) const noexcept {
    return this->absMin() < x.absMin();
  }
  NTFX_INLINE_MEMBER bool operator>(const Stereo<signal_t>& x) const noexcept {
    return this->absMax() > x.absMax();
  }
  NTFX_INLINE_MEMBER bool operator<=(const Stereo<signal_t>& x) const noexcept {
    return this->absMin() <= x.absMin();
  }
  NTFX_INLINE_MEMBER bool operator>=(const Stereo<signal_t>& x) const noexcept {
    return this->absMax() >= x.absMax();
  }
  NTFX_INLINE_MEMBER bool operator<(const signal_t& x) const noexcept {
    return this->absMin() < x;
  }
  NTFX_INLINE_MEMBER bool operator>(const signal_t& x) const noexcept {
    return this->absMax() > x;
  }
  NTFX_INLINE_MEMBER bool operator<=(const signal_t& x) const noexcept {
    return this->absMin() <= x;
  }
  NTFX_INLINE_MEMBER bool operator>=(const signal_t& x) const noexcept {
    return this->absMax() >= x;
  }

  NTFX_INLINE_MEMBER Stereo<signal_t>& operator*=(const signal_t x) noexcept {
    this->l = this->l * x;
    this->r = this->r * x;
    return *this;
  }
  NTFX_INLINE_MEMBER Stereo<signal_t>& operator*=(const Stereo<signal_t> x) noexcept {
    this->l = this->l * x.l;
    this->r = this->r * x.r;
    return *this;
  }
  NTFX_INLINE_MEMBER signal_t avgSquared() const noexcept {
    return (this->l * this->l + this->r * this->r) * 0.5f;
  }
  NTFX_INLINE_MEMBER signal_t absMax() const noexcept {
    return std::abs(this->l) > std::abs(this->r) ? this->l : this->r;
  }
  NTFX_INLINE_MEMBER signal_t absMin() const noexcept {
    return std::abs(this->l) < std::abs(this->r) ? this->l : this->r;
  }
};
NTFX_INLINE_TEMPLATE Stereo<signal_t> operator+(
    const Stereo<signal_t>& y, const signal_t& x) noexcept {
  return { (y.l + x), (y.r + x) };
}
NTFX_INLINE_TEMPLATE Stereo<signal_t> operator-(
    const Stereo<signal_t>& y, const signal_t& x) noexcept {
  return { (y.l - x), (y.r - x) };
}
NTFX_INLINE_TEMPLATE Stereo<signal_t> operator*(
    const Stereo<signal_t>& y, const signal_t& x) noexcept {
  return { (y.l * x), (y.r * x) };
}
NTFX_INLINE_TEMPLATE Stereo<signal_t> operator/(
    const Stereo<signal_t>& y, const signal_t& x) noexcept {
  return { (y.l / x), (y.r / x) };
}
NTFX_INLINE_TEMPLATE Stereo<signal_t> operator+(
    const Stereo<signal_t>& y, const Stereo<signal_t>& x) noexcept {
  return { (y.l + x.l), (y.r + x.r) };
}
NTFX_INLINE_TEMPLATE Stereo<signal_t> operator-(
    const Stereo<signal_t>& y, const Stereo<signal_t>& x) noexcept {
  return { (y.l - x.l), (y.r - x.r) };
}
NTFX_INLINE_TEMPLATE Stereo<signal_t> operator*(
    const Stereo<signal_t>& y, const Stereo<signal_t>& x) noexcept {
  return { (y.l * x.l), (y.r * x.r) };
}
NTFX_INLINE_TEMPLATE Stereo<signal_t> operator/(
    const Stereo<signal_t>& y, const Stereo<signal_t>& x) noexcept {
  return { (y.l / x.l), (y.r / x.r) };
}
NTFX_INLINE_TEMPLATE Stereo<signal_t> operator+(
    const signal_t& x, const Stereo<signal_t>& y) noexcept {
  return { (x + y.l), (x + y.r) };
}
NTFX_INLINE_TEMPLATE Stereo<signal_t> operator-(
    const signal_t& x, const Stereo<signal_t>& y) noexcept {
  return { (x - y.l), (x - y.r) };
}
NTFX_INLINE_TEMPLATE Stereo<signal_t> operator*(
    const signal_t& x, const Stereo<signal_t>& y) noexcept {
  return { (x * y.l), (x * y.r) };
}
NTFX_INLINE_TEMPLATE Stereo<signal_t> operator/(
    const signal_t& x, const Stereo<signal_t>& y) noexcept {
  return { (x / y.l), (x / y.r) };
}
NTFX_INLINE_TEMPLATE Stereo<signal_t> ensureFinite(
    Stereo<signal_t> x, signal_t def = NTFX_SIG_T(0)) noexcept {
  return Stereo<signal_t>((x.l == x.l ? x.l : def), (x.r == x.r ? x.r : def));
}

} // namespace NtFx
