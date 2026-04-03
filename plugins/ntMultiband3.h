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

/**
 * @brief A 3-band multiband compressor with first order crossovers. Uses a
 * linear domain, peak sensing sidechain for each band.
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
  bool linkEnable { false };
  bool feedbackEnable { false };
  bool bypass { false };
  // bool noise { false };

  std::array<bool, Bands::n> solos   = { false, false, false };
  std::array<bool, Bands::n> mutesUi = { false, false, false };
  std::array<bool, Bands::n> mutes   = { false, false, false };

  std::array<NtFx::ScSettings<signal_t>, 3> scSettings;
  std::array<NtFx::PeakSideChainLinear<signal_t>, 3> sc;
  // TODO: Add makeup gain to sidechain?
  std::array<signal_t, Bands::n> makeup_db;
  std::array<signal_t, Bands::n> makeup_lin;
  std::array<NtFx::Stereo<signal_t>, Bands::n> fbState;
  const std::array<std::string, Bands::n> BandNames = { "High", "Mid", "Low" };
  NtFx::FirstOrder::StereoFilter<signal_t, NtFx::FirstOrder::Shape::lpfZero>
      loFlt;
  NtFx::FirstOrder::StereoFilter<signal_t, NtFx::FirstOrder::Shape::hpf>
      loMidFlt;
  NtFx::FirstOrder::StereoFilter<signal_t, NtFx::FirstOrder::Shape::lpfZero>
      hiMidFlt;
  NtFx::FirstOrder::StereoFilter<signal_t, NtFx::FirstOrder::Shape::hpf> hiFlt;
  ntMultiband3() : sc { scSettings[0], scSettings[1], scSettings[2] } {
    this->uiSpec.maxColumns         = 5;
    this->uiSpec.maxRows            = Bands::n;
    this->uiSpec.defaultWindowWidth = 1200;

    for (size_t i = 0; i < Bands::n; i++) {
      this->primaryKnobs.push_back({
          .p_val  = &this->scSettings[i].thresh_db,
          .name   = this->BandNames[i] + "_Threshold",
          .suffix = " dB",
          .minVal = -60,
          .maxVal = 0,
      });
      this->primaryKnobs.push_back({
          .p_val    = &this->scSettings[i].ratio_db,
          .name     = this->BandNames[i] + "_Ratio",
          .suffix   = "",
          .minVal   = 1.0,
          .maxVal   = 20.0,
          .midPoint = 2.0,
      });
      this->primaryKnobs.push_back({
          .p_val    = &this->scSettings[i].tAtt_ms,
          .name     = this->BandNames[i] + "_Attack",
          .suffix   = " ms",
          .minVal   = 0.01,
          .maxVal   = 50.0,
          .midPoint = 5,
      });
      this->primaryKnobs.push_back({
          .p_val    = &this->scSettings[i].tRel_ms,
          .name     = this->BandNames[i] + "_Release",
          .suffix   = " ms",
          .minVal   = 10.0,
          .maxVal   = 1000.0,
          .midPoint = 100.0,

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
      { &this->feedbackEnable, "Feedback" },
      { &this->bypass, "Bypass" },
      // { &this->noise, "Noise" },
    };
    this->toggleSets = {
      { "Solo", { } },
      { "Mute", { } },
    };
    for (size_t i = 0; i < Bands::n; i++) {
      this->toggleSets[0].toggles.push_back({
          .p_val = &this->solos[i],
          .name  = BandNames[i],
      });
      this->toggleSets[1].toggles.push_back({
          .p_val = &this->mutesUi[i],
          .name  = BandNames[i],
      });
    }
    this->meters = {
      { .name = "IN", .decay_s = 0.75, .addRms = true },
      { .name = "OUT", .hasScale = true, .decay_s = 0.75, .addRms = true },
    };
    for (int i = Bands::n - 1; i >= 0; i--) {
      this->meters.push_back({ .name = this->BandNames[i], .invert = true });
    }
    this->meters[Bands::n - 1 + 2].hasScale = true;
    this->uiSpec.meterHeight_dots           = 25;
    for (auto& m : this->meters) { m.minVal_db = -50; }
    this->updateDefaults();
  }

  NtFx::Stereo<signal_t> process(NtFx::Stereo<signal_t> x) noexcept override {
    if (this->bypass) { return x; }
    std::array<NtFx::Stereo<signal_t>, 3> xComp;
    xComp[Bands::hi]  = this->hiFlt.process(x);
    auto xLoMidFlt    = this->hiMidFlt.process(x);
    xComp[Bands::mid] = this->loMidFlt.process(xLoMidFlt);
    xComp[Bands::lo]  = this->loFlt.process(x);
    std::array<NtFx::Stereo<signal_t>, 3> xSc;
    if (this->feedbackEnable) {
      for (size_t i = 0; i < Bands::n; i++) { xSc[i] = this->fbState[i]; }
    } else {
      for (size_t i = 0; i < Bands::n; i++) { xSc[i] = xComp[i]; }
    }
    std::array<NtFx::Stereo<signal_t>, Bands::n> gr;
    for (size_t i = 0; i < Bands::n; i++) {
      gr[i] = this->sc[i].process(xSc[i]);
      if (this->linkEnable) { gr[i] = gr[i].absMin(); }
    }
    NtFx::Stereo<signal_t> yComp;
    for (size_t i = 0; i < Bands::n; i++) {
      auto tmp         = xComp[i] * gr[i];
      this->fbState[i] = tmp;
      if (!this->mutes[i]) { yComp += tmp * this->makeup_lin[i]; }
    }
    auto y = yComp * this->ouputGain_lin;
    this->template updatePeakLevel<0>(x);
    this->template updatePeakLevel<1>(y);
    this->template updatePeakLevel<2, true>(gr[Bands::lo]);
    this->template updatePeakLevel<3, true>(gr[Bands::mid]);
    this->template updatePeakLevel<4, true>(gr[Bands::hi]);
    // if (this->noise) { return NtFx::rand<signal_t>(); }s
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
    this->_updateMutes();
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

  void _updateMutes() {
    for (size_t i = 0; i < Bands::n; i++) { this->mutes[i] = this->mutesUi[i]; }
    for (size_t i = 0; i < Bands::n; i++) {
      if (!this->solos[i]) { continue; }
      for (size_t j = 0; j < Bands::n; j++) {
        if (i != j && !this->solos[j]) { this->mutes[j] = true; }
      }
    }
  }
};
