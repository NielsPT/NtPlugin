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
#include <algorithm>
#include <array>

#include "lib/Biquad.h"
#include "lib/Plugin.h"
#include "lib/SideChain.h"
#include "lib/SoftClip.h"
#include "lib/Stereo.h"
#include "lib/utils.h"

template <typename signal_t>
struct ntCompressor : public NtFx::NtPlugin<signal_t> {
  NtFx::SideChain<signal_t, false> scLin;
  NtFx::SideChain<signal_t, true> scDb;
  signal_t makeup_db   = static_cast<signal_t>(0.0);
  signal_t mix_percent = static_cast<signal_t>(100.0);

  bool bypassEnable   = false;
  bool linEnable      = false;
  bool feedbackEnable = false;
  bool scListenEnable = false;
  bool linkEnable     = false;

  signal_t mix_lin               = static_cast<signal_t>(1.0);
  signal_t makeup_lin            = static_cast<signal_t>(1.0);
  NtFx::Stereo<signal_t> fbState = static_cast<signal_t>(0.0);
  std::array<signal_t, 3> softClipCoeffs;

  NtFx::Biquad::EqBand<signal_t> hpf;
  NtFx::Biquad::EqBand<signal_t> boost;

  ntCompressor() {
    this->primaryKnobs = {
      {
          .p_val  = &this->sc.settings.thresh_db,
          .name   = "Threshold",
          .suffix = " dB",
          .minVal = -60.0,
          .maxVal = 0.0,
      },
      {
          .p_val    = &this->sc.settings.ratio_db,
          .name     = "Ratio",
          .suffix   = "",
          .minVal   = 1.0,
          .maxVal   = 20.0,
          .midPoint = 2.0,
      },
      {
          .p_val  = &this->sc.settings.tAtt_ms,
          .name   = "Attack",
          .suffix = " ms",
          .minVal = 0.01,
          .maxVal = 50.0,
      },
      {
          .p_val  = &this->sc.settings.tRel_ms,
          .name   = "Release",
          .suffix = " ms",
          .minVal = 10.0,
          .maxVal = 1000.0,
      },
      {
          .p_val  = &this->makeup_db,
          .name   = "Makeup",
          .suffix = " dB",
          .minVal = 0.0,
          .maxVal = 24.0,
      },
    };

    this->secondaryKnobs = {
      {
          .p_val  = &this->sc.settings.knee_db,
          .name   = "Knee",
          .suffix = " dB",
          .minVal = 0.0,
          .maxVal = 24.0,
      },
      {
          .p_val  = &this->sc.settings.tRms_ms,
          .name   = "RMS_time",
          .suffix = " ms",
          .minVal = 1.0,
          .maxVal = 80.0,
      },
      {
          .p_val    = &this->hpf.settings.fc_hz,
          .name     = "SC_HPF",
          .suffix   = " hz",
          .minVal   = 20.0,
          .maxVal   = 2000.0,
          .midPoint = 200.0,
      },
      {
          .p_val  = &this->boost.settings.gain_db,
          .name   = "SC_Boost",
          .suffix = " dB",
          .minVal = 0.0,
          .maxVal = 24.0,
      },
      {
          .p_val  = &this->mix_percent,
          .name   = "Mix",
          .suffix = " %",
          .minVal = 0.0,
          .maxVal = 100.0,
      },
    };

    this->toggles = {
      { .p_val = &this->sc.settings.rmsEnable, .name = "RMS" },
      { .p_val = &this->feedbackEnable, .name = "Feedback" },
      { .p_val = &this->linEnable, .name = "Linear" },
      { .p_val = &this->linkEnable, .name = "Link" },
      { .p_val = &this->scListenEnable, .name = "SC_Listen" },
      { .p_val = &this->bypassEnable, .name = "Bypass" },
    };

    this->uiSpec.meters = {
      { .name = "IN" },
      { .name = "OUT", .hasScale = true },
      { .name = "GR", .invert = true, .hasScale = true },
    };
    this->uiSpec.foregroundColour = 0xFF000000;
    this->uiSpec.backgroundColour = 0xFFFFFFFF;
    // this->uiSpec.maxColumns         = 2;
    // this->uiSpec.defaultWindowWidth = 600;
    this->softClipCoeffs       = NtFx::calculateSoftClipCoeffs<signal_t, 2>();
    this->hpf.settings.fc_hz   = 20;
    this->boost.settings.fc_hz = 3000.0;
    this->hpf.settings.shape   = NtFx::Biquad::Shape::hpf;
    this->boost.settings.shape = NtFx::Biquad::Shape::bell;

    this->updateDefaults();
  }

  NtFx::Stereo<signal_t> process(NtFx::Stereo<signal_t> x) noexcept override {
    this->template updatePeakLevel<0>(x);
    if (this->bypassEnable) {
      this->template updatePeakLevel<1>(x);
      return x;
    }
    NtFx::ensureFinite(x);
    NtFx::ensureFinite(this->fbState);
    NtFx::Stereo<signal_t> xHpf = x;
    if (this->feedbackEnable) { xHpf = this->fbState; }

    NtFx::Stereo<signal_t> xBoost = hpf.process(xHpf);
    NtFx::Stereo<signal_t> xSc    = boost.process(xBoost);

    NtFx::Stereo<signal_t> gr;
    if (this->linEnable) {
      gr = scLin.process(xSc);
    } else {
      gr = scDb.process(xSc);
    }
    if (this->linkEnable) { gr = gr.absMin(); }
    this->template updatePeakLevel<2, true>(gr);
    NtFx::ensureFinite(gr, static_cast<signal_t>(1.0));
    NtFx::Stereo<signal_t> yComp = x * gr;
    this->fbState                = yComp;
    auto ySoftClip               = NtFx::softClip5thStereo<signal_t>(
        this->softClipCoeffs, yComp * this->makeup_lin);
    auto y = this->mix_lin * ySoftClip + (1 - this->mix_lin) * x;
    this->template updatePeakLevel<1>(y);
    if (this->scListenEnable) { return xSc; }
    return y;
  }

  void update() noexcept override {
    this->hpf.update(this->fs);
    this->boost.update(this->fs);
    this->sc.update();
    this->makeup_lin = NtFx::invDb(this->makeup_db);
    this->mix_lin    = this->mix_percent / 100.0;
  }

  void reset(float fs) noexcept override {
    this->fs = fs;
    std::fill(this->peakLevels.begin(),
        this->peakLevels.end(),
        static_cast<signal_t>(0));
    this->peakLevels[2] = static_cast<signal_t>(1);
    this->fbState       = static_cast<signal_t>(0);
    this->sc.reset(this->fs);
    this->update();
  }
};
