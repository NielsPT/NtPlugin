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
#include "lib/Component.h"
#include "lib/Stereo.h"
#include <math.h>

namespace NtFx {
namespace FirstOrder {
  enum class Shape {
    none,
    lpf,
    lpfZero,
    hpf,
  };

  template <typename signal_t, Shape shape>
  struct Filter : public Component<signal_t> {
    signal_t fc_hz = 1000;
    signal_t alpha = 0;
    signal_t yn1   = 0;
    signal_t xn1   = 0;
    virtual signal_t process(signal_t x) noexcept override {
      signal_t y;
      if constexpr (shape == Shape::none) {
        return x;
      } else if constexpr (shape == Shape::lpf) {
        y = this->alpha * x + (1 - this->alpha) * yn1;
      } else if constexpr (shape == Shape::hpf) {
        y = this->alpha * (this->yn1 + x - this->xn1);
      } else {
        y = signal_t(0.5) * this->alpha * (x + xn1) + (1 - this->alpha) * yn1;
      }
      this->xn1 = x;
      this->yn1 = y;
      return y;
    }

    virtual void update() noexcept override {
      if (this->fs <= 0) { return; }
      signal_t z = 2 * GCEM_PI * this->fc_hz / this->fs;
      if constexpr (shape == Shape::hpf) {
        this->alpha = 1.0 / (z + 1.0);
      } else {
        this->alpha = z / (z + 1.0);
      }
    }

    virtual void reset(float fs) noexcept override {
      this->fs  = fs;
      this->xn1 = 0;
      this->yn1 = 0;
      this->update();
    }
  };

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