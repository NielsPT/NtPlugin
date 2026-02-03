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

#include <cmath>

namespace NtFx {
namespace Biquad {
  enum class Shape {
    unknown,
    bell,
    hiShelf,
    loShelf,
    notch,
    hpf,
    lpf,
    apf,
    none
  };

  template <typename signal_t>
  struct Settings {
    Shape shape { Shape::bell };
    signal_t fc_hz { 1000.0 };
    signal_t gain_db { 0.0 };
    signal_t q { 0.707 };
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

  template <typename signal_t>
  struct StereoState {
    State<signal_t> l;
    State<signal_t> r;
  };

  // template <typename signal_t>
  // struct Biquad6 {
  //   Coeffs6<signal_t> coeffs;
  //   State<signal_t> state;
  //   inline signal_t process(signal_t x) {
  //     signal_t y = this->coeffs.b[0] * x + this->coeffs.b[1] *
  //     this->state.x[0]
  //         + this->coeffs.b[2] * this->state.x[1]
  //         - this->coeffs.a[1] * this->state.y[0]
  //         - this->coeffs.a[2] * this->state.y[1];
  //     this->state.y[1] = this->state.y[0];
  //     this->state.y[0] = y / this->coeffs.a[0];
  //     this->state.x[1] = this->state.x[0];
  //     this->state.x[0] = x;
  //     return y;
  //   }
  //   inline void update(Settings<signal_t>& settings, float fs) {
  //     this->coeffs = calcCoeffs6<signal_t>(settings, fs);
  //   }
  //   // inline void reset() { this->state.reset(); }
  // };

  // template <typename signal_t>
  // struct BiQuad6Stereo {
  //   Settings<signal_t> settings;
  //   Biquad6<signal_t> l;
  //   Biquad6<signal_t> r;
  //   inline Stereo<signal_t> process(Stereo<signal_t> x) {
  //     return { l.process(x.l), r.process(x.r) };
  //   }
  //   inline void update(float fs) {
  //     this->l.update(this->settings, fs);
  //     this->r.update(this->settings, fs);
  //   }
  //   // inline void reset() {
  //   //   this->l.reset();
  //   //   this->r.reset();
  //   // }
  // };

  template <typename signal_t>
  struct Biquad5 {
    Coeffs5<signal_t> coeffs;
    State<signal_t> state;
    inline signal_t process(signal_t x) {
      signal_t y = this->coeffs.b[0] * x + this->coeffs.b[1] * this->state.x[0]
          + this->coeffs.b[2] * this->state.x[1]
          - this->coeffs.a[0] * this->state.y[0]
          - this->coeffs.a[1] * this->state.y[1];
      this->state.y[1] = this->state.y[0];
      this->state.y[0] = y;
      this->state.x[1] = this->state.x[0];
      this->state.x[0] = x;
      return y;
    }
    inline void update(Settings<signal_t>& settings, float fs) {
      this->coeffs = calcCoeffs5<signal_t>(settings, fs);
    }
  };

  template <typename signal_t>
  struct EqBand {
    Settings<signal_t> settings;
    Biquad5<signal_t> l;
    Biquad5<signal_t> r;
    // TODO: Gliders here?
    inline Stereo<signal_t> process(Stereo<signal_t> x) {
      return { l.process(x.l), r.process(x.r) };
    }
    inline void update(float fs) {
      this->l.update(this->settings, fs);
      this->r.update(this->settings, fs);
    }
  };

  template <typename signal_t>
  static inline Coeffs5<signal_t> normalizeCoeffs(Coeffs6<signal_t> coeffs6) {
    Coeffs5<signal_t> coeffs5;
    coeffs5.b[0] = coeffs6.b[0] / coeffs6.a[0];
    coeffs5.b[1] = coeffs6.b[1] / coeffs6.a[0];
    coeffs5.b[2] = coeffs6.b[2] / coeffs6.a[0];
    coeffs5.a[0] = coeffs6.a[1] / coeffs6.a[0];
    coeffs5.a[1] = coeffs6.a[2] / coeffs6.a[0];
    return coeffs5;
  }

