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

#include "lib/Audio.h"
#include "lib/Component.h"

#include "gcem.hpp"
#include <array>

namespace NtFx {
namespace Biquad {
  enum Shape : int { bell, hiShelf, loShelf, notch, hpf, lpf, apf, bpf, none };

  struct Settings {
    Shape shape { Shape::bell };
    signal_t fc_hz { 1000.0 };
    signal_t gain_db { 0.0 };
    signal_t q { 0.707 };
  };

  struct Coeffs6 {
    signal_t b[3] { 1, 0, 0 };
    signal_t a[3] { 1, 0, 0 };
  };

  struct Coeffs5 {
    signal_t b[3] { 1, 0, 0 };
    signal_t a[2] { 0, 0 };
  };

  struct Coeffs4 {
    signal_t b[2] { 0, 0 };
    signal_t a[2] { 0, 0 };
  };

  template <int nStages = 1>
  struct CascadeCoeffs {
    std::array<Coeffs4, nStages> c;
    signal_t b0 { 1 };
  };

  struct State {
    signal_t x[2] { 0, 0 };
    signal_t y[2] { 0, 0 };
  };

  struct StereoState {
    State l;
    State r;
  };

  template <int nStages = 1>
  struct CascadeState {
    signal_t _xn[2] { 0, 0 };
    signal_t _yn[2 * nStages] { 0, 0 };
  };

  inline static signal_t processBiquad5(
      signal_t x, Coeffs5& coeffs, State& state) noexcept {
    signal_t y = coeffs.b[0] * x + coeffs.b[1] * state.x[0]
        + coeffs.b[2] * state.x[1] - coeffs.a[0] * state.y[0]
        - coeffs.a[1] * state.y[1];
    state.y[1] = state.y[0];
    state.y[0] = y;
    state.x[1] = state.x[0];
    state.x[0] = x;
    return y;
  }

  static inline Coeffs6 calcCoeffsBell(
      signal_t fs, signal_t fc_hz, signal_t q, signal_t a) {
    auto w0    = 2.0 * GCEM_PI * fc_hz / fs;
    auto cosW0 = gcem::cos(w0);
    auto alpha = gcem::sin(w0) / (2.0 * q);
    Coeffs6 c;
    c.b[0] = 1.0 + alpha * a;
    c.b[1] = -2.0 * cosW0;
    c.b[2] = 1.0 - alpha * a;
    c.a[0] = 1.0 + alpha / a;
    c.a[1] = -2.0 * cosW0;
    c.a[2] = 1.0 - alpha / a;
    return c;
  }

  static inline Coeffs6 calcCoeffsLoShelf(
      signal_t fs, signal_t fc_hz, signal_t q, signal_t a) {
    auto w0    = 2.0 * GCEM_PI * fc_hz / fs;
    auto cosW0 = gcem::cos(w0);
    auto alpha = gcem::sin(w0) / (2.0 * q);
    Coeffs6 c;
    c.b[0] = a * ((a + 1.0) - (a - 1.0) * cosW0 + 2.0 * gcem::sqrt(a) * alpha);
    c.b[1] = 2.0 * a * ((a - 1.0) - (a + 1.0) * cosW0);
    c.b[2] = a * ((a + 1.0) - (a - 1.0) * cosW0 - 2.0 * gcem::sqrt(a) * alpha);
    c.a[0] = (a + 1.0) + (a - 1.0) * cosW0 + 2.0 * gcem::sqrt(a) * alpha;
    c.a[1] = -2.0 * ((a - 1.0) + (a + 1.0) * cosW0);
    c.a[2] = (a + 1.0) + (a - 1.0) * cosW0 - 2.0 * gcem::sqrt(a) * alpha;
    return c;
  }

