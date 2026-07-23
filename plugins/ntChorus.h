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
#include "lib/Plugin.h"
#include "lib/utils.h"

constexpr int tDlMax = 100;

struct ntChorus : public NtFx::NtPlugin {
  NtFx::Delay::FractDelayLine<192 * 8 * tDlMax, signal_t, 4> dlL;
  NtFx::Delay::FractDelayLine<192 * 8 * tDlMax, signal_t, 4> dlR;
  NtFx::Delay::ShortDelayLine<192 * 8 * tDlMax, Audio> dlWet;
  signal_t tDelayMod_ms { 10 };
  signal_t tDelayMod_s { 0.025 };
  signal_t fMod_hz { 0.25 };
  signal_t phaseMod_deg { 90 };
  signal_t phaseMod_rad { GCEM_PI / 2 };
  signal_t depth_p { 25 };
  signal_t depth_lin { 0.25 };
  // signal_t mix_p { 50 };
  // signal_t mix_lin { 0.5 };
  signal_t gWet_db { -3 };
  signal_t gDry_db { -3 };
  signal_t gWet_lin { 1 };
  signal_t gDry_lin { 1 };
  signal_t tSample { 1 / 48e3 };
  signal_t _t { 0 };

  bool bypassEnable { false };

  ntChorus() {
    this->primaryKnobs = {
      { &this->fMod_hz, "Rate", " Hz", 0.1, 10, 1 },
      { &this->depth_p, "Depth", " %", 0, 100 },
      { &this->phaseMod_deg, "Mod_phase", " deg", 0, 180 },
    };
    this->secondaryKnobs = {
      { &this->tDelayMod_ms, "Mod_Delay", " ms", 0, tDlMax },
      { &this->dlWet.t_ms, "Wet_Delay", " ms", 0, tDlMax },
      { &this->gDry_db, "Dry", " dB", -100, 0 },
      { &this->gWet_db, "Wet", " dB", -100, 0 },
      // { &this->mix_p, "Mix", " %", 0, 100 },
    };
    this->toggles = {
      { .p_val = &this->bypassEnable, .name = "Bypass" },
    };
    this->updateDefaults();
  }

  Audio process(Audio x) noexcept override {
    this->template updatePeakLevel<0>(x);
    if (this->bypassEnable) {
      this->template updatePeakLevel<1>(x);
      return x;
    }
    signal_t omegaT_rad = 2 * GCEM_PI * this->fMod_hz * this->_t;
    this->_t += this->tSample;
    if (this->_t >= 1 / this->fMod_hz) { this->_t = 0; }
    // TODO: modDelayLine. Inherits InterSampleDelayLine.
    this->dlL.t_ms =
        (gcem::sin(omegaT_rad) * this->tDelayMod_s * this->depth_lin
            + this->tDelayMod_s)
        * 1000;
    this->dlR.t_ms = (gcem::sin(omegaT_rad + this->phaseMod_rad)
                             * this->tDelayMod_s * this->depth_lin
                         + this->tDelayMod_s)
        * 1000;
    this->dlL.update();
    this->dlR.update();
    // TODO: Proper mix w/ -3dB midpoint.
    Audio xDlWet = { this->dlL.process(x.l), this->dlR.process(x.r) };
    Audio xMix   = this->dlWet.process(xDlWet);
    Audio y      = xMix * gWet_lin + x * gDry_lin;
    // y          = xMix * this->mix_lin + x * (1 - this->mix_lin);
    // y *= (2 - gcem::abs(this->mix_lin * 2 - 1));
    this->template updatePeakLevel<1>(y);
    return y;
  }

  void update() noexcept override {
    // this->mix_lin      = mix_p / 100;
    this->gWet_lin     = NtFx::invDb(this->gWet_db);
    this->gDry_lin     = NtFx::invDb(this->gDry_db);
    this->depth_lin    = depth_p / 100;
    this->tDelayMod_s  = this->tDelayMod_ms / 1000;
    this->phaseMod_rad = this->phaseMod_deg * GCEM_PI / 180;
    this->tSample      = 1 / this->fs;
    this->dlWet.update();
  }

  void reset(float fs) noexcept override {
    this->fs = fs;
    this->dlL.reset(fs);
    this->dlR.reset(fs);
    this->dlWet.reset(fs);
    this->update();
  }
};
