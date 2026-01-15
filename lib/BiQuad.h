#pragma once

#include "Stereo.h"
#include "utils.h"

#include <cmath>

namespace NtFx {
namespace Biquad {
  enum class Shape { unknown, bell, hiShelf, loShelf, notch, hpf, lpf, apf, none };

  template <typename signal_t>
  struct Settings {
    Shape shape { Shape::bell };
    signal_t fc { 1000.0f };
    signal_t gain_db { 0.0f };
    signal_t q { 0.707f };
    bool enable { true };
  };

  template <typename signal_t>
  struct Coeffs6 {
    signal_t b[3] { 1, 0, 0 };
    signal_t a[3] { 1, 0, 0 };
  };

  template <typename signal_t>
  struct Coeffs5 {
    signal_t b[3] { 1, 0, 0 };
    signal_t a[2] { 0, 0 };
  };

  template <typename signal_t>
  struct State {
    signal_t x[2] { 0, 0 };
    signal_t y[2] { 0, 0 };
  };

  NTFX_INLINE_TEMPLATE signal_t biQuad6(
      const Coeffs6<signal_t>* p_coeffs, State<signal_t>* p_state, signal_t x) {
    signal_t y = p_coeffs->b[0] * x
        + p_coeffs->b[1] * p_state->x[0]
        + p_coeffs->b[2] * p_state->x[1]
        - p_coeffs->a[1] * p_state->y[0]
        - p_coeffs->a[2] * p_state->y[1];
    p_state->y[1] = p_state->y[0];
    p_state->y[0] = y / p_coeffs->a[0];
    p_state->x[1] = p_state->x[0];
    p_state->x[0] = x;
    return y;
  }

  NTFX_INLINE_TEMPLATE Stereo<signal_t> biQuad6s(const Coeffs6<signal_t>* p_coeffs,
      State<signal_t>* p_stateL,
      State<signal_t>* p_stateR,
      Stereo<signal_t> x) {
    return { biQuad(p_coeffs, p_stateL, x.l), biQuad(p_coeffs, p_stateR, x.r) };
  }

  NTFX_INLINE_TEMPLATE signal_t biQuad5(
      const Coeffs5<signal_t>* p_coeffs, State<signal_t>* p_state, signal_t x) {
    signal_t y = p_coeffs->b[0] * x
        + p_coeffs->b[1] * p_state->x[0]
        + p_coeffs->b[2] * p_state->x[1]
        - p_coeffs->a[0] * p_state->y[0]
        - p_coeffs->a[1] * p_state->y[1];
    p_state->y[1] = p_state->y[0];
    p_state->y[0] = y;
    p_state->x[1] = p_state->x[0];
    p_state->x[0] = x;
    return y;
  }

  NTFX_INLINE_TEMPLATE Stereo<signal_t> biQuad5s(const Coeffs5<signal_t>* p_coeffs,
      State<signal_t>* p_stateL,
      State<signal_t>* p_stateR,
      Stereo<signal_t> x) {
    return { biQuad5(p_coeffs, p_stateL, x.l), biQuad5(p_coeffs, p_stateR, x.r) };
  }

  NTFX_INLINE_TEMPLATE Coeffs5<signal_t> normalizeCoeffs(Coeffs6<signal_t> coeffs6) {
    Coeffs5<signal_t> coeffs5;
    coeffs5.b[0] = coeffs6.b[0] / coeffs6.a[0];
    coeffs5.b[1] = coeffs6.b[1] / coeffs6.a[0];
    coeffs5.b[2] = coeffs6.b[2] / coeffs6.a[0];
    coeffs5.a[0] = coeffs6.a[1] / coeffs6.a[0];
    coeffs5.a[1] = coeffs6.a[2] / coeffs6.a[0];
    return coeffs5;
  }

  NTFX_INLINE_TEMPLATE Coeffs5<signal_t> calcCoeffs5(
      Settings<signal_t> settings, float fs) {
    return calcCoeffs5<signal_t>((settings.enable ? settings.shape : Shape::none),
        fs,
        settings.fc,
        settings.q,
        std::pow(10, (settings.gain_db / 40)));
  }

