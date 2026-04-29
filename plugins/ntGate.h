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
#include "lib/Biquad.h"
#include "lib/DynamicFilter.h"
#include "lib/GateSc.h"
#include "lib/Plugin.h"
#include "lib/Stereo.h"
#include "lib/utils.h"

#define _DO_INVERT true

enum ScMode { internal, external, ignore };

template <typename signal_t>
struct ntGate : NtFx::NtPlugin<signal_t> {
  NtFx::Gate::ScSettings<signal_t> scSettings;
  NtFx::Gate::Sc<signal_t> sc;
  NtFx::Gate::ScSettings<signal_t> scHfSettings;
  NtFx::Gate::Sc<signal_t> scHf;
  NtFx::DynamicFilter::Shelf<signal_t> flt;
  NtFx::Biquad::EqBand<signal_t> hpf;
  NtFx::Biquad::EqBand<signal_t> lpf;
  signal_t ignoreThresh_db { -20 };
  signal_t ignoreThresh_lin { 0.1 };
  signal_t tIgnore_ms { 6 };
  signal_t ignorePeak { 0 };
  int ignoreCount { 0 };
  int nIgnore { 384 };
  int scMode { 0 };
  bool bypassEnable { false };
  bool scListenEnable { false };
  bool hfAccelEnable { false };

  ntGate() : sc(scSettings), scHf(scHfSettings) {
    this->primaryKnobs = {
      {
          .p_val  = &this->scSettings.thresh_db,
          .name   = "Threshold",
          .suffix = " dB",
          .minVal = -60,
          .maxVal = 0,
      },
      {
          .p_val  = &this->scSettings.range_db,
          .name   = "Range",
          .suffix = " dB",
          .minVal = -60,
          .maxVal = 0,
      },
      {
          .p_val  = &this->scSettings.tAtt_ms,
          .name   = "Attack",
          .suffix = " ms",
          .minVal = 0.01,
          .maxVal = 50.0,
      },
      {
          .p_val  = &this->scSettings.tHold_ms,
          .name   = "Hold",
          .suffix = " ms",
          .minVal = 0.01,
          .maxVal = 1000.0,
      },
      {
          .p_val  = &this->scSettings.tRel_ms,
          .name   = "Release",
          .suffix = " ms",
          .minVal = 10.0,
          .maxVal = 1000.0,
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
      },
      {
          .p_val    = &this->scHf.settings.tRel_ms,
          .name     = "HF_Release",
          .suffix   = " ms",
          .minVal   = 0.1,
          .maxVal   = 1000,
          .isActive = false,
      },
      {
          .p_val    = &this->ignoreThresh_db,
          .name     = "Ignore_Sens",
          .suffix   = " dB",
          .minVal   = -80,
          .maxVal   = 0,
          .isActive = false,
      },
    };
    this->toggles = {
      { .p_val = &this->scListenEnable, .name = "SC_Listen" },
      { .p_val = &this->hfAccelEnable, .name = "Dual_Band" },
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
      { .name = "GR", .invert = _DO_INVERT },
      { .name = "HF_GR", .invert = _DO_INVERT, .hasScale = true },
    };
    this->updateDefaults();
  }

  NtFx::Stereo<signal_t> process(NtFx::Stereo<signal_t> x) noexcept override {
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
      auto xAbs = gcem::abs(this->xSc);
      if (xAbs > this->ignorePeak) {
        this->ignorePeak  = xAbs;
        this->ignoreCount = 0;
      }
      if (this->ignoreCount++ > this->nIgnore) { this->ignorePeak = 0; }
      if (this->ignorePeak > this->ignoreThresh_lin
          && this->sc.state != NtFx::Gate::State::open) {
        xSc = x / signal_t(4);
      }
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
    auto y    = x * gr;
    if (this->hfAccelEnable) {
      grHf         = this->scHf.process(yLpf);
      auto A       = grHf / gr;
      flt.gain_lin = A.absMax();
      y            = this->flt.process(x) * gr;
    }
    this->template updatePeakLevel<1>(y);
    this->template updatePeakLevel<2, _DO_INVERT>(gr);
    this->template updatePeakLevel<3, _DO_INVERT>(grHf);
    return y;
  }

  void update() noexcept override {
    if (this->hfAccelEnable) {
      this->secondaryKnobs[2].isActive = true;
      this->secondaryKnobs[3].isActive = true;
      this->secondaryKnobs[4].isActive = true;
    } else {
      this->secondaryKnobs[2].isActive = false;
      this->secondaryKnobs[3].isActive = false;
      this->secondaryKnobs[4].isActive = false;
    }
    if (this->scMode == ScMode::ignore) {
      this->secondaryKnobs[5].isActive = true;
      this->secondaryKnobs[6].isActive = true;
    } else {
      this->secondaryKnobs[5].isActive = false;
      this->secondaryKnobs[6].isActive = false;
    }
    this->nIgnore          = gcem::round(this->tIgnore_ms * 0.001 * this->fs);
    this->ignoreThresh_lin = NtFx::invDb(this->ignoreThresh_db);
    this->flt.q1           = 0.6;
    this->flt.q2           = 0.6;
    this->flt.update();
    this->lpf.update();
    this->hpf.update();
    this->sc.update();
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
    this->uiNeedsUpdate = true;
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
    this->update();
  }
};
#undef _DO_INVERT