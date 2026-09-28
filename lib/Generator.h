#pragma once

/**
 * @file Generator.h
 * @author Niels Thøgersen (niels.thoegersen@gmail.com)
 * @brief Audio component producing white noise.
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
#include "lib/SoftClip.h"
#include "lib/Tilt.h"
#include <cstddef>

namespace NtFx {

namespace Generator {
  /**
   * @brief Saw wave generator.
   *
   * @tparam T Datatype.
   * @param x Input in radians. Same as input for 'sin' function.
   * @return T Output.
   */
  template <typename T>
  static inline T saw(T x) {
    const T alpha = 2 / NTFX_PI;
    T x_          = gcem::fmod(x, T(2.0) * NTFX_PI);
    x_            = (x_ < 0 ? x_ + 2 * NTFX_PI : x_);
    T y;
    if (x_ < 0.5 * NTFX_PI) {
      y = x_ * alpha;
    } else if (x_ < 1.5 * NTFX_PI) {
      y = -x_ * alpha + 2;
    } else {
      y = x_ * alpha - 4;
    }
    return y;
  }

  /**
   * @brief Saw wave generator.
   *
   * @tparam T Datatype.
   * @param x Input in radians. Same as input for 'sin' function.
   * @return T Output.
   */
  template <typename T>
  static inline T square(T x) {
    T x_ = gcem::fmod(x, T(2.0) * NTFX_PI);
    x_   = (x_ < 0 ? x_ + 2 * NTFX_PI : x_);
    T y;
    if (x_ < NTFX_PI) {
      y = 1;
    } else {
      y = -1;
    }
    return y;
  }

  /**
   * @brief Pseudorandom unsigned long.
   *
   * Marsaglia, George (2003) "Random Number Generators,"Journal of Modern
   * Applied Statistical Methods: Vol. 2 : Iss. 1 , Article 2.
   * DOI: 10.22237/jmasm/1051747320
   *
   * @return unsigned long
   */
  static inline uint64_t KISS() noexcept {
    static uint64_t x = 123456789, y = 362436000, z = 521288629, c = 7654321, t;
    x = 69069 * x + 12345;
    y ^= y << 13;
    y ^= y >> 17;
    y ^= y << 5;
    t = 698769069ULL * z + c;
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
    const uint64_t uintMax = std::numeric_limits<uint64_t>::max();
    return T(KISS() - (uintMax >> 1)) / T(uintMax) * 2 - 1;
  }

  struct WhiteNoise final : public ComponentBase<Audio> {
    Audio process(Audio) noexcept override {
      return { rand<signal_t>(), rand<signal_t>() };
    }
  };
  struct FilteredWhiteNoise : public ComponentBase<Audio> {
    FirstOrder::StereoFilter<FirstOrder::Shape::lpfZero> lpf;
    FirstOrder::StereoFilter<FirstOrder::Shape::hpf> hpf;
    FilteredWhiteNoise() {
      this->lpf.fc_hz = 20e3;
      this->hpf.fc_hz = 20;
    }
    FilteredWhiteNoise(FilteredWhiteNoise&)                  = default;
    FilteredWhiteNoise(FilteredWhiteNoise&&)                 = default;
    virtual ~FilteredWhiteNoise()                            = default;
    FilteredWhiteNoise& operator=(FilteredWhiteNoise const&) = default;
    FilteredWhiteNoise& operator=(FilteredWhiteNoise&&)      = default;
    Audio process(Audio) noexcept override {
      return this->lpf.process(
          this->hpf.process({ rand<signal_t>(), rand<signal_t>() }));
    }
    void update() noexcept override {
      this->lpf.update();
      this->hpf.update();
    }
    void reset(signal_t fs) noexcept override {
      this->lpf.reset(fs);
      this->hpf.reset(fs);
    }
  };
  struct PinkNoise final : public ComponentBase<Audio> {
    Tilt<> tilt;
    Clip::Hard clip;
    PinkNoise() { this->tilt.tilt_db = -10; }
    Audio process(Audio) noexcept override {
      return clip.process(tilt.process({ rand<signal_t>(), rand<signal_t>() }));
    }
    void update() noexcept override { this->tilt.update(); }
    void reset(signal_t fs) noexcept override { this->tilt.reset(fs); }
  };
  struct FilteredPinkNoise final : public ComponentBase<Audio> {
    Tilt<> tilt;
    FirstOrder::StereoFilter<FirstOrder::Shape::lpfZero> lpf;
    FirstOrder::StereoFilter<FirstOrder::Shape::hpf> hpf;
    FilteredPinkNoise() {
      this->lpf.fc_hz    = 20e3;
      this->hpf.fc_hz    = 20;
      this->tilt.tilt_db = -10;
    }
    Audio process(Audio) noexcept override {
      return this->lpf.process(this->hpf.process(
          this->tilt.process({ rand<signal_t>(), rand<signal_t>() })));
    }
    void update() noexcept override {
      this->lpf.update();
      this->hpf.update();
      this->tilt.update();
    }
    void reset(signal_t fs) noexcept override {
      this->lpf.reset(fs);
      this->hpf.reset(fs);
      this->tilt.reset(fs);
    }
  };
  struct Sin final : public ComponentBase<Audio> {
    signal_t f_hz { 1e3 };
    signal_t ph_rad { 0 };
    size_t _i { 0 };
    size_t _n { 1 };
    signal_t _w { 0 };
    Audio process(Audio) noexcept override {
      auto w = this->_w * signal_t(this->_i) + this->ph_rad;
      if (++this->_i >= _n) { _i = 0; }
      return gcem::sin(w);
    }
    void update() noexcept override {
      this->_n = size_t(this->_fs / this->f_hz);
      this->_w = 2 * NTFX_PI * this->f_hz / this->_fs;
    }
    void reset(signal_t fs) noexcept override {
      this->_fs = fs;
      this->update();
    }
  };
  struct Saw final : public ComponentBase<Audio> {
    signal_t f_hz { 1e3 };
    signal_t ph_rad { 0 };
    size_t _i { 0 };
    signal_t _w { 0 };
    Audio process(Audio) noexcept override {
      auto w = this->_w * signal_t(this->_i++) + this->ph_rad;
      return saw(w);
    }
    void update() noexcept override {
      this->_w = 2 * NTFX_PI * this->f_hz / this->_fs;
    }
    void reset(signal_t fs) noexcept override {
      this->_fs = fs;
      this->update();
    }
  };
  struct Square final : public ComponentBase<Audio> {
    signal_t f_hz { 1e3 };
    signal_t ph_rad { 0 };
    size_t _i { 0 };
    signal_t _w { 0 };
    Audio process(Audio) noexcept override {
      auto w = this->_w * signal_t(this->_i++) + this->ph_rad;
      return square(w);
    }
    void update() noexcept override {
      this->_w = 2 * NTFX_PI * this->f_hz / this->_fs;
    }
    void reset(signal_t fs) noexcept override {
      this->_fs = fs;
      this->update();
    }
  };
}
}