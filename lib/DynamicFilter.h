#pragma once

/**
 * @file DynamicFilter.h
 * @author Niels Thøgersen (niels.thoegersen@gmail.com)
 * @brief Dynamic biquad filter.
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

#include "lib/Audio.h"
#include "lib/Biquad.h"
#include "lib/Component.h"
#include "lib/gcem.h"

namespace NtFx {
namespace DynamicFilter {
  /**
   * @brief An optimized dynamic shelving filter, which exposes both Q values.
   * Feedback coeffs are calculated in update, while feed forward is calculated
   * in process, so the gain of the filter can be updated while running, making
   * the filter usefull for dynamic processing. Gain is in the linear domain for
   * the same reason.
   *
   * @tparam signal_t
   */
  struct ShelfBase : public ComponentBase<Audio> {
    Biquad::EqBand6 _flt;
    signal_t fc_hz { 2000 }; ///< Cutoff frequency.
    signal_t gain_lin { 1 }; ///< Gain in linear domain.
    signal_t q1 { 0.707 };   ///< Q at the cutoff frequency.
    signal_t q2 { 0.707 };   ///< Q where the response flattens.
    signal_t _alphaSquared { 0 };
    signal_t _beta1 { 0 };
    signal_t _beta2 { 0 };

    Audio process(Audio x) noexcept override {
      this->calcCoeffsProcess();
      return _flt.process(x);
    }

    void update() noexcept override {
      auto alpha = signal_t(2.0) * signal_t(GCEM_PI) * this->fc_hz / this->_fs;
      this->_alphaSquared = alpha * alpha;
      this->_beta1        = signal_t(2) * alpha / this->q1;
      this->_beta2        = signal_t(2) * alpha / this->q2;
      this->calcCoeffsUpdate();
    }

    void reset(signal_t fs) noexcept override {
      this->_fs = fs;
      this->_flt.reset(fs);
      this->update();
    }

    virtual void calcCoeffsUpdate() noexcept  = 0;
    virtual void calcCoeffsProcess() noexcept = 0;
  };

  struct ShelfFixedPoles : public ShelfBase {
    void calcCoeffsUpdate() noexcept override {
      this->_flt.coeffs.a[0] = this->_alphaSquared + this->_beta1 + 4;
      this->_flt.coeffs.a[1] = signal_t(2) * this->_alphaSquared - 8;
      this->_flt.coeffs.a[2] = this->_alphaSquared - this->_beta1 + 4;
    }

    void calcCoeffsProcess() noexcept override {
      auto A4                = signal_t(4) * this->gain_lin;
      auto z                 = this->_beta2 * gcem::sqrt(this->gain_lin);
      this->_flt.coeffs.b[0] = this->_alphaSquared + z + A4;
      this->_flt.coeffs.b[1] = signal_t(2) * (this->_alphaSquared - A4);
      this->_flt.coeffs.b[2] = this->_alphaSquared - z + A4;
    }
  };

  struct ShelfFixedZeros : public ShelfBase {
    void calcCoeffsUpdate() noexcept override {
      this->_flt.coeffs.b[0] = this->_alphaSquared + this->_beta2 + 4;
      this->_flt.coeffs.b[1] = signal_t(2) * (this->_alphaSquared - 4);
      this->_flt.coeffs.b[2] = this->_alphaSquared - this->_beta2 + 4;
    }

    void calcCoeffsProcess() noexcept override {
      auto A4                = signal_t(4) / this->gain_lin;
      auto z                 = this->_beta1 * gcem::sqrt(1 / this->gain_lin);
      this->_flt.coeffs.a[0] = this->_alphaSquared + z + A4;
      this->_flt.coeffs.a[1] = signal_t(2) * (this->_alphaSquared - A4);
      this->_flt.coeffs.a[2] = this->_alphaSquared - z + A4;
    }
  };
}
}