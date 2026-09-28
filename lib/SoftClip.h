#pragma once

/**
 * @file Soft.h
 * @author Niels Thøgersen (niels.thoegersen@gmail.com)
 * @brief Soft clippers for audio processing. Calculates coeffs at compile time
 * and applies polynomials to signals.
 *
 * @copyright Copyright (c) 2026
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
 */

#include "lib/Audio.h"
#include "lib/Component.h"
#include "lib/FirstOrder.h"
#include "lib/utils.h"
#include <array>

namespace NtFx {
namespace Clip {
  static inline signal_t hardMono(signal_t x) {
    return (x > 1 ? 1 : x < -1 ? -1 : x);
  }

  static inline Audio hardStereo(Audio x) noexcept {
    return { hardMono(x.l), hardMono(x.r) };
  }

  static inline signal_t alt1(signal_t x) {
    auto x_ = x * 0.5;
    if (x_ > 1.0) { return signal_t(1.0); }
    if (x_ < 0.0) { return x; }
    return x - x_ * x_;
  }

  static inline Audio alt1Stereo(Audio x) { return { alt1(x.l), alt1(x.r) }; }

  static inline signal_t secondMono(signal_t x, signal_t a) {
    auto x_ = x / 1.5;
    return x_ + a * (x_ * x_ - 0.5);
  }
  static inline Audio secondStereo(Audio x, signal_t a) {
    return { secondMono(x.l, a), secondMono(x.r, a) };
  }
  /**
   * @brief Calculates coefficients for symmetrical soft clipper
   * at compile time.
   *
   * @tparam signal_t Audio signal type.
   * @tparam N. Determines order. Order = 2 * N + 1.
   * @return consteval array coefficients. Length N + 1;
   */
  template <typename signal_t, size_t N>
  consteval inline std::array<signal_t, N + 1> _calculateSoftCoeffs() noexcept {
    std::array<signal_t, N + 1> a_n;
    for (int n = 0; n < N + 1; n++) {
      a_n[n] = gcem::pow(-1, n) * gcem::tgamma((2 * N + 1) + 1)
          / (gcem::pow(4, N) * gcem::tgamma(N + 1) * (2 * n + 1)
              * gcem::tgamma(n + 1) * gcem::tgamma(N - n + 1));
    }
    return a_n;
  }

  constexpr std::array<signal_t, 4> _coeffsSeventh =
      _calculateSoftCoeffs<signal_t, 3>();

  constexpr std::array<signal_t, 3> _coeffsFifth =
      _calculateSoftCoeffs<signal_t, 2>();

  constexpr std::array<signal_t, 2> _coeffsThird =
      _calculateSoftCoeffs<signal_t, 1>();

  /**
   * @brief Applied soft clipping using a third order polynomial.
   *
   * @param x Input sample.
   * @return signal_t Output sample.
   */
  static inline signal_t soft3rdMono(signal_t x) {
    auto x_ = x / _coeffsThird[0];
    if (x_ > 1.0) { return signal_t(1.0); }
    if (x_ < -1.0) { return signal_t(-1.0); }
    auto x3 = x_ * x_ * x_;
    return x + _coeffsThird[1] * x3;
  }

  /**
   * @brief Applied soft clipping using a third order polynomial on a stereo
   * signal.
   *
   * @param x Input sample.
   * @return signal_t Output sample.
   */
  static inline Audio soft3rdStereo(Audio x) {
    return { soft3rdMono(x.l), soft3rdMono(x.r) };
  }

  /**
   * @brief Applied soft clipping using a fifth order polynomial.
   *
   * @param x Input sample.
   * @return signal_t Output sample.
   */
  static inline signal_t soft5thMono(signal_t x) noexcept {
    signal_t x_ = x / _coeffsFifth[0];
    if (x_ > 1.0) { return signal_t(1.0); }
    if (x_ < -1.0) { return signal_t(-1.0); }
    auto x3 = x_ * x_ * x_;
    auto x5 = x3 * x_ * x_;
    return x + _coeffsFifth[1] * x3 + _coeffsFifth[2] * x5;
  }

  /**
   * @brief Applied soft clipping using a fifth order polynomial on a stereo
   * signal.
   *
   * @param x Input sample.
   * @return signal_t Output sample.
   */
  static inline Audio soft5thStereo(Audio x) noexcept {
    return { soft5thMono(x.l), soft5thMono(x.r) };
  }

