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
#include "gcem.hpp"
#include <climits>
#include <vector>

namespace NtFx {

template <typename signal_t>
static inline signal_t db(signal_t x) noexcept {
  return signal_t(20.0) * std::log10(x);
}

template <typename signal_t>
static inline signal_t invDb(signal_t x) noexcept {
  return gcem::pow(signal_t(10.0), x * signal_t(0.05));
}

template <typename signal_t>
static inline void ensureFinite(
    signal_t& x, signal_t def = signal_t(0)) noexcept {
  signal_t y = def;
  if (x == x) {
    ;
  } else {
    x = y;
  }
}

// constexpr int rmsDelayLineLength = 16384;
// template <typename signal_t>
// struct RmsSensorState {
//   std::array<signal_t, rmsDelayLineLength> delayLine;
//   signal_t accum = signal_t(0.0);
//   int i          = 0;
//   void reset() noexcept {
//     std::fill(delayLine.begin(), delayLine.end(), signal_t(0.0));
//     accum = signal_t(0.0);
//   }
// };

// template <typename signal_t>
// static inline signal_t rmsSensor(
//     RmsSensorState<signal_t>* p_state, size_t nRms, signal_t x) noexcept {
//   if (nRms > rmsDelayLineLength) { return signal_t(0.0); }
//   signal_t _x = x * x;
//   if (_x != _x) { _x = signal_t(0.0); }
//   p_state->accum += _x - p_state->delayLine[p_state->i];
//   p_state->delayLine[p_state->i] = _x;
//   p_state->i++;
//   if (p_state->i >= nRms) { p_state->i = 0; }
//   signal_t y = gcem::sqrt(2.0 * p_state->accum / nRms);
//   if (y != y) { y = signal_t(0.0); }
//   return y;
// }

// template <typename signal_t>
// static inline signal_t peakSensor(
//     signal_t alpha, signal_t& p_state, signal_t x) {
//   auto xAbs            = gcem::abs(x);
//   signal_t sensRelease = alpha * p_state + (1 - alpha) * xAbs;
//   signal_t ySens       = gcem::max(xAbs, sensRelease);
//   ensureFinite(ySens);
//   p_state = ySens;
//   return ySens;
// }

template <typename T>
static inline std::vector<T> zeros(size_t n) {
  return std::vector<T>(n, 0.0);
}

template <typename T>
static inline T saw(T x) {
  const T alpha = 2 / GCEM_PI;

  T x_ = std::fmod(x, static_cast<T>(2.0) * GCEM_PI);
  x_   = (x_ < 0 ? x_ + 2 * GCEM_PI : x_);
  T y;
  if (x_ < 0.5 * GCEM_PI) {
    y = x_ * alpha;
  } else if (x_ < 1.5 * GCEM_PI) {
    y = -x_ * alpha + 2;
  } else {
    y = x_ * alpha - 4;
  }
  return y;
}

/**
 * @brief Pseudorandom unsigned long.
 *
 * Marsaglia, George (2003) "Random Number Generators,"Journal of Modern Applied
 * Statistical Methods: Vol. 2 : Iss. 1 , Article 2.
 * DOI: 10.22237/jmasm/1051747320
 *
 * @return unsigned long
 */
static inline unsigned long KISS() noexcept {
  static unsigned long x = 123456789, y = 362436000, z = 521288629, c = 7654321;
  unsigned long long t;
  x = 69069 * x + 12345;
  y ^= y << 13;
  y ^= y >> 17;
  y ^= y << 5;
  t = 698769069LL * z + c;
  c = t >> 32;
  return x + y + (z = t);
}

template <typename T>
static inline T rand() noexcept {
  return (KISS() - LONG_MAX) / static_cast<T>(LONG_MAX);
}
}
