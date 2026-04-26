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
#include "lib/GateSc.h"
#include "lib/Plugin.h"
#include "lib/Stereo.h"

template <typename signal_t>
struct ntGate : NtFx::NtPlugin<signal_t> {
  bool bypassEnable { false };
  NtFx::GateScSettings<signal_t> scSettings;
  NtFx::GateSc<signal_t> sc;
  NtFx::Biquad::EqBand<signal_t> hpf;
  NtFx::Biquad::EqBand<signal_t> lpf;

  ntGate() : sc(scSettings) {
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
    this->hpf.settings.fc_hz = 20e3;
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
    };
    this->secondaryKnobs[0].setLogScale();
    this->secondaryKnobs[1].setLogScale();
    this->toggles = {
      { .p_val = &this->bypassEnable, .name = "Bypass" },
    };
    this->meters = {
      { .name = "IN", .addRms = true },
      { .name = "OUT", .hasScale = true, .addRms = true },
      { .name = "GR", .invert = true, .hasScale = true },
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
    auto gr   = this->sc.process(yLpf);
    auto y    = x * gr;
    this->template updatePeakLevel<2, true>(gr);
    this->template updatePeakLevel<1>(y);
    return y;
  }

  void update() noexcept override {
    this->sc.update();
    this->lpf.update();
    this->hpf.update();
  }

  void reset(float fs) noexcept override {
    this->fs = fs;
    this->sc.reset(fs);
    this->hpf.settings.shape = NtFx::Biquad::Shape::hpf;
    this->hpf.reset(fs);
    this->lpf.settings.shape = NtFx::Biquad::Shape::lpf;
    this->lpf.reset(fs);
    this->update();
  }
};
