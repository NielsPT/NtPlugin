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
#include "lib/Biquad.h"
#include "lib/Delay.h"
#include "lib/DryMix.h"
#include "lib/Glider.h"
#include "lib/Plugin.h"
#include "lib/SoftClip.h"
#include "lib/gcem.h"
#include "lib/utils.h"

enum SubDev : int {
  half,
  fourth,
  eighth_dot,
  eighth,
  sixteenth_dot,
  sixteenth,
};

const std::vector<std::string> subDevOptions {
  "half",
  "fourth",
  "eighth dot",
  "eighth",
  "sixteenth dot",
  "sixteenth",
};

struct ntTapeEcho final : public NtFx::Plugin {
  NtFx::Delay::LongGlided<2e3, signal_t> dlL;
  NtFx::Delay::LongGlided<2e3, signal_t> dlR;
  NtFx::Delay::Mod mod;
  NtFx::Biquad::EqBand hpf;
  NtFx::Biquad::EqBand lpf;
  NtFx::DryMix dryMix;
  Audio fbState;
  Audio fb_lin { 0.2 };
  signal_t tL_ms { 500 };
  signal_t tR_ms { 500 };
  signal_t fb_percent { 20 };
  signal_t clipG_db { 0.0 };
  signal_t tOffset { 0.0 };
  signal_t noise_db { -100 };
  signal_t tempoScaleL { 1 };
  signal_t tempoScaleR { 1 };
  signal_t noise_lin { 0 };
  size_t iStore { 0 };
  signal_t aClip_lin { 1 };
  SubDev subDevL { SubDev::fourth };
  SubDev subDevR { SubDev::fourth };
  // Mode mode { Mode::indvidual };
  bool linkEnable { true };
  bool syncEnable { false };
  bool modEnable { true };
  bool clipEnable { true };
  bool bypassEnable { false };
  bool swapEnable { false };

  ntTapeEcho() {
    this->primaryKnobs = {
      { &this->tL_ms, "Time Left", " ms", 20, 2e3 },
      { &this->tR_ms, "Time Right", " ms", 20, 2e3 },
      { &this->fb_percent, "Feedback", " %", 0, 200 },
      { &this->clipG_db, "Drive", " dB", -20, 20 },
      { &this->hpf.settings.fc_hz, "HPF", " Hz", 20, 2000, 200 },
      { &this->lpf.settings.fc_hz, "LPF", " Hz", 200, 20000, 2000 },
    };
    this->secondaryKnobs = {
      { &this->hpf.settings.q, "Q HP", "", 0.5, 2, 1 },
      { &this->lpf.settings.q, "Q LP", "", 0.5, 2, 1 },
      { &this->mod.fMod_hz.ui, "Mod Freq", " Hz", 0.1, 10, 1 },
      { &this->mod.depth_p, "Mod Depth", " %", 0, 100 },
      { &this->mod.phaseMod_deg, "Mod Phase", "deg", 0, 180 },
      { &this->tOffset, "Offset", " ms", 0, 50 },
      { &this->noise_db, "Noise", " dB", -100, 0 },
      { &this->dryMix.mix_p, "Mix", " %", 0, 100 },
    };

    this->dropdowns = {
      // {
      //     .p_val    = (int*)&this->mode,
      //     .name     = "Mode",
      //     .options  = { "Dual", "Link",
      //       // "Sync",
      //       // "Ping Pong",
      //     },
      //     .hideName = true,
      // },
      {
          .p_val    = (int*)&this->subDevL,
          .name     = "Subdev L",
          .options  = subDevOptions,
          .hideName = true,
      },
      {
          .p_val    = (int*)&this->subDevR,
          .name     = "Subdev R",
          .options  = subDevOptions,
          .hideName = true,
      },
    };
    this->toggles = {
      { &this->linkEnable, "Link" },
      { &this->syncEnable, "Sync" },
      { &this->modEnable, "Mod" },
      { &this->clipEnable, "Softclip" },
      { &this->swapEnable, "Swap FB" },
      { &this->bypassEnable, "Bypass" },
    };
    this->meters = { { "IN" }, { .name = "OUT", .hasScale = true } };
    this->lpf.settings.shape = NtFx::Biquad::Shape::lpf;
    this->hpf.settings.shape = NtFx::Biquad::Shape::hpf;
    this->lpf.settings.fc_hz = 2e3;
    this->hpf.settings.fc_hz = 300;
    this->lpf.settings.q     = 0.6;
    this->hpf.settings.q     = 0.6;
    this->updateDefaults();
  }

