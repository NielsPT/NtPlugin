#pragma once
/**
 * @file GateSc.h
 * @brief Gate side chain.
 * @author Niels Thøgersen
 * @copyright Copyright (C) 2026 Niels Thøgersen, NTlyd
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

#include "lib/Component.h"
#include "lib/PeakSensor.h"
#include "lib/Stereo.h"
#include "lib/utils.h"

namespace NtFx {

template <typename signal_t>
struct GateScSettings {
  signal_t thresh_db { 0 };
  signal_t range_db { 0 };
  signal_t tAtt_ms { 0.01 };
  signal_t tHold_ms { 10 };
  signal_t tRel_ms { 200 };
};

template <typename signal_t>
struct GateSc : public Component<Stereo<signal_t>> {
  PeakSensor<signal_t> sensor;
  GateScSettings<signal_t>& settings;
  signal_t _slopeRel { 0 };
  signal_t _alphaAtt { 0 };
  signal_t _stateAtt { 0 };
  signal_t _stateRel { 0 };
  int _nHold { 0 };
  int _holdCount { 0 };

  GateSc(GateScSettings<signal_t>& settings) : settings(settings) { }

  virtual Stereo<signal_t> process(Stereo<signal_t> x) noexcept override {
    auto xSc = x.absMax();
    auto y   = this->gateSc_db(xSc);
    return { y, y };
  }

  virtual void update() noexcept override {
    this->_alphaAtt = gcem::exp(-2200 / (this->settings.tAtt_ms * this->fs));
    this->_slopeRel = this->settings.range_db * signal_t(20)
        / (this->settings.range_db * this->settings.tRel_ms * signal_t(0.001)
            * this->fs);
    this->_nHold = gcem::round(this->settings.tHold_ms * 0.001 * this->fs);
    this->sensor.update();
  }

  virtual void reset(float fs) noexcept override {
    this->fs              = fs;
    _stateAtt             = -100;
    _stateRel             = -100;
    this->sensor.tPeak_ms = 1;
    this->sensor.reset(fs);
    this->update();
  }

  signal_t gateSc_db(signal_t x) {
    auto ySens { this->sensor.process(x) };
    auto x_db = NtFx::db(ySens + 1e-20);
    signal_t target_db { -1e-20 };
    if (x_db > this->settings.thresh_db) {
      this->_holdCount = this->_nHold;
    } else {
      target_db = this->settings.range_db;
    }
    if (this->_stateRel >= target_db) {
      if (this->_holdCount > 0) {
        this->_holdCount--;
      } else {
        this->_stateRel -= this->_slopeRel;
      }
    } else {
      this->_stateRel = target_db;
    }
    auto y_db = this->_alphaAtt * this->_stateAtt
        + (1 - this->_alphaAtt) * this->_stateRel;
    this->_stateAtt = y_db;
    return NtFx::invDb(y_db);
  }
};
}