  static inline Coeffs6 calcCoeffsHiShelf(
      signal_t fs, signal_t fc_hz, signal_t q, signal_t a) {
    auto w0    = 2.0 * GCEM_PI * fc_hz / fs;
    auto cosW0 = gcem::cos(w0);
    auto alpha = gcem::sin(w0) / (2.0 * q);
    Coeffs6 c;
    c.b[0] = a * ((a + 1.0) + (a - 1.0) * cosW0 + 2.0 * gcem::sqrt(a) * alpha);
    c.b[1] = -2.0 * a * ((a - 1.0) + (a + 1.0) * cosW0);
    c.b[2] = a * ((a + 1.0) + (a - 1.0) * cosW0 - 2.0 * gcem::sqrt(a) * alpha);
    c.a[0] = (a + 1.0) - (a - 1.0) * cosW0 + 2.0 * gcem::sqrt(a) * alpha;
    c.a[1] = 2.0 * ((a - 1.0) - (a + 1.0) * cosW0);
    c.a[2] = (a + 1.0) - (a - 1.0) * cosW0 - 2.0 * gcem::sqrt(a) * alpha;
    return c;
  }

  static inline Coeffs6 calcCoeffsHpf(signal_t fs, signal_t fc_hz, signal_t q) {
    auto w0    = 2.0 * GCEM_PI * fc_hz / fs;
    auto cosW0 = gcem::cos(w0);
    auto alpha = gcem::sin(w0) / (2.0 * q);
    Coeffs6 c;
    c.b[0] = (1.0 + cosW0) / 2;
    c.b[1] = -(1.0 + cosW0);
    c.b[2] = (1.0 + cosW0) / 2;
    c.a[0] = 1.0 + alpha;
    c.a[1] = -2.0 * cosW0;
    c.a[2] = 1.0 - alpha;
    return c;
  }

  static inline Coeffs6 calcCoeffsLpf(signal_t fs, signal_t fc_hz, signal_t q) {
    auto w0    = 2.0 * GCEM_PI * fc_hz / fs;
    auto cosW0 = gcem::cos(w0);
    auto alpha = gcem::sin(w0) / (2.0 * q);
    Coeffs6 c;
    c.b[0] = (1.0 - cosW0) / 2;
    c.b[1] = 1.0 - cosW0;
    c.b[2] = (1.0 - cosW0) / 2;
    c.a[0] = 1.0 + alpha;
    c.a[1] = -2.0 * cosW0;
    c.a[2] = 1.0 - alpha;
    return c;
  }

  static inline Coeffs6 calcCoeffsApf(signal_t fs, signal_t fc_hz, signal_t q) {
    auto w0    = 2.0 * GCEM_PI * fc_hz / fs;
    auto cosW0 = gcem::cos(w0);
    auto alpha = gcem::sin(w0) / (2.0 * q);
    Coeffs6 c;
    c.b[0] = 1.0 - alpha;
    c.b[1] = -2.0 * cosW0;
    c.b[2] = 1.0 + alpha;
    c.a[0] = 1.0 + alpha;
    c.a[1] = -2.0 * cosW0;
    c.a[2] = 1.0 - alpha;
    return c;
  }

  static inline Coeffs6 calcCoeffsBpf(signal_t fs, signal_t fc_hz, signal_t q) {
    auto w0    = 2.0 * GCEM_PI * fc_hz / fs;
    auto cosW0 = gcem::cos(w0);
    auto alpha = gcem::sin(w0) / (2.0 * q);
    Coeffs6 c;
    c.b[0] = alpha;
    c.b[1] = 0;
    c.b[2] = -alpha;
    c.a[0] = 1.0 + alpha;
    c.a[1] = -2.0 * cosW0;
    c.a[2] = 1.0 - alpha;
    return c;
  }

  static inline Coeffs6 calcCoeffsNotch(
      signal_t fs, signal_t fc_hz, signal_t q) {
    auto w0    = 2.0 * GCEM_PI * fc_hz / fs;
    auto cosW0 = gcem::cos(w0);
    auto alpha = gcem::sin(w0) / (2.0 * q);
    Coeffs6 c;
    c.b[0] = 1.0;
    c.b[1] = -2.0 * cosW0;
    c.b[2] = 1.0;
    c.a[0] = 1.0 + alpha;
    c.a[1] = -2.0 * cosW0;
    c.a[2] = 1.0 - alpha;
    return c;
  }

