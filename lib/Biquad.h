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
    bpf,
    none
  };

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

  struct State {
    signal_t x[2] { 0, 0 };
    signal_t y[2] { 0, 0 };
  };

  struct StereoState {
    State l;
    State r;
  };

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
    c.b[0] = q * alpha;
    c.b[1] = 0;
    c.b[2] = -q * alpha;
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
    case Shape::unknown:
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

  static inline Coeffs5 calcCoeffs5(Settings& settings, signal_t fs) {
    return calcCoeffs5(settings.shape,
        fs,
        settings.fc_hz,
        settings.q,
        gcem::pow(10, (settings.gain_db / 40)));
  }

  static inline Coeffs6 calcCoeffs6(Settings& settings, signal_t fs) {
    return calcCoeffs6(settings.shape,
        fs,
        settings.fc_hz,
        settings.q,
        gcem::pow(10, (settings.gain_db / 40)));
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

  using EqBand6 = EqBand6Stereo;
  using EqBand  = EqBandStereo;
} // namespace Biquad
} // namespace NtFx

// #pragma once

// // Indexes for coeffs
// constexpr int b0     = 0;
// constexpr int b1     = 1;
// constexpr int b2     = 2;
// constexpr int a1     = 3;
// constexpr int a2     = 4;
// constexpr int minus1 = 0;
// constexpr int minus2 = 1;

// template <typename T, bool t_32bitState = false> struct BiquadParameterSet {
//    /**
//     * @brief Coeffs are stored in the order { b0 a1 b1 a2 b2, padding * 3}
//     *
//     */
//    T m_coeffs[8] { 1, 0, 0, 0, 0, 0, 0, 0 };
// };

// /**
//  * @brief Cascade of t_numStages biquad filters. Optimized to recycle feed
//  back state
//  * of previous stage as feed forward state for the next stage in the cascade.
//  This
//  * should decrease the amount of writes by half in the filter. To be
//  confirmed.
//  *
//  * @tparam T Data type of audio. Double or float.
//  * @tparam t_numStages Number of stages.
//  */
// template <typename T, int t_numStages = 1> struct BiquadCascade {
//    BiquadParameterSet<T> m_params[t_numStages];

//    /**
//     * @brief Feed forward state for frist filter in chain.
//     *
//     */
//    T m_xn[2];

//    /**
//     * @brief Feed back state for each stage doubles as feed forward state for
//     the
//     * next stage.
//     *
//     */
//    T m_yn[2 * t_numStages];

//    /**
//     * @brief Processes one sample with BiquadCascade.
//     *
//     * @param x Input sample.
//     * @return T y Out put sample.
//     */
//     T processSample(T x) {
//       // First stage uses the stored feed forward state m_xn.
//       T acc = m_params[0].m_coeffs[b0] * x;

//       acc += m_params[0].m_coeffs[b1] * m_xn[minus1];
//       acc += m_params[0].m_coeffs[b2] * m_xn[minus2];
//       acc += m_params[0].m_coeffs[a1] * m_yn[minus1];
//       acc += m_params[0].m_coeffs[a2] * m_yn[minus2];

//       // Store feed forward state for first stage.
//       m_xn[minus2] = m_xn[minus1];
//       m_xn[minus1] = x;

//       T xRemainingStages = acc;

//       // Process the remaining stages using feedback state from the previous
//       stage
//       // for feed forward state.
//       for (size_t i = 1; i < t_numStages; i++) {
//          acc = m_params[i].m_coeffs[b0] * xRemainingStages;

//          acc += m_params[i].m_coeffs[b1] * m_yn[(i - 1) * 2 + minus1];
//          acc += m_params[i].m_coeffs[b2] * m_yn[(i - 1) * 2 + minus2];
//          acc += m_params[i].m_coeffs[a1] * m_yn[i * 2 + minus1];
//          acc += m_params[i].m_coeffs[a2] * m_yn[i * 2 + minus2];

//          // Update feed back state for previous stage.
//          m_yn[(i - 1) * 2 + minus2] = m_yn[(i - 1) * 2 + minus1];
//          m_yn[(i - 1) * 2 + minus1] = xRemainingStages;

//          xRemainingStages = acc;
//       }

//       // update feed back state for last stage.
//       m_yn[(t_numStages - 1) * 2 + minus2] = m_yn[(t_numStages - 1) * 2 +
//       minus1]; m_yn[(t_numStages - 1) * 2 + minus1] = acc; return acc;
//    }

//    /**
//     * @brief Initialize cascade by resetting all states.
//     *
//     * @return error
//     */
//    Error init() {
//       for (size_t i = 0; i < t_numStages * 2; i++) {
//          m_yn[i] = 0;
//       }
//       m_xn[0] = 0;
//       m_xn[1] = 0;

//       return Error::noError;
//    }

//    /**
//     * @brief Set the Coeffs of biquad filter, with normalization and scaling.
//     * This is the method to use in the general case and for any static
//     filter.
//     *
//     * @param[in] p_coeffs Pointer to unnormalized, floating point
//     coefficients.
//     * @param coeffSet Number of stage in cascade to change coeffs for.
//     * @return error
//     */
//    template <typename T_coeffs>
//    Error setCoeffs(const BiquadCoeffs<T_coeffs>* const p_coeffs, int coeffSet
//    = 0) {
//       if (coeffSet < 0 || coeffSet >= t_numStages) {
//          return Error::outOfRangeError;
//       }

//       // Copy the coeffs so that we don't mutate the original.
//       BiquadCoeffs<T_coeffs> coeffs = *p_coeffs;

//       // Normalize coeffs with respect to a0.
//       normalizeCoeffs(&coeffs);

//       m_params[coeffSet].m_coeffs[b0] = coeffs.m_b[0];
//       m_params[coeffSet].m_coeffs[b1] = coeffs.m_b[1];
//       m_params[coeffSet].m_coeffs[b2] = coeffs.m_b[2];
//       m_params[coeffSet].m_coeffs[a1] = -coeffs.m_a[1];
//       m_params[coeffSet].m_coeffs[a2] = -coeffs.m_a[2];

//       return Error::noError;
//    }
// };
// }
