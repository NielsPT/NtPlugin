/**
 * @file FirstOrder.h
 * @author Niels Thøgersen (niels.thoegersen@gmail.com)
 * @brief First order high- and low pass filters.
 * @version 0.1
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

#include "lib/Component.h"
#include "lib/Stereo.h"
#include <math.h>

namespace NtFx {
namespace FirstOrder {
  /**
   * @brief Filter shape for first order filters. 'none' denoted bypass and
   * 'lpfZero' has a zero added at Nyquist.
   *
   */
  enum class Shape {
    none,
    lpf,
    lpfZero,
    hpf,
  };

  /**
   * @brief A single channel first order filter.
   *
   * @tparam signal_t Audio datatype.
   * @tparam shape Type of filter.
   */
  template <typename signal_t, Shape shape>
  struct Filter : public Component<signal_t> {
    signal_t fc_hz = 1000;
    signal_t _a    = 0;
    signal_t _yn1  = 0;
    signal_t _xn1  = 0;

    virtual signal_t process(signal_t x) noexcept override {
      signal_t y;
      if constexpr (shape == Shape::none) {
        return x;
      } else if constexpr (shape == Shape::lpf) {
        y = this->_a * x + (1 - this->_a) * _yn1;
      } else if constexpr (shape == Shape::hpf) {
        y = this->_a * (this->_yn1 + x - this->_xn1);
      } else {
        y = signal_t(0.5) * this->_a * (x + _xn1) + (1 - this->_a) * _yn1;
      }
      this->_xn1 = x;
      this->_yn1 = y;
      return y;
    }

    virtual void update() noexcept override {
      if (this->fs <= 0) { return; }
      signal_t z = 2 * GCEM_PI * this->fc_hz / this->fs;
      if constexpr (shape == Shape::hpf) {
        this->_a = 1.0 / (z + 1.0);
      } else {
        this->_a = z / (z + 1.0);
      }
    }

    virtual void reset(float fs) noexcept override {
      this->fs   = fs;
      this->_xn1 = 0;
      this->_yn1 = 0;
      this->update();
    }
  };

  /**
   * @brief Stereo pair of first order filters.
   *
   * @tparam signal_t Audio datatype.
   * @tparam shape Filter shape.
   */
  template <typename signal_t, Shape shape>
  struct StereoFilter : public Component<Stereo<signal_t>> {
    Filter<signal_t, shape> l;
    Filter<signal_t, shape> r;

    Stereo<signal_t> process(Stereo<signal_t> x) noexcept override {
      return { l.process(x.l), r.process(x.r) };
    }

    virtual void update() noexcept override {
      l.update();
      r.update();
    }

    virtual void reset(float fs) noexcept override {
      l.reset(fs);
      r.reset(fs);
    }

    void setFc(signal_t fc) noexcept {
      l.fc_hz = fc;
      r.fc_hz = fc;
    }
  };
}
}