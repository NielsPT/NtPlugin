/*
 * Copyright (C) 2026 Niels Thøgersen, NTlyd
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Affero General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option) any
 * later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 **/

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
