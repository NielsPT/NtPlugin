#pragma once

#include "lib/Stereo.h"
#include <array>

namespace NtFx {
template <typename signal_t>
static inline signal_t softClip5thMono(
    std::array<signal_t, 3>& p_coeffs, signal_t x) noexcept {
  signal_t x_ = x / p_coeffs[0];
  if (x_ > 1.0) { return static_cast<signal_t>(1.0); }
  if (x_ < -1.0) { return static_cast<signal_t>(-1.0); }
  return p_coeffs[0] * x_ + p_coeffs[1] * x_ * x_ * x_
      + p_coeffs[2] * x_ * x_ * x_ * x_ * x_;
}

template <typename signal_t>
static inline Stereo<signal_t> softClip5thStereo(
    std::array<signal_t, 3>& p_coeffs, Stereo<signal_t> x) noexcept {
  return { softClip5thMono(p_coeffs, x.l), softClip5thMono(p_coeffs, x.r) };
}

template <typename signal_t>
static inline signal_t softClip3rd(signal_t x) {
  if (x > 1.0) { return static_cast<signal_t>(1.0); }
  if (x < -1.0) { return static_cast<signal_t>(-1.0); }
  auto x_ = x / 1.5;
  return x - 0.5 * x_ * x_ * x_;
}

template <typename signal_t>
static inline signal_t softClip3rdStereo(signal_t x) {
  return { softClip3rd(x.l), softClip3rd(x.r) };
}

template <typename signal_t, size_t N>
constexpr inline static std::array<signal_t, N + 1>
calculateSoftClipCoeffs() noexcept {
  // order = 2 * N + 1
  std::array<signal_t, N + 1> a_n;
  for (int n = 0; n < N + 1; n++) {
    a_n[n] = (std::pow(-1, n) * std::tgamma((2 * N + 1) + 1)
        / (std::pow(4, N) * std::tgamma(N + 1) * (2 * n + 1)
            * std::tgamma(n + 1) * std::tgamma(N - n + 1)));
  }
  return a_n;
}

} // namespace NtFx
