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

#include "lib/Component.h"
#include "lib/Stereo.h"
#include "lib/utils.h"

#include "lib/gcem/include/gcem.hpp"
namespace NtFx {
template <typename signal_t, bool linDomain = false>
struct SideChain : public Component<Stereo<signal_t>> {
  struct Settings {
    signal_t thresh_db = static_cast<signal_t>(0);
    signal_t ratio_db  = static_cast<signal_t>(2);
    signal_t knee_db   = static_cast<signal_t>(12);
    signal_t tAtt_ms   = static_cast<signal_t>(1);
    signal_t tRel_ms   = static_cast<signal_t>(100);
    signal_t tRms_ms   = static_cast<signal_t>(80);
    signal_t tPeak_ms  = static_cast<signal_t>(20.0);
    bool rmsEnable     = false;
  };

  struct Coeffs {
    signal_t thresh_lin = static_cast<signal_t>(1);
    signal_t ratio_lin  = static_cast<signal_t>(1);
    signal_t knee_lin   = static_cast<signal_t>(1);
    signal_t alphaAtt   = static_cast<signal_t>(0);
    signal_t alphaRel   = static_cast<signal_t>(0);
    signal_t alphaPeak  = static_cast<signal_t>(0);
    size_t nRms         = 1;
  };

  struct State {
    signal_t ySensLast   = static_cast<signal_t>(0.0);
    signal_t yFilterLast = static_cast<signal_t>(0.0);
    RmsSensorState<signal_t> rms;
    void reset() {
      this->ySensLast   = static_cast<signal_t>(0.0);
      this->yFilterLast = static_cast<signal_t>(0.0);
      rms.reset();
    }
  };

  Settings settings;
  Coeffs coeffs;
  State stateL;
  State stateR;
  virtual Stereo<signal_t> process(Stereo<signal_t> x) noexcept override {
    if constexpr (linDomain) {
      return { this->sideChain_lin(x.l, stateL),
        this->sideChain_lin(x.r, stateR) };
    }
    return { this->sideChain_db(x.l, stateL), this->sideChain_db(x.r, stateR) };
  }

  virtual void update() noexcept override {
    this->coeffs.alphaAtt =
        gcem::exp(-2200.0 / (this->settings.tAtt_ms * this->fs));
    this->coeffs.alphaRel =
        gcem::exp(-2200.0 / (this->settings.tRel_ms * this->fs));
    if (this->coeffs.alphaRel < this->coeffs.alphaAtt) {
      this->coeffs.alphaRel = this->coeffs.alphaAtt;
    }
    this->coeffs.alphaPeak =
        gcem::exp(-2200.0 / (this->settings.tPeak_ms * this->fs));
    this->coeffs.thresh_lin =
        gcem::pow(10.0, (this->settings.thresh_db / 20.0));
    this->coeffs.knee_lin  = gcem::pow(10.0, (this->settings.knee_db / 20.0));
    signal_t oneOverSqrt2  = 1.0 / gcem::sqrt(2.0);
    this->coeffs.ratio_lin = (1.0 - 1.0 / this->settings.ratio_db)
        * (oneOverSqrt2
            - gcem::pow(
                oneOverSqrt2 - (this->settings.ratio_db - 3.0) / 18.0, 5.0));
    this->coeffs.nRms = std::floor(this->settings.tRms_ms * this->fs * 0.001);
  }

  virtual void reset(float fs) noexcept override {
    this->fs = fs;
    this->update();
  }

  inline signal_t sideChain_lin(signal_t x, State& p_state) noexcept {
    signal_t xAbs;
    if (this->settings.rmsEnable) {
      xAbs = rmsSensor(&p_state.rms, this->coeffs.nRms, x);
    } else {
      xAbs = gcem::abs(x);
    }

    signal_t sensRelease = this->coeffs.alphaPeak * p_state.ySensLast
        + (1 - this->coeffs.alphaPeak) * xAbs;
    signal_t ySens    = std::max(xAbs, sensRelease);
    p_state.ySensLast = ySens;

    signal_t target;
    if (ySens < this->coeffs.thresh_lin / this->coeffs.knee_lin) {
      target = static_cast<signal_t>(0);
    } else if (ySens < this->coeffs.thresh_lin) {
      target = (ySens / this->coeffs.thresh_lin) * this->coeffs.ratio_lin
          * this->coeffs.thresh_lin / (this->coeffs.knee_lin * ySens);
    } else {
      target = (ySens / this->coeffs.thresh_lin) * this->coeffs.ratio_lin;
    }

    signal_t alpha = this->coeffs.alphaRel;
    if (target > p_state.yFilterLast) { alpha = this->coeffs.alphaAtt; }

    signal_t yFilter    = p_state.yFilterLast * alpha + target * (1 - alpha);
    p_state.yFilterLast = yFilter;
    return static_cast<signal_t>(1.0) / (yFilter + 1);
  }

  inline signal_t sideChain_db(signal_t x, State& p_state) noexcept {
    signal_t xAbs;
    if (this->settings.rmsEnable) {
      xAbs = rmsSensor(&p_state.rms, this->coeffs.nRms, x);
    } else {
      xAbs = gcem::abs(x);
    }

    signal_t sensRelease = this->coeffs.alphaPeak * p_state.ySensLast
        + (1 - this->coeffs.alphaPeak) * xAbs;
    signal_t ySens = std::max(xAbs, sensRelease);
    if (ySens != ySens) { ySens = static_cast<signal_t>(0); }
    p_state.ySensLast = ySens;

    signal_t x_db = db(ySens);
    signal_t y_db;
    if ((x_db - this->settings.thresh_db) > (this->settings.knee_db / 2)) {
      y_db = this->settings.thresh_db
          + (x_db - this->settings.thresh_db) / this->settings.ratio_db;
    } else if ((x_db - this->settings.thresh_db)
        < -(this->settings.knee_db / 2)) {
      y_db = x_db;
    } else {
      signal_t tmp =
          (x_db - this->settings.thresh_db + this->settings.knee_db / 2);
      y_db = x_db
          + (1 / this->settings.ratio_db - 1) * tmp * tmp
              / (2 * this->settings.knee_db);
    }

    signal_t target = x_db - y_db;

    signal_t alpha = this->coeffs.alphaRel;
    if (target > p_state.yFilterLast) { alpha = this->coeffs.alphaAtt; }

    signal_t yFilter = p_state.yFilterLast * alpha + target * (1 - alpha);
    if (yFilter != yFilter) { yFilter = static_cast<signal_t>(0); }
    p_state.yFilterLast = yFilter;
    return invDb(-yFilter);
  }
};
} // namespace NtFx
