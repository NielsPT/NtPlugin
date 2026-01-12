#pragma once
#include <algorithm>
#include <array>

#define ALWAYS_INLINE __attribute__((always_inline))
// #define ALWAYS_INLINE
#define NTFX_INLINE_TEMPLATE                                                           \
  template <typename signal_t>                                                         \
  ALWAYS_INLINE static inline
#define NTFX_INLINE_MEMBER ALWAYS_INLINE inline
#define NTFX_INLINE_STATIC ALWAYS_INLINE inline static
#define NTFX_SIGNAL(x) static_cast<signal_t>(x)

NTFX_INLINE_TEMPLATE signal_t db(signal_t x) noexcept {
  return NTFX_SIGNAL(20.0) * std::log10(x);
}

NTFX_INLINE_TEMPLATE signal_t invDb(signal_t x) noexcept {
  return std::pow(NTFX_SIGNAL(10.0), x * NTFX_SIGNAL(0.05));
}

constexpr int rmsDelayLineLength = 16384;
template <typename signal_t>
struct RmsSensorState {
  std::array<signal_t, rmsDelayLineLength> delayLine;
  signal_t accum = NTFX_SIGNAL(0.0);
  int i          = 0;
  NTFX_INLINE_MEMBER void reset() noexcept {
    std::fill(delayLine.begin(), delayLine.end(), NTFX_SIGNAL(0.0));
    accum = NTFX_SIGNAL(0.0);
  }
};

NTFX_INLINE_TEMPLATE signal_t rmsSensor(
    RmsSensorState<signal_t>* p_state, size_t nRms, signal_t x) noexcept {
  if (nRms > rmsDelayLineLength) { return NTFX_SIGNAL(0.0); }
  signal_t _x = x * x;
  if (_x != _x) { _x = NTFX_SIGNAL(0.0); }
  p_state->accum += _x - p_state->delayLine[p_state->i];
  p_state->delayLine[p_state->i] = _x;
  p_state->i++;
  if (p_state->i >= nRms) { p_state->i = 0; }
  signal_t y = std::sqrt(2.0 * p_state->accum / nRms);
  if (y != y) { y = NTFX_SIGNAL(0.0); }
  // checkNotFinite(y, ErrorVal::e_rmsSensor);
  return y;
}