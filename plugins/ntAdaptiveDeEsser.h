#pragma once

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
 *
 * You are free to download, build and use this code for commercial
 * purposes. Just don't resell it or a build of it, modified or otherwise.
 **/

#include "lib/AdaptiveDeEssSc.h"
#include "lib/Audio.h"
#include "lib/Delay.h"
#include "lib/DynamicFilter.h"
#include "lib/Plugin.h"
#include "lib/Tilt.h"
#include "lib/gcem.h"
#include "lib/utils.h"
#include <cstddef>

struct ntAdaptiveDeEsser final : public NtFx::Plugin {
  NtFx::Delay::Short<10.0> dl;
  NtFx::AdaptiveDeEssSc sc;
  NtFx::DynamicFilter::ShelfFixedPoles shelf;
  NtFx::Tilt<> scTilt;
  signal_t red_p { 100 };
  signal_t red_lin { 100 };
  signal_t range_db { 24 };
  signal_t range_lin { 0.125 };
  bool extScEnable { false };
  bool bypassEnable { false };

  ntAdaptiveDeEsser() {
    this->primaryKnobs = {
      {
          .p_val    = &this->sc.fc_hz,
          .name     = "Frequency",
          .suffix   = " Hz",
          .minVal   = 2e3,
          .maxVal   = 20e3,
          .logScale = true,
      },
      {
          .p_val  = &this->sc.offset_db,
          .name   = "Threshold",
          .suffix = " dB",
          .minVal = -24,
          .maxVal = 24,
      },
      {
          .p_val  = &this->red_p,
          .name   = "Reduction",
          .suffix = " %",
          .minVal = 0,
          .maxVal = 100,
      },
    };
    this->secondaryKnobs = {
      {
          .p_val    = &this->sc.scHpf.settings.fc_hz,
          .name     = "SC HPF",
          .suffix   = " Hz",
          .minVal   = 20,
          .maxVal   = 2000,
          .midPoint = 200,
      },
      {
          .p_val  = &this->scTilt.tilt_db,
          .name   = "SC Tilt",
          .suffix = " dB/decade",
          .minVal = -10,
          .maxVal = 10,
      },
      {
          .p_val    = &this->sc.peakLo.tHold_ms,
          .name     = "LF peak hold",
          .suffix   = " ms",
          .minVal   = 0,
          .maxVal   = 10,
          .midPoint = 1,
      },
      {
          .p_val    = &this->sc.peakLo.tRel_ms,
          .name     = "LF release",
          .suffix   = " ms",
          .minVal   = 0,
          .maxVal   = 1000,
          .midPoint = 10,
      },
      {
          .p_val    = &this->dl.t_ms,
          .name     = "Lookahead",
          .suffix   = " ms",
          .minVal   = 0,
          .maxVal   = 10,
          .midPoint = 1,
      },
      {
          .p_val    = &this->sc.sc.settings.tAtt_ms,
          .name     = "Attack",
          .suffix   = " ms",
          .minVal   = 0,
          .maxVal   = 10,
          .midPoint = 1,
      },
      {
          .p_val    = &this->sc.sc.settings.tPeakHold_ms,
          .name     = "Peak Hold ",
          .suffix   = " ms",
          .minVal   = 0,
          .maxVal   = 10,
          .midPoint = 1,
      },
      {
          .p_val    = &this->sc.sc.settings.tRel_ms,
          .name     = "Release",
          .suffix   = " ms",
          .minVal   = 1.0,
          .maxVal   = 250.0,
          .midPoint = 10.0,
      },
    };
    this->toggles = {
      { .p_val = &this->extScEnable, .name = "Ext SC" },
      { .p_val = &this->sc.scListen, .name = "SC Listen" },
      { .p_val = &this->bypassEnable, .name = "Bypass" },
    };
    this->meters.push_back({ .name = "GR", .invert = true });
    this->dl.t_ms                = 1.25;
    this->sc.sc.settings.tAtt_ms = 1;
    this->shelf.q1               = 0.508;
    this->shelf.q2               = 0.508;
    this->updateDefaults();
  }

  Audio process(Audio x) noexcept override {
    auto yDl = this->dl.process(x);
    this->updatePeakLevel(0, x);
    if (this->bypassEnable) {
      this->updatePeakLevel(1, x);
      return x;
    }
    auto xSc = x;
    if (this->extScEnable) { xSc = this->xSc; }
    auto yTilt = this->scTilt.process(xSc);
    auto ySc   = this->sc.process(yTilt);
    if (this->sc.scListen) {
      this->updatePeakLevel(1, ySc);
      return ySc;
    }
    auto yScReduced = (ySc * this->red_lin - this->red_lin + 1).absMin();
    if (yScReduced < this->range_lin) { yScReduced = this->range_lin; }
    this->shelf.gain_lin = yScReduced;
    auto y               = this->shelf.process(yDl);
    this->updatePeakLevel(1, y);
    this->updatePeakLevel(2, yScReduced);
    return y;
  }

  void update() noexcept override {
    this->range_lin   = NtFx::invDb(-this->range_db);
    this->red_lin     = gcem::sqrt(this->red_p / signal_t(100.0));
    this->shelf.fc_hz = this->sc.fc_hz;
    this->latency     = size_t(this->dl.t_ms / 1000 * this->_fs);
    this->dl.update();
    this->sc.update();
    this->shelf.update();
    this->scTilt.update();
  }

  void reset(signal_t fs) noexcept override {
    this->_fs = fs;
    this->dl.reset(fs);
    this->sc.reset(fs);
    this->shelf.reset(fs);
    this->scTilt.reset(fs);
    this->update();
  }
};
