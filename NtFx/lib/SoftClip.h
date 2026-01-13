#pragma once

#include "Stereo.h"
#include "utils.h"

namespace NtFx {
NTFX_INLINE_TEMPLATE signal_t softClip5thMono(
    std::array<signal_t, 3>& p_coeffs, signal_t x) noexcept {
  signal_t x_ = x / p_coeffs[0];
  if (x_ > 1.0) { return NTFX_SIGNAL(1.0); }
  if (x_ < -1.0) { return NTFX_SIGNAL(-1.0); }
  return p_coeffs[0] * x_
      + p_coeffs[1] * x_ * x_ * x_
      + p_coeffs[2] * x_ * x_ * x_ * x_ * x_;
}

NTFX_INLINE_TEMPLATE Stereo<signal_t> softClip5thStereo(
    std::array<signal_t, 3>& p_coeffs, Stereo<signal_t> x) noexcept {
  return { softClip5thMono(p_coeffs, x.l), softClip5thMono(p_coeffs, x.r) };
}

template <typename signal_t, size_t N>
NTFX_INLINE_STATIC std::array<signal_t, N + 1> calculateSoftClipCoeffs() noexcept {
  // order = 2 * N + 1
  std::array<signal_t, N + 1> a_n;
  for (int n = 0; n < N + 1; n++) {
    a_n[n] = (std::pow(-1, n)
        * std::tgamma((2 * N + 1) + 1)
        / (std::pow(4, N)
            * std::tgamma(N + 1)
            * (2 * n + 1)
            * std::tgamma(n + 1)
            * std::tgamma(N - n + 1)));
  }
  return a_n;
}

} // namespace NtFx
