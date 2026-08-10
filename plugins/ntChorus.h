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

#pragma once

#include "lib/Audio.h"
#include "lib/Delay.h"
#include "lib/FirstOrder.h"
#include "lib/Glider.h"
#include "lib/Plugin.h"
#include "lib/utils.h"

constexpr double tWetDlMax_ms = 100;

struct ntChorus : public NtFx::NtPlugin {
  NtFx::Delay::Mod dlMod;
  NtFx::Delay::ShortGlided<tWetDlMax_ms, Audio> dlWet;
  NtFx::FirstOrder::StereoFilter<NtFx::FirstOrder::Shape::hpf> hpf;
  NtFx::FirstOrder::StereoFilter<NtFx::FirstOrder::Shape::lpf> lpf;

  signal_t gWet_db { -3 };
  signal_t gDry_db { -3 };
  signal_t gWet_lin { 1 };
  signal_t gDry_lin { 1 };
  signal_t fcHpf_hz { 20 };
  signal_t fcLpf_hz { 20e3 };
  bool bypassEnable { false };

  ntChorus() {
    this->primaryKnobs = {
      { &this->dlMod.fMod_hz.ui, "Rate", " Hz", this->dlMod.minRate_hz, 10, 1 },
      { &this->dlWet.t_ms.ui, "Delay", " ms", 0, tWetDlMax_ms },
      { &this->dlMod.depth_p, "Depth", " %", 0, 100 },
    };
    this->secondaryKnobs = {
      { &this->dlMod.phaseMod_deg, "Mod phase", " deg", 0, 180 },
      { &this->fcHpf_hz, "HPF", " Hz", 20, 20e3, 2e3 },
      { &this->fcLpf_hz, "LPF", " Hz", 20, 20e3, 2e3 },
      { &this->gDry_db, "Dry", " dB", -100, 0 },
      { &this->gWet_db, "Wet", " dB", -100, 0 },
    };
    this->toggles       = { { &this->bypassEnable, "Bypass" } };
    this->dlWet.t_ms.ui = 10;
    this->dlWet.t_ms.pr = 10;
    this->updateDefaults();
  }

  Audio process(Audio x) noexcept override {

    this->template updatePeakLevel<0>(x);
    if (this->bypassEnable) {
      this->template updatePeakLevel<1>(x);
      return x;
    }

    Audio xDlWet = this->dlMod.process(x);
    Audio xFlt   = this->dlWet.process(xDlWet);
    Audio yHpf   = this->hpf.process(xFlt);
    Audio xMix   = this->lpf.process(yHpf);
    Audio y      = xMix * gWet_lin + x * gDry_lin;
    this->template updatePeakLevel<1>(y);
    return y;
  }

  void update() noexcept override {
    this->dlMod.update();
    this->gWet_lin = NtFx::invDb(this->gWet_db);
    this->gDry_lin = NtFx::invDb(this->gDry_db);
    this->dlWet.update();
    this->hpf.fc_hz = this->fcHpf_hz;
    this->lpf.fc_hz = this->fcLpf_hz;
    this->hpf.update();
    this->lpf.update();
  }

  void reset(float fs) noexcept override {
    this->fs = fs;
    this->dlMod.reset(fs);
    this->dlWet.reset(fs);
    this->hpf.reset(fs);
    this->lpf.reset(fs);
    this->update();
  }
};
