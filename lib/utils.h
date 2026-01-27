#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <vector>

namespace NtFx {

template <typename signal_t>
static inline signal_t db(signal_t x) noexcept {
  return static_cast<signal_t>(20.0) * std::log10(x);
}

template <typename signal_t>
static inline signal_t invDb(signal_t x) noexcept {
  return std::pow(static_cast<signal_t>(10.0), x * static_cast<signal_t>(0.05));
}

template <typename signal_t>
static inline signal_t ensureFinite(
    signal_t x, signal_t def = static_cast<signal_t>(0)) noexcept {
  signal_t y = def;
  if (x == x) { y = x; }
  return y;
}

constexpr int rmsDelayLineLength = 16384;
template <typename signal_t>
struct RmsSensorState {
  std::array<signal_t, rmsDelayLineLength> delayLine;
  signal_t accum = static_cast<signal_t>(0.0);
  int i          = 0;
  void reset() noexcept {
    std::fill(delayLine.begin(), delayLine.end(), static_cast<signal_t>(0.0));
    accum = static_cast<signal_t>(0.0);
  }
};

template <typename signal_t>
static inline signal_t rmsSensor(
    RmsSensorState<signal_t>* p_state, size_t nRms, signal_t x) noexcept {
  if (nRms > rmsDelayLineLength) { return static_cast<signal_t>(0.0); }
  signal_t _x = x * x;
  if (_x != _x) { _x = static_cast<signal_t>(0.0); }
  p_state->accum += _x - p_state->delayLine[p_state->i];
  p_state->delayLine[p_state->i] = _x;
  p_state->i++;
  if (p_state->i >= nRms) { p_state->i = 0; }
  signal_t y = std::sqrt(2.0 * p_state->accum / nRms);
  if (y != y) { y = static_cast<signal_t>(0.0); }
  return y;
}

template <typename T>
static inline std::vector<T> zeros(size_t n) {
  return std::vector<T>(n, 0.0);
}

template <typename T>
static inline T saw(T x) {
  const T alpha = 2 / M_PI;

  T x_ = std::fmod(x, static_cast<T>(2.0) * M_PI);
  x_   = (x_ < 0 ? x_ + 2 * M_PI : x_);
  T y;
  if (x_ < 0.5 * M_PI) {
    y = x_ * alpha;
  } else if (x_ < 1.5 * M_PI) {
    y = -x_ * alpha + 2;
  } else {
    y = x_ * alpha - 4;
  }
  return y;
}
}
