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
 *
 **/

#pragma once

#include "lib/FirstOrder.h"
#include "lib/Plugin.h"
#include "lib/SideChain.h"
#include "lib/Stereo.h"
#include "lib/utils.h"
#include <array>
#include <string>

enum Bands { hi, mid, lo, n };

template <typename signal_t>
struct ntMultiband3 : public NtFx::NtPlugin<signal_t> {
  signal_t xOverLo_hz { 200 };
  signal_t xOverHi_hz { 4000 };
  signal_t ouputGain_db { 0 };
  signal_t ouputGain_lin { 1 };
  bool linkEnable;
  bool bypass;

  std::array<NtFx::SideChain<signal_t, true>, 3> sc;
  // TODO: Add makeup gain to sidechain?
  std::array<signal_t, 3> makeup_db;
  std::array<signal_t, 3> makeup_lin;
  const std::array<std::string, Bands::n> BandNames = { "High", "Mid", "Low" };
  NtFx::FirstOrder::StereoFilter<signal_t, NtFx::FirstOrder::Shape::lpf> loFlt;
  NtFx::FirstOrder::StereoFilter<signal_t, NtFx::FirstOrder::Shape::hpf>
      loMidFlt;
  NtFx::FirstOrder::StereoFilter<signal_t, NtFx::FirstOrder::Shape::lpf>
      hiMidFlt;
  NtFx::FirstOrder::StereoFilter<signal_t, NtFx::FirstOrder::Shape::hpf> hiFlt;
  ntMultiband3() {
    this->uiSpec.maxColumns = 5;
    this->uiSpec.maxRows    = Bands::n;

    for (size_t i = 0; i < Bands::n; i++) {
      this->primaryKnobs.push_back({
          .p_val  = &this->sc[i].settings.thresh_db,
          .name   = this->BandNames[i] + "_Threshold",
          .suffix = " dB",
          .minVal = -60,
          .maxVal = 0,
      });
      this->primaryKnobs.push_back({
          .p_val    = &this->sc[i].settings.ratio_db,
          .name     = this->BandNames[i] + "_Ratio",
          .suffix   = "",
          .minVal   = 1.0,
          .maxVal   = 20.0,
          .midPoint = 2.0,
      });
      this->primaryKnobs.push_back({
          .p_val  = &this->sc[i].settings.tAtt_ms,
          .name   = this->BandNames[i] + "_Attack",
          .suffix = " ms",
          .minVal = 0.01,
          .maxVal = 50.0,
      });
      this->primaryKnobs.push_back({
          .p_val  = &this->sc[i].settings.tRel_ms,
          .name   = this->BandNames[i] + "_Release",
          .suffix = " ms",
          .minVal = 10.0,
          .maxVal = 1000.0,

      });
      this->primaryKnobs.push_back({
          .p_val  = &this->makeup_db[i],
          .name   = this->BandNames[i] + "_Makeup",
          .suffix = " dB",
          .minVal = 0.0,
          .maxVal = 24.0,
      });
    }
    this->secondaryKnobs = {
      { &this->xOverLo_hz, "Lo_Xover", " Hz", 20, 2000, 200 },
      { &this->xOverHi_hz, "Hi_Xover", " Hz", 200, 20000, 2000 },
      { &this->ouputGain_db, "Out", " dB", -12, 12 },
    };
    this->toggles = {
      { &this->linkEnable, "Link" },
      { &this->bypass, "Bypass" },
    };
    this->uiSpec.meters = { { "IN" }, { "OUT", true } };
    for (int i = Bands::n - 1; i >= 0; i--) {
      this->uiSpec.meters.push_back({ this->BandNames[i], true });
    }
    this->uiSpec.meters[Bands::n - 1 + 2].hasScale = true;
    this->updateDefaults();
  }

  NtFx::Stereo<signal_t> process(NtFx::Stereo<signal_t> x) noexcept override {
    if (this->bypass) { return x; }
    std::array<NtFx::Stereo<signal_t>, 3> xComp;
    xComp[Bands::hi]  = this->hiFlt.process(x);
    auto xLoMidFlt    = this->hiMidFlt.process(x);
    xComp[Bands::mid] = this->loMidFlt.process(xLoMidFlt);
    xComp[Bands::lo]  = this->loFlt.process(x);
    std::array<NtFx::Stereo<signal_t>, Bands::n> gr;
    for (size_t i = 0; i < Bands::n; i++) {
      gr[i] = this->sc[i].process(xComp[i]);
      if (this->linkEnable) { gr[i] = gr[i].absMin(); }
    }
    NtFx::Stereo<signal_t> yComp;
    for (size_t i = 0; i < Bands::n; i++) {
      yComp += xComp[i] * gr[i] * this->makeup_lin[i];
    }
    auto y = yComp * this->ouputGain_lin;
    this->template updatePeakLevel<0>(x);
    this->template updatePeakLevel<1>(y);
    this->template updatePeakLevel<2, true>(gr[Bands::lo]);
    this->template updatePeakLevel<3, true>(gr[Bands::mid]);
    this->template updatePeakLevel<4, true>(gr[Bands::hi]);
    return y;
  }

  void update() noexcept override {
    this->hiFlt.setFc(this->xOverHi_hz);
    this->hiMidFlt.setFc(this->xOverHi_hz);
    this->loMidFlt.setFc(this->xOverLo_hz);
    this->loFlt.setFc(this->xOverLo_hz);
    this->hiFlt.update();
    this->hiMidFlt.update();
    this->loMidFlt.update();
    this->loFlt.update();
    this->ouputGain_lin = NtFx::invDb(this->ouputGain_db);
    for (size_t i = 0; i < Bands::n; i++) {
      this->makeup_lin[i] = NtFx::invDb(makeup_db[i]);
      this->sc[i].update();
    }
  }

  void reset(float fs) noexcept override {
    this->fs = fs;
    this->hiFlt.reset(this->fs);
    this->hiMidFlt.reset(this->fs);
    this->loMidFlt.reset(this->fs);
    this->loFlt.reset(this->fs);
    for (size_t i = 0; i < Bands::n; i++) { this->sc[i].reset(this->fs); }
    this->update();
  }
};