  NTFX_INLINE_TEMPLATE Coeffs5<signal_t> calcCoeffs5(
      Shape s, signal_t fs, signal_t fc, signal_t q, signal_t a) {
    auto coeffs6 = calcCoeffs6(s, fs, fc, q, a);
    return normalizeCoeffs(coeffs6);
  }

  NTFX_INLINE_TEMPLATE Coeffs6<signal_t> calcCoeffs6(
      Settings<signal_t> settings, float fs) {
    return calcCoeffs6<signal_t>((settings.enable ? settings.shape : Shape::none),
        fs,
        settings.fc,
        settings.q,
        std::pow(10, (settings.gain_db / 40)));
  }

  NTFX_INLINE_TEMPLATE Coeffs6<signal_t> calcCoeffsBell(
      signal_t fs, signal_t fc, signal_t q, signal_t a) {
    auto w0    = 2.0 * M_PI * fc / fs;
    auto cosW0 = std::cos(w0);
    auto alpha = std::sin(w0) / (2.0 * q);
    Coeffs6<signal_t> c;
    c.b[0] = 1.0 + alpha * a;
    c.b[1] = -2.0 * cosW0;
    c.b[2] = 1.0 - alpha * a;
    c.a[0] = 1.0 + alpha / a;
    c.a[1] = -2.0 * cosW0;
    c.a[2] = 1.0 - alpha / a;
    return c;
  }

  NTFX_INLINE_TEMPLATE Coeffs6<signal_t> calcCoeffsLoShelf(
      signal_t fs, signal_t fc, signal_t q, signal_t a) {
    auto w0    = 2.0 * M_PI * fc / fs;
    auto cosW0 = std::cos(w0);
    auto alpha = std::sin(w0) / (2.0 * q);
    Coeffs6<signal_t> c;
    c.b[0] = a * ((a + 1.0) - (a - 1.0) * cosW0 + 2.0 * std::sqrt(a) * alpha);
    c.b[1] = 2.0 * a * ((a - 1.0) - (a + 1.0) * cosW0);
    c.b[2] = a * ((a + 1.0) - (a - 1.0) * cosW0 - 2.0 * std::sqrt(a) * alpha);
    c.a[0] = (a + 1.0) + (a - 1.0) * cosW0 + 2.0 * std::sqrt(a) * alpha;
    c.a[1] = -2.0 * ((a - 1.0) + (a + 1.0) * cosW0);
    c.a[2] = (a + 1.0) + (a - 1.0) * cosW0 - 2.0 * std::sqrt(a) * alpha;
    return c;
  }

  NTFX_INLINE_TEMPLATE Coeffs6<signal_t> calcCoeffsHiShelf(
      signal_t fs, signal_t fc, signal_t q, signal_t a) {
    auto w0    = 2.0 * M_PI * fc / fs;
    auto cosW0 = std::cos(w0);
    auto alpha = std::sin(w0) / (2.0 * q);
    Coeffs6<signal_t> c;
    c.b[0] = a * ((a + 1.0) + (a - 1.0) * cosW0 + 2.0 * std::sqrt(a) * alpha);
    c.b[1] = -2.0 * a * ((a - 1.0) + (a + 1.0) * cosW0);
    c.b[2] = a * ((a + 1.0) + (a - 1.0) * cosW0 - 2.0 * std::sqrt(a) * alpha);
    c.a[0] = (a + 1.0) - (a - 1.0) * cosW0 + 2.0 * std::sqrt(a) * alpha;
    c.a[1] = 2.0 * ((a - 1.0) - (a + 1.0) * cosW0);
    c.a[2] = (a + 1.0) - (a - 1.0) * cosW0 - 2.0 * std::sqrt(a) * alpha;
    return c;
  }

