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

// TODO: Document this file.

#pragma once

#include "lib/Stereo.h"
#include <array>

namespace NtFx {

/**
 * @brief Calculates coefficients for symmetrical soft clipper at compile time.
 *
 * @tparam signal_t Audio signal type.
 * @tparam N. Determines order. Order = 2 * N + 1.
 * @return consteval array coefficients. Length N + 1;
 */
template <typename signal_t, size_t N>
consteval inline std::array<signal_t, N + 1>
calculateSoftClipCoeffs() noexcept {
  std::array<signal_t, N + 1> a_n;
  for (int n = 0; n < N + 1; n++) {
    a_n[n] = gcem::pow(-1, n) * gcem::tgamma((2 * N + 1) + 1)
        / (gcem::pow(4, N) * gcem::tgamma(N + 1) * (2 * n + 1)
            * gcem::tgamma(n + 1) * gcem::tgamma(N - n + 1));
  }
  return a_n;
}

// template <typename signal_t, int N>
// consteval inline std::array<signal_t, N + 1>
// calculateSoftClipCoeffs() noexcept {
//   std::array<signal_t, N + 1> a_n;
//   for (int n = 0; n < N + 1; n++) {
//     a_n[n] = gcem::pow(signal_t(-1), n) * NtFx::factorial(2 * N + 1)
//         / (gcem::pow(signal_t(4), N) * NtFx::factorial(N)
//             * (signal_t(2) * n + signal_t(1)) * NtFx::factorial(n)
//             * NtFx::factorial(N - n));
//   }
//   return a_n;
// }

template <typename signal_t>
constexpr std::array<signal_t, 4> coeffsSeventh =
    calculateSoftClipCoeffs<signal_t, 3>();

template <typename signal_t>
constexpr std::array<signal_t, 3> coeffsFifth =
    calculateSoftClipCoeffs<signal_t, 2>();

template <typename signal_t>
constexpr std::array<signal_t, 2> coeffsThird =
    calculateSoftClipCoeffs<signal_t, 1>();

template <typename signal_t>
static inline signal_t softClip5thMono(signal_t x) noexcept {
  signal_t x_ = x / coeffsFifth<signal_t>[0];
  if (x_ > 1.0) { return signal_t(1.0); }
  if (x_ < -1.0) { return signal_t(-1.0); }
  auto x3 = x_ * x_ * x_;
  auto x5 = x3 * x_ * x_;
  return x + coeffsFifth<signal_t>[1] * x3 + coeffsFifth<signal_t>[2] * x5;
}

template <typename signal_t>
static inline Stereo<signal_t> softClip5thStereo(Stereo<signal_t> x) noexcept {
  return { softClip5thMono(x.l), softClip5thMono(x.r) };
}

template <typename signal_t>
static inline signal_t softClip3rdMono(signal_t x) {
  if (x > 1.0) { return signal_t(1.0); }
  if (x < -1.0) { return signal_t(-1.0); }
  auto x_ = x / coeffsThird<signal_t>[0];
  auto x3 = x_ * x_ * x_;
  return x - coeffsThird<signal_t>[1] * x3;
}

template <typename signal_t>
static inline signal_t softClip3rdStereo(signal_t x) {
  return { softClip3rdMono(x.l), softClip3rdMono(x.r) };
}
} // namespace NtFx
