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

#include "lib/Plugin.h"
#include "lib/RmsSensor.h"
#include "lib/Stereo.h"

template <typename signal_t>
struct ntRmsMeter : NtFx::NtPlugin<signal_t> {
  enum Meter {
    Peak,
    RMS,
  };

  NtFx::RmsSensorStereo<signal_t> msSensor;
  signal_t decay_s = 0.1;
  signal_t hold_s  = 2;
  signal_t tRms_ms = 10;

  ntRmsMeter() {
    this->meters = {
      { .name = "Peak", .minVal_db = -50, .addRms = true },
      { .name = "RMS", .minVal_db = -50, .hasScale = true },
    };
    this->primaryKnobs = {
      { &this->decay_s, "Decay", " s", 0, 1 },
      { &this->hold_s, "Hold", " s", 0, 10 },
      { &this->tRms_ms, "RMS_Time", " ms", 1, 1000 },
    };
    this->uiSpec.meterHeight_dots   = 25;
    this->uiSpec.defaultWindowWidth = 350;
    this->uiSpec.maxColumns         = 1;
    this->updateDefaults();
  }

  virtual NtFx::Stereo<signal_t> process(
      NtFx::Stereo<signal_t> x) noexcept override {
    this->template updatePeakLevel<Peak>(x);
    this->template updatePeakLevel<RMS>(msSensor.process(x));
    return x;
  }

  virtual void update() noexcept override {
    for (auto& m : this->meters) { m.decay_s = this->decay_s; }
    for (auto& m : this->meters) { m.hold_s = this->hold_s; }
    this->msSensor.setT_ms(this->tRms_ms);
    this->uiNeedsUpdate = true;
  }

  virtual void reset(float fs) noexcept override {
    this->fs = fs;
    this->msSensor.reset(fs);
    this->update();
  }
};