  NTFX_INLINE_TEMPLATE Coeffs6<signal_t> calcCoeffsHpf(
      signal_t fs, signal_t fc, signal_t q) {
    auto w0    = 2.0 * M_PI * fc / fs;
    auto cosW0 = std::cos(w0);
    auto alpha = std::sin(w0) / (2.0 * q);
    Coeffs6<signal_t> c;
    c.b[0] = (1.0 + cosW0) / 2;
    c.b[1] = -(1.0 + cosW0);
    c.b[2] = (1.0 + cosW0) / 2;
    c.a[0] = 1.0 + alpha;
    c.a[1] = -2.0 * cosW0;
    c.a[2] = 1.0 - alpha;
    return c;
  }

  NTFX_INLINE_TEMPLATE Coeffs6<signal_t> calcCoeffsLpf(
      signal_t fs, signal_t fc, signal_t q) {
    auto w0    = 2.0 * M_PI * fc / fs;
    auto cosW0 = std::cos(w0);
    auto alpha = std::sin(w0) / (2.0 * q);
    Coeffs6<signal_t> c;
    c.b[0] = (1.0 - cosW0) / 2;
    c.b[1] = 1.0 - cosW0;
    c.b[2] = (1.0 - cosW0) / 2;
    c.a[0] = 1.0 + alpha;
    c.a[1] = -2.0 * cosW0;
    c.a[2] = 1.0 - alpha;
    return c;
  }

  NTFX_INLINE_TEMPLATE Coeffs6<signal_t> calcCoeffsApf(
      signal_t fs, signal_t fc, signal_t q) {
    auto w0    = 2.0 * M_PI * fc / fs;
    auto cosW0 = std::cos(w0);
    auto alpha = std::sin(w0) / (2.0 * q);
    Coeffs6<signal_t> c;
    c.b[0] = 1.0 - alpha;
    c.b[1] = -2.0 * cosW0;
    c.b[2] = 1.0 + alpha;
    c.a[0] = 1.0 + alpha;
    c.a[1] = -2.0 * cosW0;
    c.a[2] = 1.0 - alpha;
    return c;
  }

  NTFX_INLINE_TEMPLATE Coeffs6<signal_t> calcCoeffsNotch(
      signal_t fs, signal_t fc, signal_t q) {
    auto w0    = 2.0 * M_PI * fc / fs;
    auto cosW0 = std::cos(w0);
    auto alpha = std::sin(w0) / (2.0 * q);
    Coeffs6<signal_t> c;
    c.b[0] = 1.0;
    c.b[1] = -2.0 * cosW0;
    c.b[2] = 1.0;
    c.a[0] = 1.0 + alpha;
    c.a[1] = -2.0 * cosW0;
    c.a[2] = 1.0 - alpha;
    return c;
  }

  NTFX_INLINE_TEMPLATE Coeffs6<signal_t> calcCoeffs6(
      Shape s, signal_t fs, signal_t fc, signal_t q, signal_t a) {
    Coeffs6<signal_t> c;
    switch (s) {
    case Shape::loShelf:
      c = calcCoeffsLoShelf<signal_t>(fs, fc, q, a);
      break;
    case Shape::hiShelf:
      c = calcCoeffsHiShelf<signal_t>(fs, fc, q, a);
      break;
    case Shape::bell:
      c = calcCoeffsBell<signal_t>(fs, fc, q, a);
      break;
    case Shape::lpf:
      c = calcCoeffsLpf<signal_t>(fs, fc, q);
      break;
    case Shape::hpf:
      c = calcCoeffsHpf<signal_t>(fs, fc, q);
      break;
    case Shape::apf:
      c = calcCoeffsApf<signal_t>(fs, fc, q);
      break;
    case Shape::notch:
      c = calcCoeffsNotch<signal_t>(fs, fc, q);
      break;
    default:
      c = { 1.0, 0.0, 0.0, 1.0, 0.0, 0.0 };
      break;
    }
    return c;
  }

} // namespace Biquad
} // namespace NtFx
