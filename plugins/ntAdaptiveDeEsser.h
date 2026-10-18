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
#include "lib/FirstOrder.h"
#include "lib/Plugin.h"
#include "lib/gcem.h"
#include <cstddef>

struct ntAdaptiveDeEsser final : public NtFx::Plugin {
  enum Order { e_first, e_second };
  NtFx::Delay::Short<10.0> dl;
  NtFx::AdaptiveDeEssSc sc;
  NtFx::DynamicFilter::ShelfFixedPoles shelf2nd;
  NtFx::FirstOrder::StereoFilter<NtFx::FirstOrder::Shape::hpf> hpf1st;
  signal_t red_p { 100 };
  signal_t red_lin { 1 };
  Order order { e_second };
  bool extScEnable { false };
  bool linkParams { false };
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
          .name     = "Peak Hold",
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
      {
          .p_val    = &this->sc.peakLo.tHold_ms,
          .name     = "LF peak hold",
          .suffix   = " ms",
          .minVal   = 0,
          .maxVal   = 10,
          .midPoint = 1,
      },
      // {
      //     .p_val    = &this->sc.peakLo.tRel_ms,
      //     .name     = "LF release",
      //     .suffix   = " ms",
      //     .minVal   = 0,
      //     .maxVal   = 1000,
      //     .midPoint = 10,
      // },
      {
          .p_val  = &this->sc.rmsLo.t_ms,
          .name   = "LF RMS",
          .suffix = " ms",
          .minVal = 1.0,
          .maxVal = 40.0,
      },
      {
          .p_val  = &this->sc.rmsHi.t_ms,
          .name   = "HF RMS",
          .suffix = " ms",
          .minVal = 1.0,
          .maxVal = 40.0,
      },
    };
    this->toggles = {
      { .p_val = &this->sc.rmsEnable, .name = "RMS mode" },
      { .p_val = &this->extScEnable, .name = "Ext SC" },
      { .p_val = &this->sc.scListen, .name = "SC Listen" },
      { .p_val = &this->linkParams, .name = "Auto" },
      { .p_val = &this->bypassEnable, .name = "Bypass" },
    };
    this->meters.push_back({
        .name     = "GR",
        .dbPrDot  = 2,
        .invert   = true,
        .hasScale = true,
    });
    this->radioButtons = {
      {
          .p_val   = (int*)&this->order,
          .name    = "Filter type",
          .options = { "1st Order", "2nd Order" },
      },
    };
    this->dl.t_ms     = 5;
    this->shelf2nd.q1 = 0.508;
    this->shelf2nd.q2 = 0.508;
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
    auto ySc = this->sc.process(xSc);
    if (this->sc.scListen) {
      this->updatePeakLevel(1, ySc);
      return ySc;
    }
    Audio y;
    auto gain = (ySc * this->red_lin - this->red_lin + 1).absMin();
    if (this->order == Order::e_second) {
      this->shelf2nd.gain_lin = gain;
      y                       = this->shelf2nd.process(yDl);
    } else {
      y = yDl + this->hpf1st.process(yDl) * (gain - 1);
    }
    this->updatePeakLevel(1, y);
    this->updatePeakLevel(2, gain);
    return y;
  }

  void update() noexcept override {

    if (this->linkParams) {
      this->deactivateParameter("Attack");
      this->deactivateParameter("LF peak hold");
      // this->deactivateParameter("LF release");
      this->deactivateParameter("Peak Hold");
      this->deactivateParameter("LF RMS");
      this->deactivateParameter("HF RMS");
      this->sc.rmsHi.t_ms               = this->dl.t_ms * 2;
      this->sc.rmsLo.t_ms               = this->dl.t_ms * 2;
      this->sc.sc.settings.tPeakHold_ms = this->dl.t_ms;
      this->sc.sc.settings.tAtt_ms      = this->dl.t_ms;
      this->sc.peakLo.tHold_ms          = this->dl.t_ms;
      if (this->sc.rmsEnable) { this->sc.sc.settings.tAtt_ms = 0.1; }
      if (this->sc.sc.settings.tPeakHold_ms < 0.25) {
        this->sc.sc.settings.tPeakHold_ms = 0.25;
      }
      if (this->sc.peakLo.tHold_ms < 0.25) { this->sc.peakLo.tHold_ms = 0.25; }
    } else {
      this->activateParameter("Attack");
      this->activateParameter("Peak Hold");
      if (this->sc.rmsEnable) {
        this->deactivateParameter("LF peak hold");
        // this->deactivateParameter("LF release");
        this->deactivateParameter("Peak Hold");
        this->activateParameter("LF RMS");
        this->activateParameter("HF RMS");
      } else {
        this->activateParameter("LF peak hold");
        // this->activateParameter("LF release");
        this->activateParameter("Peak Hold");
        this->deactivateParameter("LF RMS");
        this->deactivateParameter("HF RMS");
      }
    }
    this->red_lin        = gcem::sqrt(this->red_p / signal_t(100.0));
    this->shelf2nd.fc_hz = this->sc.fc_hz;
    this->latency        = size_t(this->dl.t_ms / 1000 * this->_fs);
    this->hpf1st.fc_hz   = this->sc.fc_hz;
    this->hpf1st.update();
    this->sc.update();
    this->shelf2nd.update();
    this->dl.update();
  }

  void reset(signal_t fs) noexcept override {
    this->_fs = fs;
    this->dl.reset(fs);
    this->sc.reset(fs);
    this->hpf1st.reset(fs);
    this->shelf2nd.reset(fs);
    this->update();
  }
};
