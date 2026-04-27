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

#define _DO_INVERT true

template <typename signal_t>
struct ntGate : NtFx::NtPlugin<signal_t> {
  NtFx::GateScSettings<signal_t> scSettings;
  NtFx::GateSc<signal_t> sc;
  NtFx::GateScSettings<signal_t> scHfSettings;
  NtFx::GateSc<signal_t> scHf;
  NtFx::DynamidShelf<signal_t> flt;
  NtFx::Biquad::EqBand<signal_t> hpf;
  NtFx::Biquad::EqBand<signal_t> lpf;
  signal_t hfScale { 1 };
  bool bypassEnable { false };
  bool scListenEnable = false;

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
    this->primaryKnobs[6].setLogScale();
    this->hpf.settings.fc_hz = 20;
    this->lpf.settings.fc_hz = 20e3;
    this->secondaryKnobs     = {
      {
          .p_val  = &this->hpf.settings.fc_hz,
          .name   = "SC HPF",
          .suffix = " Hz",
          .minVal = 20,
          .maxVal = 20e3,
      },
      {
          .p_val  = &this->lpf.settings.fc_hz,
          .name   = "SC LPF",
          .suffix = " Hz",
          .minVal = 20,
          .maxVal = 20e3,
      },
      {
          .p_val  = &this->hfScale,
          .name   = "Hf accel",
          .suffix = " x",
          .minVal = 1,
          .maxVal = 10,
      },
      {
          .p_val  = &this->flt.fc_hz,
          .name   = "Hf xover",
          .suffix = " Hz",
          .minVal = 200,
          .maxVal = 20e3,
      },
    };
    this->secondaryKnobs[0].setLogScale();
    this->secondaryKnobs[1].setLogScale();
    this->toggles = {
      { .p_val = &this->scListenEnable, .name = "SC Listen" },
      { .p_val = &this->bypassEnable, .name = "Bypass" },
    };
    this->meters = {
      { .name = "IN", .addRms = true },
      { .name = "OUT", .hasScale = true, .addRms = true },
      { .name = "GR", .invert = _DO_INVERT },
      { .name = "HF GR", .invert = _DO_INVERT, .hasScale = true },
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
    auto yHpf = this->hpf.process(x);
    auto yLpf = this->lpf.process(yHpf);
    if (this->scListenEnable) {
      this->template updatePeakLevel<1>(yLpf);
      this->template updatePeakLevel<2>(1);
      return yLpf;
    }
    auto gr      = this->sc.process(yLpf);
    auto grHf    = this->scHf.process(yLpf);
    auto A       = grHf / gr;
    flt.gain_lin = A.absMax();
    auto yFlt    = this->flt.process(x);

    auto y = yFlt * gr;
    this->template updatePeakLevel<2, _DO_INVERT>(gr);
    this->template updatePeakLevel<3, _DO_INVERT>(grHf);
    this->template updatePeakLevel<1>(y);
    return y;
  }

  void update() noexcept override {
    this->flt.q1 = 0.6;
    this->flt.q2 = 0.6;
    this->flt.update();
    this->lpf.update();
    this->hpf.update();
    this->sc.update();
    this->scHf.settings.thresh_db = this->sc.settings.thresh_db;
    this->scHf.settings.range_db  = this->sc.settings.range_db;
    this->scHf.settings.tAtt_ms   = this->sc.settings.tAtt_ms;
    this->scHf.settings.tRel_ms   = this->sc.settings.tRel_ms / this->hfScale;
    this->scHf.settings.tHold_ms  = this->sc.settings.tHold_ms / this->hfScale;
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
    this->update();
  }
};
#undef _DO_INVERT