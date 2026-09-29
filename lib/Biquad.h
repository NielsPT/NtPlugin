/**
 * @file Biquad.h
 * @author Niels Thøgersen (niels.thoegersen@gmail.com)
 * @brief Biquad filters.
 *
 * @copyright 2026 Niels Thøgersen, NTlyd
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

#include "lib/FirstOrder.h"
#include "lib/gcem.h"
#include <array>
#include <cstddef>
#include <type_traits>

namespace NtFx {
namespace Biquad {
  struct ButterworthTable {
    struct Second {
      static const constexpr signal_t q0 = 0.7071;
    };
    struct Third {
      static const constexpr signal_t q0 = 1.0000;
    };
    struct Fourth {
      static const constexpr signal_t q0 = 0.5412;
      static const constexpr signal_t q1 = 1.3065;
    };
    struct Fifth {
      static const constexpr signal_t q0 = 0.6180;
      static const constexpr signal_t q1 = 1.6182;
    };
    struct Sixth {
      static const constexpr signal_t q0 = 0.5176;
      static const constexpr signal_t q1 = 0.7071;
      static const constexpr signal_t q2 = 1.9319;
    };
    struct Seventh {
      static const constexpr signal_t q0 = 0.5550;
      static const constexpr signal_t q1 = 0.8019;
      static const constexpr signal_t q2 = 2.2471;
    };
    struct Eighth {
      static const constexpr signal_t q0 = 0.5098;
      static const constexpr signal_t q1 = 0.6013;
      static const constexpr signal_t q2 = 0.9000;
      static const constexpr signal_t q3 = 2.5628;
    };
  };

  struct LinkwitzTable {
    struct Second {
      static const constexpr signal_t q0 = 0.5;
    };
    struct Third {
      static const constexpr signal_t q0 = 0.5;
    };
    struct Fourth {
      static const constexpr signal_t q0 = 0.7071;
      static const constexpr signal_t q1 = 0.7071;
    };
    struct Fifth {
      static const constexpr signal_t q0 = 0.7071;
      static const constexpr signal_t q1 = 0.7071;
    };
    struct Sixth {
      static const constexpr signal_t q0 = 0.5;
      static const constexpr signal_t q1 = 1.0;
      static const constexpr signal_t q2 = 1.0;
    };
    struct Seventh {
      static const constexpr signal_t q0 = 0.5;
      static const constexpr signal_t q1 = 1.0;
      static const constexpr signal_t q2 = 1.0;
    };
    struct Eighth {
      static const constexpr signal_t q0 = 0.54;
      static const constexpr signal_t q1 = 1.35;
      static const constexpr signal_t q2 = 0.54;
      static const constexpr signal_t q3 = 1.35;
    };
  };

  enum class Shape : int {
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
    std::array<signal_t, 3> b { 1, 0, 0 };
    std::array<signal_t, 3> a { 1, 0, 0 };
  };

  struct Coeffs5 {
    std::array<signal_t, 3> b { 1, 0, 0 };
    std::array<signal_t, 2> a { 0, 0 };
  };

  struct Coeffs4 {
    std::array<signal_t, 2> b { 0, 0 };
    std::array<signal_t, 2> a { 0, 0 };
  };

  template <int nStages = 1>
  struct CascadeCoeffs {
    std::array<Coeffs4, nStages> c;
    signal_t b0 { 1 };
  };

  struct State {
    std::array<signal_t, 2> x { 0, 0 };
    std::array<signal_t, 2> y { 0, 0 };
  };

  struct StereoState {
    State l;
    State r;
  };

  template <size_t nStages = 1>
  struct CascadeState {
    std::array<signal_t, 2> _xn { 0, 0 };
    std::array<signal_t, 2 * nStages> _yn { 0, 0 };
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
    double w0  = 2.0 * NTFX_PI * fc_hz / fs;
    auto cosW0 = gcem::cos(w0);
    auto alpha = gcem::sin(w0) / (2.0 * q);
    Coeffs6 c;
    c.b[0] = signal_t(1.0 + alpha * a);
    c.b[1] = signal_t(-2.0 * cosW0);
    c.b[2] = signal_t(1.0 - alpha * a);
    c.a[0] = signal_t(1.0 + alpha / a);
    c.a[1] = signal_t(-2.0 * cosW0);
    c.a[2] = signal_t(1.0 - alpha / a);
    return c;
  }

  static inline Coeffs6 calcCoeffsLoShelf(
      signal_t fs, signal_t fc_hz, signal_t q, signal_t a) {
    double w0  = 2.0 * NTFX_PI * fc_hz / fs;
    auto cosW0 = gcem::cos(w0);
    auto alpha = gcem::sin(w0) / (signal_t(2.0) * q);
    Coeffs6 c;
    c.b[0] = signal_t(
        a * ((a + 1.0) - (a - 1.0) * cosW0 + 2.0 * gcem::sqrt(a) * alpha));
    c.b[1] = signal_t(2.0 * a * ((a - 1.0) - (a + 1.0) * cosW0));
    c.b[2] = signal_t(
        a * ((a + 1.0) - (a - 1.0) * cosW0 - 2.0 * gcem::sqrt(a) * alpha));
    c.a[0] =
        signal_t((a + 1.0) + (a - 1.0) * cosW0 + 2.0 * gcem::sqrt(a) * alpha);
    c.a[1] = signal_t(-2.0 * ((a - 1.0) + (a + 1.0) * cosW0));
    c.a[2] =
        signal_t((a + 1.0) + (a - 1.0) * cosW0 - 2.0 * gcem::sqrt(a) * alpha);
    return c;
  }

  static inline Coeffs6 calcCoeffsHiShelf(
      signal_t fs, signal_t fc_hz, signal_t q, signal_t a) {
    double w0  = 2.0 * NTFX_PI * fc_hz / fs;
    auto cosW0 = gcem::cos(w0);
    auto alpha = gcem::sin(w0) / (2.0 * q);
    Coeffs6 c;
    c.b[0] = signal_t(
        a * ((a + 1.0) + (a - 1.0) * cosW0 + 2.0 * gcem::sqrt(a) * alpha));
    c.b[1] = signal_t(-2.0 * a * ((a - 1.0) + (a + 1.0) * cosW0));
    c.b[2] = signal_t(
        a * ((a + 1.0) + (a - 1.0) * cosW0 - 2.0 * gcem::sqrt(a) * alpha));
    c.a[0] =
        signal_t((a + 1.0) - (a - 1.0) * cosW0 + 2.0 * gcem::sqrt(a) * alpha);
    c.a[1] = signal_t(2.0 * ((a - 1.0) - (a + 1.0) * cosW0));
    c.a[2] =
        signal_t((a + 1.0) - (a - 1.0) * cosW0 - 2.0 * gcem::sqrt(a) * alpha);
    return c;
  }

  static inline Coeffs6 calcCoeffsHpf(signal_t fs, signal_t fc_hz, signal_t q) {
    double w0  = 2.0 * NTFX_PI * fc_hz / fs;
    auto cosW0 = gcem::cos(w0);
    auto alpha = gcem::sin(w0) / (2.0 * q);
    Coeffs6 c;
    c.b[0] = signal_t((1.0 + cosW0) / 2);
    c.b[1] = signal_t(-(1.0 + cosW0));
    c.b[2] = signal_t((1.0 + cosW0) / 2);
    c.a[0] = signal_t(1.0 + alpha);
    c.a[1] = signal_t(-2.0 * cosW0);
    c.a[2] = signal_t(1.0 - alpha);
    return c;
  }

  static inline Coeffs6 calcCoeffsLpf(signal_t fs, signal_t fc_hz, signal_t q) {
    double w0  = 2.0 * NTFX_PI * fc_hz / fs;
    auto cosW0 = gcem::cos(w0);
    auto alpha = gcem::sin(w0) / (2.0 * q);
    Coeffs6 c;
    c.b[0] = signal_t((1.0 - cosW0) / 2);
    c.b[1] = signal_t(1.0 - cosW0);
    c.b[2] = signal_t((1.0 - cosW0) / 2);
    c.a[0] = signal_t(1.0 + alpha);
    c.a[1] = signal_t(-2.0 * cosW0);
    c.a[2] = signal_t(1.0 - alpha);
    return c;
  }

  static inline Coeffs6 calcCoeffsApf(signal_t fs, signal_t fc_hz, signal_t q) {
    double w0  = 2.0 * NTFX_PI * fc_hz / fs;
    auto cosW0 = gcem::cos(w0);
    auto alpha = gcem::sin(w0) / (2.0 * q);
    Coeffs6 c;
    c.b[0] = signal_t(1.0 - alpha);
    c.b[1] = signal_t(-2.0 * cosW0);
    c.b[2] = signal_t(1.0 + alpha);
    c.a[0] = signal_t(1.0 + alpha);
    c.a[1] = signal_t(-2.0 * cosW0);
    c.a[2] = signal_t(1.0 - alpha);
    return c;
  }

  static inline Coeffs6 calcCoeffsBpf(signal_t fs, signal_t fc_hz, signal_t q) {
    double w0  = 2.0 * NTFX_PI * fc_hz / fs;
    auto cosW0 = gcem::cos(w0);
    auto alpha = gcem::sin(w0) / (2.0 * q);
    Coeffs6 c;
    c.b[0] = signal_t(alpha);
    c.b[1] = signal_t(0);
    c.b[2] = signal_t(-alpha);
    c.a[0] = signal_t(1.0 + alpha);
    c.a[1] = signal_t(-2.0 * cosW0);
    c.a[2] = signal_t(1.0 - alpha);
    return c;
  }

  static inline Coeffs6 calcCoeffsNotch(
      signal_t fs, signal_t fc_hz, signal_t q) {
    double w0  = 2.0 * NTFX_PI * fc_hz / fs;
    auto cosW0 = gcem::cos(w0);
    auto alpha = gcem::sin(w0) / (2.0 * q);
    Coeffs6 c;
    c.b[0] = signal_t(1.0);
    c.b[1] = signal_t(-2.0 * cosW0);
    c.b[2] = signal_t(1.0);
    c.a[0] = signal_t(1.0 + alpha);
    c.a[1] = signal_t(-2.0 * cosW0);
    c.a[2] = signal_t(1.0 - alpha);
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
      c = { { 1.0, 0.0, 0.0 }, { 1.0, 0.0, 0.0 } };
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
        signal_t(gcem::pow(10.0, (settings.gain_db / 40.0))));
  }

  static inline Coeffs6 calcCoeffs6(const Settings& settings, signal_t fs) {
    return calcCoeffs6(settings.shape,
        fs,
        settings.fc_hz,
        settings.q,
        signal_t(gcem::pow(10.0, (settings.gain_db / 40.0))));
  }

  struct Biquad6 {
    const Coeffs6* _coeffs;
    State _state;
    Biquad6(const Coeffs6* coeffs) : _coeffs(coeffs) { }
    inline signal_t process(signal_t x) {
      signal_t y =
          (this->_coeffs->b[0] * x + this->_coeffs->b[1] * this->_state.x[0]
              + this->_coeffs->b[2] * this->_state.x[1]
              - this->_coeffs->a[1] * this->_state.y[0]
              - this->_coeffs->a[2] * this->_state.y[1])
          / this->_coeffs->a[0];
      this->_state.y[1] = this->_state.y[0];
      this->_state.y[0] = y;
      this->_state.x[1] = this->_state.x[0];
      this->_state.x[0] = x;
      return y;
    }
  };

  template <int nStages = 1>
  static inline CascadeCoeffs<nStages> calcCascadeCoeffs(
      const std::array<Settings, nStages>& ra_settings, signal_t fs) {
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

  struct EqBandMono final : public ComponentBase<Audio> {
    Settings settings;
    Coeffs5 coeffs;
    State state;

    Audio process(Audio x) noexcept override {
      return processBiquad5(x.l, this->coeffs, this->state);
    }
    void update() noexcept override {
      this->coeffs = calcCoeffs5(this->settings, this->_fs);
    }
    void reset(signal_t fs) noexcept override {
      this->_fs   = fs;
      this->state = { { 0, 0 }, { 0, 0 } };
      this->update();
    }
  };

  struct EqBand6Stereo final : public ComponentBase<Audio> {
    Biquad6 l;
    Biquad6 r;
    Settings settings;
    Coeffs6 coeffs;
    EqBand6Stereo() : l(&coeffs), r(&coeffs) { }
    Audio process(Audio x) noexcept override {
      return { this->l.process(x.l), this->r.process(x.r) };
    }
    void update() noexcept override {
      this->coeffs = calcCoeffs6(settings, this->_fs);
    }
    void reset(signal_t fs) noexcept override {
      this->_fs = fs;
      this->update();
    }
  };

  struct EqBand6Mono final : public ComponentBase<Audio> {
    Biquad6 l;
    Settings settings;
    Coeffs6 coeffs;
    EqBand6Mono() : l(&coeffs) { }
    Audio process(Audio x) noexcept override { return this->l.process(x.l); }
    void update() noexcept override {
      this->coeffs = calcCoeffs6(settings, this->_fs);
    }
    void reset(signal_t fs) noexcept override {
      this->_fs = fs;
      this->update();
    }
  };

  struct EqBandStereo final : public ComponentBase<Audio> {
    Settings settings;
    Coeffs5 coeffs;
    State stateL;
    State stateR;

    Audio process(Audio x) noexcept override {
      return { processBiquad5(x.l, this->coeffs, this->stateL),
        processBiquad5(x.r, this->coeffs, this->stateR) };
    }
    void update() noexcept override {
      this->coeffs = calcCoeffs5(this->settings, this->_fs);
    }
    void reset(signal_t fs) noexcept override {
      this->_fs    = fs;
      this->stateL = { { 0, 0 }, { 0, 0 } };
      this->stateR = { { 0, 0 }, { 0, 0 } };
      this->update();
    }
  };

  template <int nStages>
  struct Cascade final : public ComponentBase<Audio> {
    std::array<Settings, nStages> settings;
    CascadeCoeffs<nStages> coeffs;
    CascadeState<nStages> stateL;
    CascadeState<nStages> stateR;
    Audio process(Audio x) noexcept override {
      return { processCascade<nStages>(x.l, stateL, coeffs),
        processCascade<nStages>(x.r, stateR, coeffs) };
    }
    void update() noexcept override {
      this->coeffs = calcCascadeCoeffs<nStages>(this->settings, this->_fs);
    }
  };

  template <int order, Shape shape, class qTable>
  struct FilterBase : public ComponentBase<Audio> {
    constexpr static const int nStages = order / 2;
    Cascade<nStages> cascade;
    FirstOrder::StereoFilter<(
        shape == Shape::lpf ? FirstOrder::Shape::lpf : FirstOrder::Shape::hpf)>
        firstOrder;
    signal_t fc_hz { 20 };
    FilterBase() {
      if constexpr (std::is_same_v<qTable, LinkwitzTable>) {
        static_assert(order % 2 == 0, "LR filters only exist in even orders.");
      }
      static_assert(order >= 2, "Min order is 2.");
      static_assert(order <= 8, "Max order is 8.");
      static_assert(shape == Shape::lpf || shape == Shape::hpf,
          "Shape must be HPF or LPF.");
      for (size_t i = 0; i < nStages; i++) {
        this->cascade.settings[i].shape = shape;
      }
      if (order == 2) {
        this->cascade.settings[0].q = qTable::Second::q0;
      } else if (order == 3) {
        this->cascade.settings[0].q = qTable::Third::q0;
      } else if (order == 4) {
        this->cascade.settings[0].q = qTable::Fourth::q0;
        this->cascade.settings[1].q = qTable::Fourth::q1;
      } else if (order == 5) {
        this->cascade.settings[0].q = qTable::Fifth::q0;
        this->cascade.settings[1].q = qTable::Fifth::q1;
      } else if (order == 6) {
        this->cascade.settings[0].q = qTable::Sixth::q0;
        this->cascade.settings[1].q = qTable::Sixth::q1;
        this->cascade.settings[2].q = qTable::Sixth::q2;
      } else if (order == 7) {
        this->cascade.settings[0].q = qTable::Seventh::q0;
        this->cascade.settings[1].q = qTable::Seventh::q1;
        this->cascade.settings[2].q = qTable::Seventh::q2;
      } else if (order == 8) {
        this->cascade.settings[0].q = qTable::Eighth::q0;
        this->cascade.settings[1].q = qTable::Eighth::q1;
        this->cascade.settings[2].q = qTable::Eighth::q2;
        this->cascade.settings[3].q = qTable::Eighth::q3;
      }
    }
    Audio process(Audio x) noexcept override {
      if constexpr (order % 2) {
        return this->firstOrder.process(this->cascade.process(x));
      }
      return this->cascade.process(x);
    }
    void update() noexcept override {
      for (size_t i = 0; i < nStages; i++) {
        this->cascade.settings[i].fc_hz = fc_hz;
      }
      this->cascade.update();
      if constexpr (order % 2) {
        this->firstOrder.fc_hz = this->fc_hz;
        this->firstOrder.update();
      }
    }
    void reset(signal_t fs) noexcept override {
      this->cascade.reset(fs);
      if constexpr (order % 2) { this->firstOrder.reset(fs); }
      this->update();
    }
  };

  using ButterLpfSecond   = FilterBase<2, Shape::lpf, ButterworthTable>;
  using ButterLpfThird    = FilterBase<3, Shape::lpf, ButterworthTable>;
  using ButterLpfFourth   = FilterBase<4, Shape::lpf, ButterworthTable>;
  using ButterLpfFifth    = FilterBase<5, Shape::lpf, ButterworthTable>;
  using ButterLpfSixth    = FilterBase<6, Shape::lpf, ButterworthTable>;
  using ButterLpfSeventh  = FilterBase<7, Shape::lpf, ButterworthTable>;
  using ButterLpfEighth   = FilterBase<8, Shape::lpf, ButterworthTable>;
  using ButterHpfSecond   = FilterBase<2, Shape::hpf, ButterworthTable>;
  using ButterHpfThird    = FilterBase<3, Shape::hpf, ButterworthTable>;
  using ButterHpfFourth   = FilterBase<4, Shape::hpf, ButterworthTable>;
  using ButterHpfFifth    = FilterBase<5, Shape::hpf, ButterworthTable>;
  using ButterHpfSixth    = FilterBase<6, Shape::hpf, ButterworthTable>;
  using ButterHpfSeventh  = FilterBase<7, Shape::hpf, ButterworthTable>;
  using ButterHpfEighth   = FilterBase<8, Shape::hpf, ButterworthTable>;
  using LinkwitzLpfSecond = FilterBase<2, Shape::lpf, LinkwitzTable>;
  using LinkwitzLpfFourth = FilterBase<4, Shape::lpf, LinkwitzTable>;
  using LinkwitzLpfSixth  = FilterBase<6, Shape::lpf, LinkwitzTable>;
  using LinkwitzLpfEighth = FilterBase<8, Shape::lpf, LinkwitzTable>;
  using LinkwitzHpfSecond = FilterBase<2, Shape::hpf, LinkwitzTable>;
  using LinkwitzHpfFourth = FilterBase<4, Shape::hpf, LinkwitzTable>;
  using LinkwitzHpfSixth  = FilterBase<6, Shape::hpf, LinkwitzTable>;
  using LinkwitzHpfEighth = FilterBase<8, Shape::hpf, LinkwitzTable>;

  using EqBand6 = EqBand6Stereo;
  using EqBand  = EqBandStereo;
}
}