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
#include "lib/DelayLine.h"
#include "lib/FirstOrder.h"
#include "lib/Glider.h"
#include "lib/Plugin.h"
#include "lib/utils.h"

constexpr signal_t minRate_hz = 0.1;
constexpr int tModDlMax_ms    = 10 * int(1.0 / minRate_hz);
constexpr int tWetDlMax_ms    = 100;

struct ntChorus : public NtFx::NtPlugin {
  NtFx::Delay::FractDelayLine<192 * 8 * tModDlMax_ms, signal_t, 4> dlL;
  NtFx::Delay::FractDelayLine<192 * 8 * tModDlMax_ms, signal_t, 4> dlR;
  NtFx::Delay::ShortGlideDelayLine<192 * 8 * tWetDlMax_ms, Audio> dlWet;
  NtFx::FirstOrder::StereoFilter<NtFx::FirstOrder::Shape::hpf> hpf;
  NtFx::FirstOrder::StereoFilter<NtFx::FirstOrder::Shape::lpf> lpf;
  NtFx::ExpGlider tDelayMod_s;
  NtFx::ExpGlider phaseMod_rad;
  NtFx::ExpGlider fMod_hz { 0.25 };
  signal_t tDelayMod_ms { 10 };
  signal_t phaseMod_deg { 90 };
  signal_t gWet_db { -3 };
  signal_t gDry_db { -3 };
  signal_t gWet_lin { 1 };
  signal_t gDry_lin { 1 };
  signal_t tSample { 1 / 48e3 };
  signal_t _t { 0 };
  signal_t fcHpf_hz { 20 };
  signal_t fcLpf_hz { 20e3 };
  bool bypassEnable { false };

  ntChorus() {
    this->primaryKnobs = {
      { &this->fMod_hz.ui, "Rate", " Hz", minRate_hz, 10, 1 },
      { &this->dlWet.t_ms.ui, "Delay", " ms", 0, tWetDlMax_ms },
      { &this->tDelayMod_ms, "Depth", " ms", 0, tModDlMax_ms * minRate_hz },
    };
    this->secondaryKnobs = {
      { &this->phaseMod_deg, "Mod phase", " deg", 0, 180 },
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
    this->tDelayMod_s.process();
    this->phaseMod_rad.process();
    this->fMod_hz.process();
    this->template updatePeakLevel<0>(x);
    if (this->bypassEnable) {
      this->template updatePeakLevel<1>(x);
      return x;
    }
    signal_t omegaT_rad = 2 * GCEM_PI * this->fMod_hz.pr * this->_t;
    this->_t += this->tSample;
    if (this->_t >= 1 / this->fMod_hz.pr) { this->_t = 0; }
    this->dlL.t_ms =
        (gcem::sin(omegaT_rad) * this->tDelayMod_s.pr + this->tDelayMod_s.pr)
        * 1000;
    this->dlR.t_ms =
        (gcem::sin(omegaT_rad + this->phaseMod_rad.pr) * this->tDelayMod_s.pr
            + this->tDelayMod_s.pr)
        * 1000;
    this->dlL.update();
    this->dlR.update();
    // TODO: Proper mix w/ -3dB midpoint.
    Audio xDlWet = { this->dlL.process(x.l), this->dlR.process(x.r) };
    Audio xFlt   = this->dlWet.process(xDlWet);
    Audio yHpf   = this->hpf.process(xFlt);
    Audio xMix   = this->lpf.process(yHpf);
    Audio y      = xMix * gWet_lin + x * gDry_lin;
    this->template updatePeakLevel<1>(y);
    return y;
  }

  void update() noexcept override {
    this->gWet_lin       = NtFx::invDb(this->gWet_db);
    this->gDry_lin       = NtFx::invDb(this->gDry_db);
    this->tDelayMod_s.ui = this->tDelayMod_ms / 2000;
    this->tDelayMod_s.ui /= this->fMod_hz.ui;
    this->phaseMod_rad.ui = this->phaseMod_deg * GCEM_PI / 180;
    this->tSample         = 1 / this->fs;
    this->fMod_hz.update(this->fs);
    this->phaseMod_rad.update(this->fs);
    this->tDelayMod_s.update(this->fs);
    this->dlWet.update();
    this->hpf.fc_hz = this->fcHpf_hz;
    this->lpf.fc_hz = this->fcLpf_hz;
    this->hpf.update();
    this->lpf.update();
  }

  void reset(float fs) noexcept override {
    this->fs = fs;
    this->dlL.reset(fs);
    this->dlR.reset(fs);
    this->dlWet.reset(fs);
    this->hpf.reset(fs);
    this->lpf.reset(fs);
    this->update();
  }
};