  static inline Coeffs6 calcCoeffs6(
      Shape s, signal_t fs, signal_t fc_hz, signal_t q, signal_t a) {
    Coeffs6 c;
    switch (s) {
    case Shape::loShelf:
      c = calcCoeffsLoShelf(fs, fc_hz, q, a);
      break;
    case Shape::hiShelf:
      c = calcCoeffsHiShelf(fs, fc_hz, q, a);
      break;
    case Shape::bell:
      c = calcCoeffsBell(fs, fc_hz, q, a);
      break;
    case Shape::lpf:
      c = calcCoeffsLpf(fs, fc_hz, q);
      break;
    case Shape::hpf:
      c = calcCoeffsHpf(fs, fc_hz, q);
      break;
    case Shape::apf:
      c = calcCoeffsApf(fs, fc_hz, q);
      break;
    case Shape::bpf:
      c = calcCoeffsBpf(fs, fc_hz, q);
      break;
    case Shape::notch:
      c = calcCoeffsNotch(fs, fc_hz, q);
      break;
    case Shape::none:
    default:
      c = { 1.0, 0.0, 0.0, 1.0, 0.0, 0.0 };
      break;
    }
    return c;
  }

  static inline Coeffs5 normalizeCoeffs(Coeffs6 coeffs6) {
    Coeffs5 coeffs5;
    coeffs5.b[0] = coeffs6.b[0] / coeffs6.a[0];
    coeffs5.b[1] = coeffs6.b[1] / coeffs6.a[0];
    coeffs5.b[2] = coeffs6.b[2] / coeffs6.a[0];
    coeffs5.a[0] = coeffs6.a[1] / coeffs6.a[0];
    coeffs5.a[1] = coeffs6.a[2] / coeffs6.a[0];
    return coeffs5;
  }

  static inline Coeffs5 calcCoeffs5(
      Shape s, signal_t fs, signal_t fc_hz, signal_t q, signal_t a) {
    auto coeffs6 = calcCoeffs6(s, fs, fc_hz, q, a);
    return normalizeCoeffs(coeffs6);
  }

  static inline Coeffs5 calcCoeffs5(const Settings& settings, signal_t fs) {
    return calcCoeffs5(settings.shape,
        fs,
        settings.fc_hz,
        settings.q,
        gcem::pow(10, (settings.gain_db / 40)));
  }

  static inline Coeffs6 calcCoeffs6(const Settings& settings, signal_t fs) {
    return calcCoeffs6(settings.shape,
        fs,
        settings.fc_hz,
        settings.q,
        gcem::pow(10, (settings.gain_db / 40)));
  }

  struct Biquad6 {
    Coeffs6& coeffs;
    State state;
    Biquad6(Coeffs6& coeffs) : coeffs(coeffs) { }
    inline signal_t process(signal_t x) {
      signal_t y = (this->coeffs.b[0] * x + this->coeffs.b[1] * this->state.x[0]
                       + this->coeffs.b[2] * this->state.x[1]
                       - this->coeffs.a[1] * this->state.y[0]
                       - this->coeffs.a[2] * this->state.y[1])
          / this->coeffs.a[0];
      this->state.y[1] = this->state.y[0];
      this->state.y[0] = y;
      this->state.x[1] = this->state.x[0];
      this->state.x[0] = x;
      return y;
    }
  };

  template <int nStages = 1>
  static inline CascadeCoeffs<nStages> calcCascadeCoeffs(
      const std::array<Settings, nStages>& ra_settings, float fs) {
    CascadeCoeffs<nStages> coeffs;
    for (size_t i = 0; i < nStages; i++) {
      auto coeffs5     = calcCoeffs5(ra_settings[i], fs);
      coeffs.c[i].b[0] = coeffs5.b[1] / coeffs5.b[0];
      coeffs.c[i].b[1] = coeffs5.b[2] / coeffs5.b[0];
      coeffs.c[i].a[0] = -coeffs5.a[0];
      coeffs.c[i].a[1] = -coeffs5.a[1];
      coeffs.b0 *= coeffs5.b[0];
    }
    return coeffs;
  }

