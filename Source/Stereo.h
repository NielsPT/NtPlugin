#pragma once

#define NT_INLINE_TEMPLATE                                                             \
  template <typename T>                                                                \
  __attribute__((always_inline)) static inline
#define NTFX_INLINE_MEMBER __attribute__((always_inline)) inline
#define NTFX_INLINE_STATIC __attribute__((always_inline)) inline static

template <typename T>
struct Stereo {
  T l { 0 };
  T r { 0 };
  Stereo<T>() : l(0.0f), r(0.0f) { }
  Stereo<T>(float x) : l(x), r(x) { }
  Stereo<T>(float left, float right) : l(left), r(right) { }
  NTFX_INLINE_MEMBER Stereo<T>& operator=(const Stereo<T>& x) {
    this->l = x.l;
    this->r = x.r;
    return *this;
  }
  NTFX_INLINE_MEMBER bool operator==(const Stereo<T>& x) const {
    return (this->l == x.l) && (this->r == x.r);
  }
  NTFX_INLINE_MEMBER bool operator<(const Stereo<T>& x) const {
    return (this->l < x.l) && (this->r < x.r);
  }
  NTFX_INLINE_MEMBER bool operator>(const Stereo<T>& x) const {
    return (this->l > x.l) && (this->r > x.r);
  }
  NTFX_INLINE_MEMBER bool operator<=(const Stereo<T>& x) const {
    return (this->l <= x.l) && (this->r <= x.r);
  }
  NTFX_INLINE_MEMBER bool operator>=(const Stereo<T>& x) const {
    return (this->l >= x.l) && (this->r >= x.r);
  }

  NTFX_INLINE_MEMBER Stereo<T>& operator*=(const float x) {
    this->l = this->l * x;
    this->r = this->r * x;
    return *this;
  }
  NTFX_INLINE_MEMBER Stereo<T>& operator*=(const Stereo<T> x) {
    this->l = this->l * x.l;
    this->r = this->r * x.r;
    return *this;
  }

  NTFX_INLINE_MEMBER Stereo<T> putInt16(const int16_t& xL, const int16_t& xR) {
    // Scale 1.15 format to float
    this->l = static_cast<T>(xL * 3.0517578125e-05f);
    this->r = static_cast<T>(xR * 3.0517578125e-05f);
    return *this;
  }
  NTFX_INLINE_MEMBER int16_t getInt16L() const {
    return ((this->l >= 1.0f)    ? 0x7FFF
            : (this->l <= -1.0f) ? -0x8000
                                 : this->l * static_cast<T>(0x7FFF));
  }
  NTFX_INLINE_MEMBER int16_t getInt16R() const {
    return ((this->r >= 1.0f)    ? 0x7FFF
            : (this->r <= -1.0f) ? -0x8000
                                 : this->r * static_cast<T>(0x7FFF));
  }
  NTFX_INLINE_MEMBER float avgSquared() const {
    return (this->l * this->l + this->r * this->r) * 0.5f;
  }
  NTFX_INLINE_MEMBER T absMax() noexcept {
    return std::abs(this->l) > std::abs(this->r) ? this->l : this->r;
  }
};
NT_INLINE_TEMPLATE Stereo<T> operator+(const Stereo<T>& y, const T& x) noexcept {
  return { (y.l + x), (y.r + x) };
}
NT_INLINE_TEMPLATE Stereo<T> operator-(const Stereo<T>& y, const T& x) noexcept {
  return { (y.l - x), (y.r - x) };
}
NT_INLINE_TEMPLATE Stereo<T> operator*(const Stereo<T>& y, const T& x) noexcept {
  return { (y.l * x), (y.r * x) };
}
NT_INLINE_TEMPLATE Stereo<T> operator/(const Stereo<T>& y, const T& x) noexcept {
  return { (y.l / x), (y.r / x) };
}
NT_INLINE_TEMPLATE Stereo<T> operator+(
    const Stereo<T>& y, const Stereo<T>& x) noexcept {
  return { (y.l + x.l), (y.r + x.r) };
}
NT_INLINE_TEMPLATE Stereo<T> operator-(
    const Stereo<T>& y, const Stereo<T>& x) noexcept {
  return { (y.l - x.l), (y.r - x.r) };
}
NT_INLINE_TEMPLATE Stereo<T> operator*(
    const Stereo<T>& y, const Stereo<T>& x) noexcept {
  return { (y.l * x.l), (y.r * x.r) };
}
NT_INLINE_TEMPLATE Stereo<T> operator/(
    const Stereo<T>& y, const Stereo<T>& x) noexcept {
  return { (y.l / x.l), (y.r / x.r) };
}
NT_INLINE_TEMPLATE Stereo<T> operator+(const T& x, const Stereo<T>& y) noexcept {
  return { (x + y.l), (x + y.r) };
}
NT_INLINE_TEMPLATE Stereo<T> operator-(const T& x, const Stereo<T>& y) noexcept {
  return { (x - y.l), (x - y.r) };
}
NT_INLINE_TEMPLATE Stereo<T> operator*(const T& x, const Stereo<T>& y) noexcept {
  return { (x * y.l), (x * y.r) };
}
NT_INLINE_TEMPLATE Stereo<T> operator/(const T& x, const Stereo<T>& y) noexcept {
  return { (x / y.l), (x / y.r) };
}
