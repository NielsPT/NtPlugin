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

#include "lib/MsRmsSensor.h"
#include "lib/MultitapRmsSensor.h"
#include "lib/Plugin.h"
#include "lib/Stereo.h"

template <typename signal_t>
struct ntRmsMeter : NtFx::NtPlugin<signal_t> {
  enum Meter { Peak, RMS, RMS_10_ms, RMS_100_ms, RMS_1_s };

  NtFx::MultitapRmsSensor<signal_t> multitap;
  NtFx::MsRmsSensor<signal_t> msSensor;
  signal_t decay_s = 0.1;
  signal_t hold_s  = 2;
  signal_t tRms_ms = 10;

  ntRmsMeter() {
    this->uiSpec.meters = {
      { "Peak", .minVal_db = -50 },
      { "RMS", .hasScale = true, .minVal_db = -50 },
      { "RMS_10_ms", .minVal_db = -50 },
      { "RMS_100_ms", .minVal_db = -50 },
      { "RMS_1_s", .hasScale = true, .minVal_db = -50 },
    };
    this->secondaryKnobs = {
      { &this->decay_s, "Decay", " s", 0, 1 },
      { &this->hold_s, "Hold", " s", 0, 10 },
      { &this->tRms_ms, "RMS_Time", " ms", 1, 1000 },
    };
    this->uiSpec.meterHeight_dots = 25;
    this->updateDefaults();
  }

  virtual NtFx::Stereo<signal_t> process(
      NtFx::Stereo<signal_t> x) noexcept override {
    this->template updatePeakLevel<Peak>(x);
    msSensor.process(x);
    this->template updatePeakLevel<RMS>(msSensor.getRms());
    multitap.process(x);
    this->template updatePeakLevel<RMS_10_ms>(multitap.getRms(1));
    this->template updatePeakLevel<RMS_100_ms>(multitap.getRms(2));
    this->template updatePeakLevel<RMS_1_s>(multitap.getRms(3));
    return x;
  }

  virtual void update() noexcept override {
    for (auto& m : this->uiSpec.meters) { m.decay_s = this->decay_s; }
    for (auto& m : this->uiSpec.meters) { m.hold_s = this->hold_s; }
    this->msSensor.setRmsAvgTime(this->tRms_ms);
    this->uiNeedsUpdate = true;
  }

  virtual void reset(float fs) noexcept override {
    this->fs = fs;
    this->multitap.reset(fs);
    this->msSensor.reset(fs);
    this->update();
  }
};