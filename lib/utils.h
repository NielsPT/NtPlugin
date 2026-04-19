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

/**
 * @brief Converts from linear to dB domain.
 *
 * @tparam signal_t Datatype.
 * @param x Value in linear domain.
 * @return signal_t Value in dB domain.
 */
template <typename signal_t>
static inline signal_t db(signal_t x) noexcept {
  return signal_t(20.0) * gcem::log10(gcem::abs(x));
}

/**
 * @brief Converts from dB to linear domain.
 *
 * @tparam signal_t Datatype.
 * @param x Value in dB domain.
 * @return signal_t Value in linear domain.
 */
template <typename signal_t>
static inline signal_t invDb(signal_t x) noexcept {
  return gcem::pow(signal_t(10.0), x * signal_t(0.05));
}

/**
 * @brief Sets input to 'def' if not a finite number.
 *
 * @tparam signal_t Datatype.
 * @param x Value to validate.
 * @param def Default value to use if 'x' is not valid.
 */
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

template <typename T>
static inline std::vector<T> zeros(size_t n) {
  return std::vector<T>(n, 0.0);
}

/**
 * @brief Saw wave generator.
 *
 * @tparam T Datatype.
 * @param x Input in radians. Same as input for 'sin' function.
 * @return T Output.
 */
template <typename T>
static inline T saw(T x) {
  const T alpha = 2 / GCEM_PI;

  T x_ = gcem::fmod(x, T(2.0) * GCEM_PI);
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
static inline unsigned long _KISS() noexcept {
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

/**
 * @brief Returns a pseudorandom number in the range -1:1.
 *
 * @tparam T Datatype to return.
 * @return T Pseudorandom number.
 */
template <typename T>
static inline T rand() noexcept {
  return (_KISS() - LONG_MAX) / static_cast<T>(LONG_MAX);
}
}