  /**
   * @brief Fifth order soft clipper wrapped in a Component.
   *
   */
  struct SoftBase : public ComponentBase<Audio> {
    signal_t gain_db { 0 };
    signal_t gainIn_lin { 1 };
    signal_t gainOut_lin { 1 };
    SoftBase()                           = default;
    SoftBase(const SoftBase&)            = default;
    SoftBase(SoftBase&&)                 = default;
    virtual ~SoftBase()                  = default;
    SoftBase& operator=(const SoftBase&) = default;
    SoftBase& operator=(SoftBase&&)      = default;
    void update() noexcept override {
      this->gainIn_lin  = invDb(this->gain_db);
      this->gainOut_lin = 1 / invDb(this->gain_db);
      if (this->gain_db > 0) {
        this->gainOut_lin = 1 / invDb(this->gain_db * 0.5);
      }
    }
  };

  struct Hard final : public SoftBase {
    Audio process(Audio x) noexcept override {
      return hardStereo(x * this->gainIn_lin) * this->gainOut_lin;
    }
  };

  struct Alt1 final : public SoftBase {
    Audio process(Audio x) noexcept override {
      return alt1Stereo(x * this->gainIn_lin) * this->gainOut_lin;
    }
  };

  struct Second final : public SoftBase {
    FirstOrder::StereoFilter<FirstOrder::Shape::hpf> hpf;
    Audio process(Audio x) noexcept override {
      return hpf.process(secondStereo(x, this->gainIn_lin / 16));
    }
    void update() noexcept override {
      this->SoftBase::update();
      this->hpf.update();
    }
    void reset(signal_t fs) noexcept override {
      this->_fs       = fs;
      this->hpf.fc_hz = 20;
      this->hpf.reset(fs);
      this->update();
    }
  };

  /**
   * @brief Third order soft clipper wrapped in a Component.
   *
   */
  struct Soft3 final : public SoftBase {
    Audio process(Audio x) noexcept override {
      return soft3rdStereo(x * this->gainIn_lin) * this->gainOut_lin;
    }
  };

  /**
   * @brief Fifth order soft clipper wrapped in a Component.
   *
   */
  struct Soft5 final : public SoftBase {
    Audio process(Audio x) noexcept override {
      return soft5thStereo(x * gainIn_lin) * gainOut_lin;
    }
  };

  /**
   * @brief Base class for soft clipper with rudimental antialiasing filter.
   *
   */
  struct SoftAntialiasBase : public SoftBase {
    FirstOrder::StereoFilter<FirstOrder::Shape::lpfZero> lpf;
    FirstOrder::StereoFilter<FirstOrder::Shape::hpf> hpf;
    FirstOrder::StereoFilter<FirstOrder::Shape::hpf> hpf2;

    bool bypass1Enable { true };
    bool bypass2Enable { true };
    SoftAntialiasBase()                                    = default;
    SoftAntialiasBase(const SoftAntialiasBase&)            = default;
    SoftAntialiasBase(SoftAntialiasBase&&)                 = default;
    ~SoftAntialiasBase() override                          = default;
    SoftAntialiasBase& operator=(const SoftAntialiasBase&) = default;
    SoftAntialiasBase& operator=(SoftAntialiasBase&&)      = default;
    void update() noexcept override {
      this->gainIn_lin  = invDb(this->gain_db);
      this->gainOut_lin = 1 / invDb(this->gain_db);
      if (this->gainOut_lin < 0.5) { this->gainOut_lin = 0.5; }
      this->lpf.update();
      this->hpf.update();
      this->hpf2.update();
    }
    void reset(signal_t fs) noexcept override {
      this->_fs        = fs;
      this->lpf.fc_hz  = fs / 8;
      this->hpf.fc_hz  = fs / 8;
      this->hpf2.fc_hz = fs / 2;
      this->lpf.reset(fs);
      this->hpf.reset(fs);
      this->hpf2.reset(fs);
      this->update();
    }
  };

  /**
   * @brief Third order soft clipper with rudimental antialiasing filter.
   *
   */
  struct SoftAntialias3 final : public SoftAntialiasBase {
    Audio process(Audio x) noexcept override {
      return soft3rdStereo(this->lpf.process(x) * gainIn_lin) * gainOut_lin
          + this->hpf.process(x) * this->bypass1Enable
          + this->hpf2.process(x) * this->bypass2Enable;
    }
  };

  /**
   * @brief Fifth order soft clipper with rudimental antialiasing filter.
   *
   */
  struct SoftAntialias5 final : public SoftAntialiasBase {
    Audio process(Audio x) noexcept override {
      return soft5thStereo(this->lpf.process(x) * gainIn_lin) * gainOut_lin
          + this->hpf.process(x) * this->bypass1Enable
          + this->hpf2.process(x) * this->bypass2Enable;
    }
  };

} // namespace Clip
} // namespace NtFx