  template <typename signal_t>
  static inline Coeffs5<signal_t> calcCoeffs5(
      Settings<signal_t>& settings, float fs) {
    return calcCoeffs5<signal_t>(settings.shape,
        fs,
        settings.fc_hz,
        settings.q,
        std::pow(10, (settings.gain_db / 40)));
  }

  template <typename signal_t>
  static inline Coeffs5<signal_t> calcCoeffs5(
      Shape s, signal_t fs, signal_t fc_hz, signal_t q, signal_t a) {
    auto coeffs6 = calcCoeffs6(s, fs, fc_hz, q, a);
    return normalizeCoeffs(coeffs6);
  }

  template <typename signal_t>
  static inline Coeffs6<signal_t> calcCoeffs6(
      Settings<signal_t>& settings, float fs) {
    return calcCoeffs6<signal_t>(settings.shape,
        fs,
        settings.fc_hz,
        settings.q,
        std::pow(10, (settings.gain_db / 40)));
  }

  template <typename signal_t>
  static inline Coeffs6<signal_t> calcCoeffsBell(
      signal_t fs, signal_t fc_hz, signal_t q, signal_t a) {
    auto w0    = 2.0 * M_PI * fc_hz / fs;
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

  template <typename signal_t>
  static inline Coeffs6<signal_t> calcCoeffsLoShelf(
      signal_t fs, signal_t fc_hz, signal_t q, signal_t a) {
    auto w0    = 2.0 * M_PI * fc_hz / fs;
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

  template <typename signal_t>
  static inline Coeffs6<signal_t> calcCoeffsHiShelf(
      signal_t fs, signal_t fc_hz, signal_t q, signal_t a) {
    auto w0    = 2.0 * M_PI * fc_hz / fs;
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

  template <typename signal_t>
  static inline Coeffs6<signal_t> calcCoeffsHpf(
      signal_t fs, signal_t fc_hz, signal_t q) {
    auto w0    = 2.0 * M_PI * fc_hz / fs;
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

  template <typename signal_t>
  static inline Coeffs6<signal_t> calcCoeffsLpf(
      signal_t fs, signal_t fc_hz, signal_t q) {
    auto w0    = 2.0 * M_PI * fc_hz / fs;
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

  template <typename signal_t>
  static inline Coeffs6<signal_t> calcCoeffsApf(
      signal_t fs, signal_t fc_hz, signal_t q) {
    auto w0    = 2.0 * M_PI * fc_hz / fs;
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

  template <typename signal_t>
  static inline Coeffs6<signal_t> calcCoeffsNotch(
      signal_t fs, signal_t fc_hz, signal_t q) {
    auto w0    = 2.0 * M_PI * fc_hz / fs;
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

  template <typename signal_t>
  static inline Coeffs6<signal_t> calcCoeffs6(
      Shape s, signal_t fs, signal_t fc_hz, signal_t q, signal_t a) {
    Coeffs6<signal_t> c;
    switch (s) {
    case Shape::loShelf:
      c = calcCoeffsLoShelf<signal_t>(fs, fc_hz, q, a);
      break;
    case Shape::hiShelf:
      c = calcCoeffsHiShelf<signal_t>(fs, fc_hz, q, a);
      break;
    case Shape::bell:
      c = calcCoeffsBell<signal_t>(fs, fc_hz, q, a);
      break;
    case Shape::lpf:
      c = calcCoeffsLpf<signal_t>(fs, fc_hz, q);
      break;
    case Shape::hpf:
      c = calcCoeffsHpf<signal_t>(fs, fc_hz, q);
      break;
    case Shape::apf:
      c = calcCoeffsApf<signal_t>(fs, fc_hz, q);
      break;
    case Shape::notch:
      c = calcCoeffsNotch<signal_t>(fs, fc_hz, q);
      break;
    default:
      c = { 1.0, 0.0, 0.0, 1.0, 0.0, 0.0 };
      break;
    }
    return c;
  }

} // namespace Eq
} // namespace NtFx
