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
  signal_t slopeRel { 0 };
  signal_t alphaAtt { 0 };
  signal_t stateAtt { 0 };
  signal_t stateRel { 0 };
  int nHold { 0 };
  int holdCount { 0 };

  GateSc(GateScSettings<signal_t>& settings) : settings(settings) { }

  virtual Stereo<signal_t> process(Stereo<signal_t> x) noexcept override {
    auto xSc = x.absMax();
    auto y   = this->gateSc_db(xSc);
    return { y, y };
  }

  virtual void update() noexcept override {
    this->alphaAtt = gcem::exp(-2200 / (this->settings.tAtt_ms * this->fs));
    this->slopeRel = this->settings.range_db * signal_t(20)
        / (this->settings.range_db * this->settings.tRel_ms * signal_t(0.001)
            * this->fs);
    this->nHold = gcem::round(this->settings.tHold_ms * 0.001 * this->fs);
    this->sensor.update();
  }

  virtual void reset(float fs) noexcept override {
    this->fs              = fs;
    stateAtt              = -100;
    stateRel              = -100;
    this->sensor.tPeak_ms = 1;
    this->sensor.reset(fs);
    this->update();
  }

  signal_t gateSc_db(signal_t x) {
    auto ySens { this->sensor.process(x) };
    auto x_db = NtFx::db(ySens + 1e-20);
    signal_t target_db { -1e-20 };
    if (x_db > this->settings.thresh_db) {
      this->holdCount = this->nHold;
    } else {
      target_db = this->settings.range_db;
    }
    if (this->stateRel >= target_db) {
      if (this->holdCount > 0) {
        this->holdCount--;
      } else {
        this->stateRel -= this->slopeRel;
      }
    } else {
      this->stateRel = target_db;
    }
    auto y_db =
        this->alphaAtt * this->stateAtt + (1 - this->alphaAtt) * this->stateRel;
    this->stateAtt = y_db;
    return NtFx::invDb(y_db);
  }
};
}