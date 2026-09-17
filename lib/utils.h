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
#include "lib/gcem.h"
#include <algorithm>
#include <cstddef>
#include <string>
#include <vector>

#define NTFX_QUOTE(str) #str
#define NTFX_EXPAND_AND_QUOTE(str) NTFX_QUOTE(str)

#define NTFX_CAT_IMPL(a, b) a##b
#define NTFX_CAT(a, b) NTFX_CAT_IMPL(a, b)

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

template <typename T>
static inline std::vector<T> zeros(size_t n) {
  return std::vector<T>(n, 0.0);
}

inline std::string spacesToUnderscores(std::string x) {
  std::string mangledName = x;
  std::replace(mangledName.begin(), mangledName.end(), ' ', '_');
  return mangledName;
}

inline std::string mangleName(
    std::string paramType, std::string groupName, std::string paramName) {
  return spacesToUnderscores(paramType + ":" + groupName + ":" + paramName);
}

template <typename signal_t>
static inline std::vector<signal_t> logspace(
    signal_t start, signal_t stop, size_t n, signal_t base = 10) {
  std::vector<signal_t> y;
  auto step = (stop - start) / signal_t(n - 1);
  for (size_t i = 0; i < n; i++) {
    y.push_back(gcem::pow(base, start + signal_t(i) * step));
  }
  return y;
}

template <typename signal_t>
static inline std::vector<signal_t> linspace(
    signal_t start, signal_t stop, size_t n) {
  std::vector<signal_t> y;
  auto step = (stop - start) / signal_t(n - 1);
  for (size_t i = 0; i < n; i++) { y.push_back(start + signal_t(i) * step); }
  return y;
}
}