  template <int nStages = 1>
  static inline signal_t processCascade(signal_t x,
      CascadeState<nStages>& r_state,
      CascadeCoeffs<nStages>& r_coeffs) {
    signal_t acc = x + r_coeffs.c[0].b[0] * r_state._xn[0]
        + r_coeffs.c[0].b[1] * r_state._xn[1]
        + r_coeffs.c[0].a[0] * r_state._yn[0]
        + r_coeffs.c[0].a[1] * r_state._yn[1];

    r_state._xn[1] = r_state._xn[0];
    r_state._xn[0] = x;
    signal_t xNext = acc;

    for (size_t i = 1; i < nStages; i++) {
      acc = xNext + r_coeffs.c[i].b[0] * r_state._yn[(i - 1) * 2]
          + r_coeffs.c[i].b[1] * r_state._yn[(i - 1) * 2 + 1]
          + r_coeffs.c[i].a[0] * r_state._yn[i * 2]
          + r_coeffs.c[i].a[1] * r_state._yn[i * 2 + 1];

      r_state._yn[(i - 1) * 2 + 1] = r_state._yn[(i - 1) * 2];
      r_state._yn[(i - 1) * 2]     = xNext;
      xNext                        = acc;
    }

    r_state._yn[(nStages - 1) * 2 + 1] = r_state._yn[(nStages - 1) * 2];
    r_state._yn[(nStages - 1) * 2]     = acc;
    return acc * r_coeffs.b0;
  }

  struct EqBandMono : public ComponentBase<Audio> {
    Settings settings;
    Coeffs5 coeffs;
    State state;

    virtual Audio process(Audio x) noexcept override {
      return processBiquad5(x.l, this->coeffs, this->state);
    }
    virtual void update() noexcept override {
      this->coeffs = calcCoeffs5(this->settings, this->fs);
    }
    virtual void reset(float fs) noexcept override {
      this->fs    = fs;
      this->state = { { 0, 0 }, { 0, 0 } };
      this->update();
    }
  };

  struct EqBand6Stereo : public ComponentBase<Audio> {
    Biquad6 l;
    Biquad6 r;
    Settings settings;
    Coeffs6 coeffs;
    EqBand6Stereo() : l(coeffs), r(coeffs) { }
    virtual Audio process(Audio x) noexcept override {
      return { this->l.process(x.l), this->r.process(x.r) };
    }
    virtual void update() noexcept override {
      this->coeffs = calcCoeffs6(settings, this->fs);
    }
    virtual void reset(float fs) noexcept override {
      this->fs = fs;
      this->update();
    }
  };

  struct EqBand6Mono : public ComponentBase<Audio> {
    Biquad6 l;
    Settings settings;
    Coeffs6 coeffs;
    EqBand6Mono() : l(coeffs) { }
    virtual Audio process(Audio x) noexcept override {
      return this->l.process(x.l);
    }
    virtual void update() noexcept override {
      this->coeffs = calcCoeffs6(settings, this->fs);
    }
    virtual void reset(float fs) noexcept override {
      this->fs = fs;
      this->update();
    }
  };

  struct EqBandStereo : public ComponentBase<Audio> {
    Settings settings;
    Coeffs5 coeffs;
    State stateL;
    State stateR;

    virtual Audio process(Audio x) noexcept override {
      return { processBiquad5(x.l, this->coeffs, this->stateL),
        processBiquad5(x.r, this->coeffs, this->stateR) };
    }
    virtual void update() noexcept override {
      this->coeffs = calcCoeffs5(this->settings, this->fs);
    }
    virtual void reset(float fs) noexcept override {
      this->fs     = fs;
      this->stateL = { { 0, 0 }, { 0, 0 } };
      this->stateR = { { 0, 0 }, { 0, 0 } };
      this->update();
    }
  };

  template <int nStages = 1>
  struct Eq : public ComponentBase<Audio> {
    std::array<Settings, nStages> settings;
    CascadeCoeffs<nStages> coeffs;
    CascadeState<nStages> stateL;
    CascadeState<nStages> stateR;
    virtual Audio process(Audio x) noexcept override {
      return { processCascade<nStages>(x.l, stateL, coeffs),
        processCascade<nStages>(x.r, stateR, coeffs) };
    }
    virtual void update() noexcept override {
      this->coeffs = calcCascadeCoeffs<nStages>(this->settings, this->fs);
    }
  };

  using EqBand6 = EqBand6Stereo;
  using EqBand  = EqBandStereo;
}
}