  Audio process(Audio x) noexcept override {
    auto xNoisy = x + NtFx::rand<signal_t>() * this->noise_lin;
    NtFx::ensureFinite(xNoisy);
    NtFx::ensureFinite(this->fbState);
    auto xMod = xNoisy
        + this->fb_lin
            * (this->swapEnable ? this->fbState.swap() : this->fbState);
    auto yMod = xMod;
    if (this->modEnable) { yMod = this->mod.process(xMod); }
    Audio yDelay = { this->dlL.process(yMod.l), this->dlR.process(yMod.r) };
    auto yFbClip =
        NtFx::softClip3rdStereo(yDelay * this->aClip_lin) / aClip_lin;
    NtFx::ensureFinite(yFbClip);
    auto yHp      = hpf.process(yFbClip);
    auto yLp      = lpf.process(yHp);
    this->fbState = yLp;
    auto yOutClip = yLp;
    if (this->clipEnable) { yOutClip = NtFx::softClip5thStereo(yLp); }
    auto y = this->dryMix.process(yOutClip, x);
    this->updatePeakLevel(0, x);
    if (this->bypassEnable) {
      this->updatePeakLevel(1, x);
      return x;
    }
    this->updatePeakLevel(1, y);
    return y;
  }

  void update() noexcept override {
    this->aClip_lin = NtFx::invDb(this->clipG_db);
    this->noise_lin = NtFx::invDb(this->noise_db);
    if (this->modEnable) {
      this->activateParameter("Mod Freq");
      this->activateParameter("Mod Depth");
      this->activateParameter("Mod Phase");
    } else {
      this->deactivateParameter("Mod Freq");
      this->deactivateParameter("Mod Depth");
      this->deactivateParameter("Mod Phase");
    }
    if (this->linkEnable) {
      this->tR_ms    = this->tL_ms + this->tOffset;
      this->fb_lin.r = this->fb_lin.l;
      this->deactivateParameter("Time Right");
      this->activateParameter("Offset");
    } else {
      this->activateParameter("Time Right");
      this->deactivateParameter("Offset");
    }
    if (this->syncEnable) {
      this->tempoScaleL = _tempoScale(this->subDevL);
      this->tL_ms       = 60 / this->tempo * this->tempoScaleL * 1000;
      if (!this->linkEnable) {
        this->tempoScaleR = _tempoScale(this->subDevR);
        this->tR_ms       = 60 / this->tempo * this->tempoScaleR * 1000;
      }
      this->deactivateParameter("Time Left");
      this->deactivateParameter("Time Right");
    } else {
      this->activateParameter("Time Left");
      if (!this->linkEnable) { this->activateParameter("Time Right"); }
    }
    this->fb_lin = this->fb_percent / 100;
    if (this->tL_ms > this->tR_ms) {
      auto t60 = -3 * this->tL_ms / gcem::log10(this->fb_lin.l);
      this->fb_lin.r =
          gcem::pow(signal_t(10), signal_t(-3) * this->tR_ms / t60);
    } else {
      auto t60 = -3 * this->tR_ms / gcem::log10(this->fb_lin.r);
      this->fb_lin.l =
          gcem::pow(signal_t(10), signal_t(-3) * this->tL_ms / t60);
    }
    this->dlL.t_ms.ui = this->tL_ms;
    this->dlR.t_ms.ui = this->tR_ms;
    this->mod.update();
    this->dlL.update();
    this->dlR.update();
    this->hpf.update();
    this->lpf.update();
    this->dryMix.update();
  }

  void reset(signal_t fs) noexcept override {
    this->_fs = fs;
    this->dlL.reset(fs);
    this->dlR.reset(fs);
    this->mod.reset(fs);
    this->hpf.reset(this->_fs);
    this->lpf.reset(this->_fs);
    this->update();
  }

  void onTempoChanged() noexcept override {
    if (this->syncEnable && (this->tempo != 0.0)) { this->update(); }
  }

  signal_t _tempoScale(SubDev subDev) {
    switch (subDev) {
    case SubDev::half:
      return 2;
      break;
    default:
    case SubDev::fourth:
      return 1;
      break;
    case SubDev::eighth_dot:
      return 0.5 * 1.5;
      break;
    case SubDev::eighth:
      return 0.5;
      break;
    case SubDev::sixteenth_dot:
      return 0.25 * 1.5;
      break;
    case SubDev::sixteenth:
      return 0.25;
      break;
    }
  }
};
