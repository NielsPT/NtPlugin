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

#include "gcem.hpp"
#include "lib/Biquad.h"
#include "lib/Glider.h"
#include "lib/Plugin.h"
#include "lib/SoftClip.h"
#include "lib/Stereo.h"
#include "lib/utils.h"
#include <algorithm>
#include <array>
#include <cstddef>

enum SubDev : int {
  half,
  fourth,
  eighth_dot,
  eighth,
  sixteenth_dot,
  sixteenth,
};
// 192 kHz * 2 seconds * 8 x oversampling * 2 stores pr sample * 2 for mod (the
// last doesn't need to be that big).
// TODO: Calculate space for mod.
// TODO: Dynamic alocation of delayline.
constexpr int delayLineLength = 192e3 * 2 * 8 * 2 * 2;

template <typename signal_t>
struct ntTapeEcho : public NtFx::NtPlugin<signal_t> {
  signal_t tGui             = 0.5;
  signal_t fb_percent       = 20;
  signal_t modFreq          = 1.0;
  signal_t modPhase         = 0.0;
  signal_t clipG_db         = 0.0;
  signal_t mix_percent      = 100.0;
  signal_t tOffset          = 0.0;
  signal_t modDepth_percent = 0.1;
  signal_t noise_db         = -100;
  bool sync                 = false;
  bool mod                  = true;
  bool clip                 = true;
  bool doGlide              = true;
  bool bypass               = false;
  SubDev subDev             = SubDev::fourth;
  signal_t tempoScale       = 1;
  NtFx::Biquad::EqBand<signal_t> hpf;
  NtFx::Biquad::EqBand<signal_t> lpf;
  NtFx::Stereo<signal_t> fbState;
  std::array<NtFx::Stereo<signal_t>, delayLineLength> delayLine;
  signal_t fb_lin    = 0.2;
  signal_t noise_lin = 0;
  signal_t tGlide    = 0.36;
  // NtFx::LinGlider<signal_t> nDelay;
  NtFx::ExpGlider<signal_t> nDelay;
  size_t iStore      = 0;
  signal_t aClip_lin = 1;
  size_t timeCounter = 0;
  NtFx::ExpGlider<signal_t> nOffset;
  // TODO: Mix should have -3 dB law.
  signal_t mix_lin = 1;
  NtFx::ExpGlider<signal_t> modDepth;
  // TODO: How in the world do we get mod to glide gracefully?
  NtFx::ExpGlider<signal_t> thetaMod;
  NtFx::ExpGlider<signal_t> thetaModPhase;

  ntTapeEcho() : modDepth(0.1) {
    this->primaryKnobs = {
      { &this->tGui, "Time", " s", 0.02, 2 },
      { &this->fb_percent, "Feedback", " %", 0, 200 },
      { &this->clipG_db, "Drive", " dB", -20, 20 },
      { &this->hpf.settings.fc_hz, "HPF", " Hz", 20, 2000 },
      { &this->lpf.settings.fc_hz, "LPF", " Hz", 200, 20000 },
    };
    this->primaryKnobs[3].setLogScale();
    this->primaryKnobs[4].setLogScale();
    this->secondaryKnobs = {
      { &this->hpf.settings.q, "Q_HP", "", 0.5, 2, 1 },
      { &this->lpf.settings.q, "Q_LP", "", 0.5, 2, 1 },
      { &this->modFreq, "Mod_Freq", " Hz", 0.1, 10 },
      { &this->modDepth_percent,
          "Mod_Depth",
          " %",
          0.01,
          1,
          0.031622776601683794 },
      { &this->modPhase, "Mod_Phase", "deg", 0, 180 },
      { &this->tOffset, "Offset", " ms", 0, 50 },
      // TODO: proper glide parameter.
      // { &this->nGlide, "Glide_Speed", "", 0, 10 },
      { &this->noise_db, "Noise", " dB", -100, 0 },
      { &this->mix_percent, "Mix", " %", 0, 100 },
      { &this->tGlide, "Glide_Time", " s", 0.0, 1 },
    };

    this->secondaryKnobs[0].setLogScale();
    this->secondaryKnobs[1].setLogScale();
    this->secondaryKnobs[2].setLogScale();
    // this->secondaryKnobs[3].setLogScale();
    this->secondaryKnobs[6].setLogScale();
    this->dropdowns = {
      {
          .p_val = (int*)&this->subDev,
          .name = "Subdevision",
          .options = {
              "half",
              "fourth",
              "eighth_dot",
              "eighth",
              "sixteenth_dot",
              "sixteenth",
          }, 
          .defaultIdx = 0,
      },
    };
    this->toggles = {
      { &this->sync, "Sync" },
      { &this->mod, "Mod" },
      { &this->clip, "Softclip" },
      { &this->doGlide, "Glide" },
      { &this->bypass, "Bypass" },
    };
    this->uiSpec.meters      = { { "IN" }, { "OUT", .hasScale = true } };
    this->lpf.settings.shape = NtFx::Biquad::Shape::lpf;
    this->hpf.settings.shape = NtFx::Biquad::Shape::hpf;
    this->lpf.settings.fc_hz = 20e3;
    this->hpf.settings.fc_hz = 20;
    this->updateDefaults();
  }

