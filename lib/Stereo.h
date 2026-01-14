#pragma once

#include "utils.h"

namespace NtFx {

template <typename signal_t>
struct Stereo {
  signal_t l { 0 };
  signal_t r { 0 };
  Stereo<signal_t>() : l(0.0f), r(0.0f) { }
  Stereo<signal_t>(signal_t x) : l(x), r(x) { }
  Stereo<signal_t>(signal_t left, signal_t right) : l(left), r(right) { }
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

  NTFX_INLINE_MEMBER Stereo<signal_t> putInt16(
      const int16_t& xL, const int16_t& xR) noexcept {
    // Scale 1.15 format to float
    this->l = NTFX_SIGNAL(xL * 3.0517578125e-05f);
    this->r = NTFX_SIGNAL(xR * 3.0517578125e-05f);
    return *this;
  }
  NTFX_INLINE_MEMBER int16_t getInt16L() const noexcept {
    return ((this->l >= 1.0f)    ? 0x7FFF
            : (this->l <= -1.0f) ? -0x8000
                                 : this->l * NTFX_SIGNAL(0x7FFF));
  }
  NTFX_INLINE_MEMBER int16_t getInt16R() const noexcept {
    return ((this->r >= 1.0f)    ? 0x7FFF
            : (this->r <= -1.0f) ? -0x8000
                                 : this->r * NTFX_SIGNAL(0x7FFF));
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

} // namespace NtFx
