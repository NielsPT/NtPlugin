#pragma once

/**
 * @file SoftClip.h
 * @author Niels Thøgersen (niels.thoegersen@gmail.com)
 * @brief Soft clippers for audio processing. Calculates coeffs at compile time
 * and applies polynomials to signals.
 *
 * @copyright Copyright (c) 2026
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
 */

#include "lib/Component.h"
#include "lib/Stereo.h"
#include <array>

namespace NtFx {

/**
 * @brief Third order soft clipper wrapped in a Component.
 *
 * @tparam signal_t Audio datatype.
 */
template <typename signal_t>
class SoftClip3 : public Component<Stereo<signal_t>> {
  virtual Stereo<signal_t> process(Stereo<signal_t> x) noexcept override {
    return softClip3rdStereo(x);
  }
};

/**
 * @brief Fifth order soft clipper wrapped in a Component.
 *
 * @tparam signal_t Audio datatype.
 */
template <typename signal_t>
class SoftClip5 : public Component<Stereo<signal_t>> {
  virtual Stereo<signal_t> process(Stereo<signal_t> x) noexcept override {
    return softClip5thStereo(x);
  }
};

/**
 * @brief Calculates coefficients for symmetrical soft clipper
 * at compile time.
 *
 * @tparam signal_t Audio signal type.
 * @tparam N. Determines order. Order = 2 * N + 1.
 * @return consteval array coefficients. Length N + 1;
 */
template <typename signal_t, size_t N>
consteval inline std::array<signal_t, N + 1>
_calculateSoftClipCoeffs() noexcept {
  std::array<signal_t, N + 1> a_n;
  for (int n = 0; n < N + 1; n++) {
    a_n[n] = gcem::pow(-1, n) * gcem::tgamma((2 * N + 1) + 1)
        / (gcem::pow(4, N) * gcem::tgamma(N + 1) * (2 * n + 1)
            * gcem::tgamma(n + 1) * gcem::tgamma(N - n + 1));
  }
  return a_n;
}

template <typename signal_t>
constexpr std::array<signal_t, 4> _coeffsSeventh =
    _calculateSoftClipCoeffs<signal_t, 3>();

template <typename signal_t>
constexpr std::array<signal_t, 3> _coeffsFifth =
    _calculateSoftClipCoeffs<signal_t, 2>();

template <typename signal_t>
constexpr std::array<signal_t, 2> _coeffsThird =
    _calculateSoftClipCoeffs<signal_t, 1>();

/**
 * @brief Applied soft clipping using a fifth order polynomial.
 *
 * @tparam signal_t Audio datatype.
 * @param x Input sample.
 * @return signal_t Output sample.
 */
template <typename signal_t>
static inline signal_t softClip5thMono(signal_t x) noexcept {
  signal_t x_ = x / _coeffsFifth<signal_t>[0];
  if (x_ > 1.0) { return signal_t(1.0); }
  if (x_ < -1.0) { return signal_t(-1.0); }
  auto x3 = x_ * x_ * x_;
  auto x5 = x3 * x_ * x_;
  return x + _coeffsFifth<signal_t>[1] * x3 + _coeffsFifth<signal_t>[2] * x5;
}

/**
 * @brief Applied soft clipping using a fifth order polynomial on a stereo
 * signal.
 *
 * @tparam signal_t Audio datatype.
 * @param x Input sample.
 * @return signal_t Output sample.
 */
template <typename signal_t>
static inline Stereo<signal_t> softClip5thStereo(Stereo<signal_t> x) noexcept {
  return { softClip5thMono(x.l), softClip5thMono(x.r) };
}

/**
 * @brief Applied soft clipping using a third order polynomial.
 *
 * @tparam signal_t Audio datatype.
 * @param x Input sample.
 * @return signal_t Output sample.
 */
template <typename signal_t>
static inline signal_t softClip3rdMono(signal_t x) {
  if (x > 1.0) { return signal_t(1.0); }
  if (x < -1.0) { return signal_t(-1.0); }
  auto x_ = x / _coeffsThird<signal_t>[0];
  auto x3 = x_ * x_ * x_;
  return x - _coeffsThird<signal_t>[1] * x3;
}

/**
 * @brief Applied soft clipping using a third order polynomial on a stereo
 * signal.
 *
 * @tparam signal_t Audio datatype.
 * @param x Input sample.
 * @return signal_t Output sample.
 */
template <typename signal_t>
static inline signal_t softClip3rdStereo(signal_t x) {
  return { softClip3rdMono(x.l), softClip3rdMono(x.r) };
}
} // namespace NtFx