  virtual NtFx::Stereo<signal_t> process(
      NtFx::Stereo<signal_t> x) noexcept override {
    this->nDelay.process();
    this->modDepth.process();
    this->thetaMod.process();
    this->thetaModPhase.process();
    this->nOffset.process();
    auto xNoisy = x + NtFx::rand<signal_t>() * this->noise_lin;
    NtFx::ensureFinite<NtFx::Stereo<signal_t>>(xNoisy);
    NtFx::ensureFinite<NtFx::Stereo<signal_t>>(this->fbState);
    this->iStore++;
    if (this->iStore >= delayLineLength) { this->iStore = 0; }
    this->delayLine[this->iStore] = xNoisy + this->fb_lin * this->fbState;

    int nModL = this->nDelay.pr;
    int nModR = this->nDelay.pr + this->nOffset.pr;
    if (this->mod) {
      auto modSawL = NtFx::saw(this->thetaMod.pr * this->timeCounter);
      auto modSawR = NtFx::saw(
          this->thetaMod.pr * this->timeCounter + this->thetaModPhase.pr);
      nModL = std::round(modSawL * this->nDelay.pr * this->modDepth.pr)
          + this->nDelay.pr;
      nModR = std::round(modSawR * this->nDelay.pr * this->modDepth.pr)
          + this->nDelay.pr + this->nOffset.pr;
      this->timeCounter++;
    }

    // TODO: DelayLine class.
    int iLoadL = this->iStore - std::round(nModL);
    if (iLoadL < 0) { iLoadL += delayLineLength; }
    int iLoadR = this->iStore - std::round(nModR);
    if (iLoadR < 0) { iLoadR += delayLineLength; }
    NtFx::Stereo<signal_t> yDelay = {
      this->delayLine[iLoadL].l,
      this->delayLine[iLoadR].r,
    };
    auto yFbClip =
        NtFx::softClip3rdStereo(yDelay * this->aClip_lin) / aClip_lin;
    NtFx::ensureFinite<NtFx::Stereo<signal_t>>(yFbClip);
    auto yHp      = hpf.process(yFbClip);
    auto yLp      = lpf.process(yHp);
    this->fbState = yLp;
    auto yOutClip = yLp;
    if (this->clip) { yOutClip = NtFx::softClip5thStereo(yLp); }
    auto y = (static_cast<signal_t>(1.0) - this->mix_lin) * x
        + this->mix_lin * yOutClip;
    // TODO: Make this a member.
    y *= (2 - gcem::abs(this->mix_lin * 2 - 1));
    this->template updatePeakLevel<0>(x);
    if (this->bypass) {
      this->template updatePeakLevel<1>(x);
      return x;
    }
    this->template updatePeakLevel<1>(y);
    return y;
  }

  virtual void update() noexcept override {
    this->hpf.update();
    this->lpf.update();
    this->nOffset.ui  = std::round(this->tOffset / 1000 * this->fs);
    this->aClip_lin   = NtFx::invDb(this->clipG_db);
    this->mix_lin     = this->mix_percent / 100;
    this->fb_lin      = this->fb_percent / 100;
    this->noise_lin   = NtFx::invDb(this->noise_db);
    this->modDepth.ui = this->modDepth_percent / 100;
    switch (this->subDev) {
    case SubDev::half:
      this->tempoScale = 2;
      break;
    case SubDev::fourth:
      this->tempoScale = 1;
      break;
    case SubDev::eighth_dot:
      this->tempoScale = 0.5 * 1.5;
      break;
    case SubDev::eighth:
      this->tempoScale = 0.5;
      break;
    case SubDev::sixteenth_dot:
      this->tempoScale = 0.25 * 1.5;
      break;
    case SubDev::sixteenth:
      this->tempoScale = 0.25;
      break;
    }
    this->thetaMod.ui      = 2.0 * GCEM_PI * this->modFreq / this->fs;
    this->thetaModPhase.ui = this->modPhase * GCEM_PI / 180;
    this->onTempoChanged();
    signal_t tGlide = 0.0;
    if (this->doGlide) { tGlide = this->tGlide; }
    this->modDepth.update(this->fs, tGlide);
    // TODO: fix mod glider.
    // this->thetaMod.update(this->fs, tGlide);
    this->thetaMod.update(this->fs, 0);
    this->thetaModPhase.update(this->fs, tGlide);
    this->nOffset.update(this->fs, tGlide);
    this->nDelay.update(this->fs, tGlide);
  }

  virtual void reset(float fs) noexcept override {
    this->fs = fs;
    std::fill(this->delayLine.begin(), this->delayLine.end(), 0);
    this->hpf.reset(this->fs);
    this->lpf.reset(this->fs);
    this->update();
  }

  virtual void onTempoChanged() noexcept override {
    if (this->sync && this->tempo) {
      this->tGui                     = 60 / this->tempo * this->tempoScale;
      this->primaryKnobs[0].isActive = false;
    } else {
      this->primaryKnobs[0].isActive = true;
    }
    this->uiNeedsUpdate = true;
    this->nDelay.ui     = std::round(this->tGui * this->fs);
  }
};
