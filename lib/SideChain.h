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
#include "lib/PeakSensor.h"
#include "lib/RmsSensor.h"
#include "lib/Stereo.h"
#include "lib/utils.h"

#include "gcem.hpp"

namespace NtFx {
template <typename signal_t>
struct ScSettings {
  signal_t thresh_db = signal_t(0);
  signal_t ratio_db  = signal_t(2);
  signal_t knee_db   = signal_t(12);
  signal_t tAtt_ms   = signal_t(1);
  signal_t tRel_ms   = signal_t(100);
  signal_t tRms_ms   = signal_t(80);
  signal_t tPeak_ms  = signal_t(20.0);
};

template <typename signal_t>
struct PeakSideChainDb : public Component<Stereo<signal_t>> {
  PeakSensorStereo<signal_t> peakSensor;
  signal_t alphaAtt = signal_t(0);
  signal_t alphaRel = signal_t(0);

  ScSettings<signal_t>& settings;
  Stereo<signal_t> stateFilter = signal_t(0.0);

  PeakSideChainDb(ScSettings<signal_t>& settings) : settings(settings) { }

  virtual Stereo<signal_t> process(Stereo<signal_t> x) noexcept override {
    auto ySens = this->peakSensor.process(x);
    ensureFinite(this->stateFilter);
    return {
      this->_gainComputer_db(ySens.l, this->stateFilter.l),
      this->_gainComputer_db(ySens.r, this->stateFilter.r),
    };
  }

  virtual void update() noexcept override {
    this->alphaAtt = gcem::exp(-2200.0 / (this->settings.tAtt_ms * this->fs));
    this->alphaRel = gcem::exp(-2200.0 / (this->settings.tRel_ms * this->fs));
    if (this->alphaRel < this->alphaAtt) { this->alphaRel = this->alphaAtt; }
    this->peakSensor.setT_ms(this->settings.tPeak_ms);
    this->peakSensor.update();
  }

  virtual void reset(float fs) noexcept override {
    this->peakSensor.reset(fs);
    this->fs = fs;
    this->update();
  }

  inline signal_t _gainComputer_db(signal_t x, signal_t& state) noexcept {
    signal_t x_db = db(x);
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
    signal_t alpha  = this->alphaRel;
    if (target > state) { alpha = this->alphaAtt; }
    signal_t yFilter = state * alpha + target * (1 - alpha);
    if (yFilter != yFilter) { yFilter = signal_t(0); }
    state = yFilter;
    return invDb(-yFilter);
  }
};

template <typename signal_t>
struct PeakSideChainLinear : public PeakSideChainDb<signal_t> {
  signal_t thresh_lin = signal_t(1);
  signal_t ratio_lin  = signal_t(1);
  signal_t knee_lin   = signal_t(1);

  PeakSideChainLinear(ScSettings<signal_t>& settings)
      : PeakSideChainDb<signal_t>(settings) { }

  virtual Stereo<signal_t> process(Stereo<signal_t> x) noexcept override {
    auto ySens = this->peakSensor.process(x);
    ensureFinite(this->stateFilter);
    return {
      this->_gainComputer_lin(ySens.l, this->stateFilter.l),
      this->_gainComputer_lin(ySens.r, this->stateFilter.r),
    };
  }

  virtual void update() noexcept override {
    this->PeakSideChainDb<signal_t>::update();
    this->thresh_lin            = invDb(this->settings.thresh_db);
    this->knee_lin              = invDb(this->settings.knee_db);
    const signal_t oneOverSqrt2 = 1.0 / gcem::sqrt(2.0);
    const signal_t tmp          = oneOverSqrt2
        - (this->settings.ratio_db - signal_t(3.0)) / signal_t(18.0);
    this->ratio_lin = (signal_t(1.0) - signal_t(1.0) / this->settings.ratio_db)
        * (oneOverSqrt2 - tmp * tmp * tmp * tmp * tmp);
  }

  inline signal_t _gainComputer_lin(signal_t x, signal_t& state) noexcept {
    signal_t target;
    if (x < this->thresh_lin / this->knee_lin) {
      target = signal_t(0);
    } else if (x < this->thresh_lin) {
      target = (x / this->thresh_lin) * this->ratio_lin * this->thresh_lin
          / (this->knee_lin * x);
    } else {
      target = (x / this->thresh_lin) * this->ratio_lin;
    }
    signal_t alpha = this->alphaRel;
    if (target > state) { alpha = this->alphaAtt; }
    signal_t yFilter = state * alpha + target * (1 - alpha);
    state            = yFilter;
    return signal_t(1.0) / (yFilter + 1);
  }
};

template <typename signal_t>
struct RmsSideChainDb : public PeakSideChainDb<signal_t> {
  RmsSensorStereo<signal_t> rmsSensor;

  RmsSideChainDb(ScSettings<signal_t>& settings)
      : PeakSideChainDb<signal_t>(settings) { }

  virtual Stereo<signal_t> process(Stereo<signal_t> x) noexcept override {
    auto ySens = rmsSensor.process(x);
    ensureFinite(this->stateFilter.l);
    ensureFinite(this->stateFilter.r);
    return {
      this->_gainComputer_db(ySens.l, this->stateFilter.l),
      this->_gainComputer_db(ySens.r, this->stateFilter.r),
    };
  }

  virtual void update() noexcept override {
    this->rmsSensor.setT_ms(this->settings.tRms_ms);
    this->rmsSensor.update();
    this->PeakSideChainDb<signal_t>::update();
  }

  virtual void reset(float fs) noexcept override {
    this->rmsSensor.reset(fs);
    this->PeakSideChainDb<signal_t>::reset(fs);
  }
};

template <typename signal_t>
struct RmsSideChainLinear : public PeakSideChainLinear<signal_t> {
  RmsSensorStereo<signal_t> rmsSensor;

  RmsSideChainLinear(ScSettings<signal_t>& settings)
      : PeakSideChainLinear<signal_t>(settings) { }

  virtual Stereo<signal_t> process(Stereo<signal_t> x) noexcept override {
    auto ySens = rmsSensor.process(x);
    ensureFinite(this->stateFilter);
    return {
      this->_gainComputer_lin(ySens.l, this->stateFilter.l),
      this->_gainComputer_lin(ySens.r, this->stateFilter.r),
    };
  }

  virtual void update() noexcept override {
    this->rmsSensor.setT_ms(this->settings.tRms_ms);
    this->rmsSensor.update();
    this->PeakSideChainLinear<signal_t>::update();
  }

  virtual void reset(float fs) noexcept override {
    this->rmsSensor.reset(fs);
    this->PeakSideChainLinear<signal_t>::reset(fs);
  }
};
} // namespace NtFx
