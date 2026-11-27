#pragma once

/**
 * @file FirstOrder.h
 * @author Niels Thøgersen (niels.thoegersen@gmail.com)
 * @brief First order high- and low pass filters.
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
 *
 */

#include "lib/Audio.h"
#include "lib/Component.h"
#include "lib/gcem.h"
#include "lib/utils.h"

namespace NtFx {
namespace FirstOrder {
  /**
   * @brief Filter shape for first order filters. 'none' denoted bypass and
   * 'lpfZero' has a zero added at Nyquist.
   *
   */
  enum class Shape { none, lpf, lpfZero, hpf };

  /**
   * @brief A single channel first order filter.
   *
   * @tparam signal_t Audio datatype.
   * @tparam shape Type of filter.
   */
  template <Shape shape, class audio_t = Audio>
  struct Filter : public ComponentBase<audio_t> {
    signal_t fc_hz { 1000 };
    signal_t _a { 0 };
    audio_t _yn1 { 0 };
    audio_t _xn1 { 0 };

    audio_t process(audio_t x) noexcept override {
      audio_t y { 0 };
      if constexpr (shape == Shape::none) {
        return x;
      } else if constexpr (shape == Shape::lpf) {
        y = this->_a * x + (1 - this->_a) * _yn1;
      } else if constexpr (shape == Shape::hpf) {
        y = this->_a * (this->_yn1 + x - this->_xn1);
      } else {
        y = audio_t(0.5) * this->_a * (x + _xn1) + (1 - this->_a) * _yn1;
      }
      this->_xn1 = x;
      this->_yn1 = y;
      return y;
    }

    void update() noexcept override {
      if (this->_fs <= 0) { return; }
      signal_t z = 2 * NTFX_PI * this->fc_hz / this->_fs;
      if constexpr (shape == Shape::hpf) {
        this->_a = 1.0 / (z + 1.0);
      } else {
        this->_a = z / (z + 1.0);
      }
    }

    void reset(signal_t fs) noexcept override {
      this->_fs  = fs;
      this->_xn1 = 0;
      this->_yn1 = 0;
      this->update();
    }
  };

  template <class audio_t>
  struct LoShelf : public ComponentBase<Audio> {
    Filter<Shape::lpf> _filter;
    signal_t gain_db { 0 };
    signal_t fc_hz { 200 };
    signal_t _gain_lin { 0 };
    Audio process(Audio x) noexcept override {
      return this->_filter.process(x) * this->_gain_lin + x;
    }
    void update() noexcept override {
      auto a              = invDb(this->gain_db);
      this->_gain_lin     = a - 1;
      this->_filter.fc_hz = this->fc_hz / gcem::sqrt(a);
      _filter.update();
    }
    void reset(signal_t fs) noexcept override {
      this->_fs = fs;
      _filter.reset(fs);
      this->update();
    }
  };

  template <class audio_t>
  struct HiShelf : public ComponentBase<Audio> {
    Filter<Shape::hpf> _filter;
    signal_t gain_db { 0 };
    signal_t fc_hz { 10000 };
    signal_t _gain_lin { 0 };
    Audio process(Audio x) noexcept override {
      return this->_filter.process(x) * this->_gain_lin + x;
    }
    void update() noexcept override {
      auto a              = invDb(this->gain_db);
      this->_gain_lin     = a - 1;
      this->_filter.fc_hz = this->fc_hz * gcem::sqrt(a);
      _filter.update();
    }
    void reset(signal_t fs) noexcept override {
      this->_fs = fs;
      _filter.reset(fs);
      this->update();
    }
  };

  template <class audio_t>
  struct ToneControl : public ComponentBase<audio_t> {
    Filter<Shape::lpfZero, audio_t> _lpf;
    Filter<Shape::hpf, audio_t> _hpf;
    signal_t fc_hz { 632 };
    signal_t gain_db { 0 };
    signal_t _gainHf_lin { 1 };
    signal_t _gainLf_lin { 1 };
    ToneControl() { }
    audio_t process(audio_t x) noexcept override {
      return this->_lpf.process(x) * this->_gainLf_lin
          + this->_hpf.process(x) * this->_gainHf_lin;
    }
    void update() noexcept override {
      if (this->gain_db > 0) {
        this->_gainHf_lin = 1;
        this->_gainLf_lin = invDb(-gain_db);
      } else {
        this->_gainLf_lin = 1;
        this->_gainHf_lin = invDb(gain_db);
      }
      this->_lpf.fc_hz = this->fc_hz;
      this->_hpf.fc_hz = this->fc_hz;
      this->_lpf.update();
      this->_hpf.update();
    }
    void reset(signal_t fs) noexcept override {
      this->_fs = fs;
      this->_lpf.reset(fs);
      this->_hpf.reset(fs);
      this->update();
    }
  };
}
}