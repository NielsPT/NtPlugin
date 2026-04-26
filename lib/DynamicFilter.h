#pragma once

/**
 * @file DynamicFilter.h
 * @author Niels Thøgersen (niels.thoegersen@gmail.com)
 * @brief Dynamic biquad filter.
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
 * You are free to download, build and use this code for commercial
 * purposes. Just don't resell it or a build of it, modified or otherwise.
 */

#include "gcem.hpp"
#include "lib/Biquad.h"
#include "lib/Component.h"
#include "lib/Stereo.h"

namespace NtFx {
template <typename signal_t>
struct DynamidShelf : public Component<Stereo<signal_t>> {
  NtFx::Biquad::BiQuad6Stereo<signal_t> flt;
  signal_t fc_hz { 2000 };
  signal_t alpha2 { 0 };
  signal_t beta { 0 };
  signal_t gain_lin { 1 };
  DynamidShelf() { }
  virtual Stereo<signal_t> process(Stereo<signal_t> x) noexcept override {
    auto A4               = signal_t(4) * gain_lin;
    auto z                = this->beta * gcem::sqrt(this->gain_lin);
    this->flt.coeffs.b[0] = this->alpha2 + z + A4;
    this->flt.coeffs.b[1] = signal_t(2) * (this->alpha2 - A4);
    this->flt.coeffs.b[2] = this->alpha2 - z + A4;
    return flt.process(x);
  }
  virtual void update() noexcept override {
    auto alpha            = signal_t(2) * GCEM_PI * this->fc_hz / this->fs;
    this->alpha2          = alpha * alpha;
    this->beta            = signal_t(2) * alpha * gcem::sqrt(signal_t(2));
    this->flt.coeffs.a[0] = this->alpha2 + this->beta + 4;
    this->flt.coeffs.a[1] = signal_t(2) * this->alpha2 - 8;
    this->flt.coeffs.a[2] = this->alpha2 - this->beta + 4;
  }
  virtual void reset(float fs) noexcept override {
    this->fs = fs;
    this->flt.reset(fs);
    this->update();
  }
};
}