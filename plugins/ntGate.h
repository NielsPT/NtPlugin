#pragma once
/**
 * @file ntGate.h
 * @brief Niose gate.
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

#include "gcem.hpp"
#include "lib/Audio.h"
#include "lib/Biquad.h"
#include "lib/Comp.h"
#include "lib/DynamicFilter.h"
#include "lib/GateSc.h"
#include "lib/Plugin.h"
#include <array>

enum ScMode { internal, external, ignore };

constexpr int dlLookaheadLen = 192 * 8 * 10;

struct ntGate : public NtFx::NtPlugin {
  NtFx::Gate::Sc sc;
  NtFx::Gate::Sc scHf;
  NtFx::Comp::ScSettings ignoreScSettings;
  NtFx::Comp::PeakSideChainLinear ignoreSc;
  NtFx::DynamicFilter::Shelf flt;
  NtFx::Biquad::EqBand hpf;
  NtFx::Biquad::EqBand lpf;
  std::array<Audio, dlLookaheadLen> dlLookahead;
  signal_t tLookahead_ms { 0.2 };
  int scMode { 0 };
  int nLookahead { 4 };
  int iLookahead { 0 };
  bool bypassEnable { false };
  bool scListenEnable { false };
  bool hfAccelEnable { false };
  bool lookaheadEnable { true };
  bool latencyCompEnable { true };

  ntGate() : ignoreSc(ignoreScSettings) {
    this->primaryKnobs = {
      {
          .p_val  = &this->sc.settings.thresh_db,
          .name   = "Threshold",
          .suffix = " dB",
          .minVal = -60,
          .maxVal = 0,
      },
      {
          .p_val  = &this->sc.settings.range_db,
          .name   = "Range",
          .suffix = " dB",
          .minVal = -60,
          .maxVal = -0.0,
      },
      {
          .p_val    = &this->sc.settings.tAtt_ms,
          .name     = "Attack",
          .suffix   = " ms",
          .minVal   = 0.01,
          .maxVal   = 100,
          .midPoint = 10,
      },
      {
          .p_val    = &this->sc.settings.tHold_ms,
          .name     = "Hold",
          .suffix   = " ms",
          .minVal   = 0.01,
          .maxVal   = 1000.0,
          .midPoint = 100,
      },
      {
          .p_val    = &this->sc.settings.tRel_ms,
          .name     = "Release",
          .suffix   = " ms",
          .minVal   = 10.0,
          .maxVal   = 1000.0,
          .midPoint = 100,
      },
    };
    this->hpf.settings.fc_hz = 20;
    this->lpf.settings.fc_hz = 20e3;
    this->secondaryKnobs     = {
      {
          .p_val    = &this->hpf.settings.fc_hz,
          .name     = "SC_HPF",
          .suffix   = " Hz",
          .minVal   = 20,
          .maxVal   = 20e3,
          .logScale = true,
      },
      {
          .p_val    = &this->lpf.settings.fc_hz,
          .name     = "SC_LPF",
          .suffix   = " Hz",
          .minVal   = 20,
          .maxVal   = 20e3,
          .logScale = true,
      },
      {
          .p_val    = &this->flt.fc_hz,
          .name     = "Xover",
          .suffix   = " Hz",
          .minVal   = 200,
          .maxVal   = 20e3,
          .logScale = true,
          .isActive = false,
      },
      {
          .p_val    = &this->scHf.settings.tHold_ms,
          .name     = "HF_Hold",
          .suffix   = " ms",
          .minVal   = 0.1,
          .maxVal   = 1000,
          .isActive = false,
          .midPoint = 100,
      },
      {
          .p_val    = &this->scHf.settings.tRel_ms,
          .name     = "HF_Release",
          .suffix   = " ms",
          .minVal   = 0.1,
          .maxVal   = 1000,
          .isActive = false,
          .midPoint = 100,
      },
      {
          .p_val    = &this->ignoreScSettings.thresh_db,
          .name     = "Ignore_Sens",
          .suffix   = " dB",
          .minVal   = -80,
          .maxVal   = 0,
          .isActive = false,
      },
      {
          .p_val  = &this->tLookahead_ms,
          .name   = "Lookahead",
          .suffix = " ms",
          .minVal = 0,
          .maxVal = 10,
      },
    };
    this->toggles = {
      { .p_val = &this->scListenEnable, .name = "SC_Listen" },
      { .p_val = &this->hfAccelEnable, .name = "Dual_Band" },
      { .p_val = &this->lookaheadEnable, .name = "Lookahead_on" },
      { .p_val = &this->latencyCompEnable, .name = "Latency_comp" },
      { .p_val = &this->bypassEnable, .name = "Bypass" },
    };
    this->radioButtons = {
      {
          .p_val   = (int*)&this->scMode,
          .name    = "Side_Chain",
          .options = { "Internal", "External", "Ignore" },
      },
    };
    this->meters = {
      { .name = "IN", .addRms = true },
      { .name = "OUT", .hasScale = true, .addRms = true },
      { .name = "GR", .invert = true },
      { .name = "HF GR", .invert = true, .hasScale = true },
    };
    this->sc.settings.tAtt_ms = 0.1;
    this->updateDefaults();
  }

  Audio process(Audio x) noexcept override {
    this->dlLookahead[this->iLookahead++] = x;
    if (this->iLookahead >= dlLookaheadLen) { this->iLookahead = 0; }
    this->template updatePeakLevel<0>(x);
    if (this->bypassEnable) {
      this->template updatePeakLevel<1>(x);
      this->template updatePeakLevel<2>(1);
      return x;
    }
    auto xSc = x;
    if (this->scMode == ScMode::external) {
      xSc = this->xSc;
    } else if (this->scMode == ScMode::ignore) {
      xSc = this->ignoreSc.process(this->xSc) * x;
    }
    NtFx::ensureFinite(xSc);
    auto yHpf = this->hpf.process(xSc);
    auto yLpf = this->lpf.process(yHpf);
    if (this->scListenEnable) {
      this->template updatePeakLevel<1>(yLpf);
      return yLpf;
    }
    auto gr   = this->sc.process(yLpf);
    auto grHf = gr;
    auto xGr  = x;
    if (this->lookaheadEnable && this->nLookahead) {
      // TODO: delayline class.
      auto i = this->iLookahead - this->nLookahead;
      if (i < 0) { i += dlLookaheadLen; }
      xGr = this->dlLookahead[i];
    }
    auto y = xGr * gr;
    if (this->hfAccelEnable) {
      grHf         = this->scHf.process(yLpf);
      auto A       = grHf / gr;
      flt.gain_lin = A.absMax();
      y            = this->flt.process(xGr) * gr;
    }
    this->template updatePeakLevel<1>(y);
    this->template updatePeakLevel<2, true>(gr);
    this->template updatePeakLevel<3, true>(grHf);
    return y;
  }

  void update() noexcept override {
    if (this->hfAccelEnable) {
      this->activateParameter("Xover");
      this->activateParameter("HF_Hold");
      this->activateParameter("HF_Release");
    } else {
      this->deactivateParameter("Xover");
      this->deactivateParameter("HF_Hold");
      this->deactivateParameter("HF_Release");
    }
    if (this->scMode == ScMode::ignore) {
      this->activateParameter("Ignore_Sens");
    } else {
      this->deactivateParameter("Ignore_Sens");
    }
    if (this->lookaheadEnable) {
      this->activateParameter("Lookahead");
    } else {
      this->deactivateParameter("Lookahead");
    }
    // this->lookaheadEnable = (this->tLookahead_ms > 0);
    this->nLookahead = int(this->tLookahead_ms * 0.001 * this->fs);
    if (this->latencyCompEnable && this->nLookahead) {
      this->latency = this->nLookahead;
    } else {
      this->latency = 0;
    }
    this->flt.q1 = 0.6;
    this->flt.q2 = 0.6;
    this->flt.update();
    this->lpf.update();
    this->hpf.update();
    this->sc.update();
    this->ignoreSc.update();
    this->scHf.settings.thresh_db = this->sc.settings.thresh_db;
    this->scHf.settings.range_db  = this->sc.settings.range_db;
    this->scHf.settings.tAtt_ms   = this->sc.settings.tAtt_ms;
    if (this->scHf.settings.tHold_ms > this->sc.settings.tHold_ms) {
      this->scHf.settings.tHold_ms = this->sc.settings.tHold_ms;
    }
    if (this->scHf.settings.tRel_ms > this->sc.settings.tRel_ms) {
      this->scHf.settings.tRel_ms = this->sc.settings.tRel_ms;
    }
    this->scHf.update();
  }

  void reset(float fs) noexcept override {
    this->fs = fs;
    this->sc.reset(fs);
    this->scHf.reset(fs);
    this->hpf.settings.shape = NtFx::Biquad::Shape::hpf;
    this->hpf.reset(fs);
    this->lpf.settings.shape = NtFx::Biquad::Shape::lpf;
    this->lpf.reset(fs);
    this->flt.reset(fs);
    this->ignoreScSettings.knee_db      = 3;
    this->ignoreScSettings.ratio        = 20;
    this->ignoreScSettings.tAtt_ms      = 0.1;
    this->ignoreScSettings.tRel_ms      = 20;
    this->ignoreScSettings.tPeakHold_ms = 10;
    this->ignoreSc.reset(fs);
    this->update();
  }
};
