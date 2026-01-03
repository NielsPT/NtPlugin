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
