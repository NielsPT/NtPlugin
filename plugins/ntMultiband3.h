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

#include "lib/Audio.h"
#include "lib/Comp.h"
#include "lib/FirstOrder.h"
#include "lib/Plugin.h"
#include "lib/utils.h"
#include <array>
#include <string>

enum Bands { hi, mid, lo, n };
constexpr std::array<std::string, Bands::n> bandNames = {
  "High", "Mid", "Low"
};

struct ntMultiband3 : public NtFx::NtPlugin {
  signal_t xOverLo_hz { 200 };
  signal_t xOverHi_hz { 4000 };
  signal_t ouputGain_db { 0 };
  signal_t ouputGain_lin { 1 };
  bool linkEnable { false };
  bool feedbackEnable { false };
  bool bypassEnable { false };
  // bool noise { false };

  std::array<bool, Bands::n> solos        = { false, false, false };
  std::array<bool, Bands::n> mutesUi      = { false, false, false };
  std::array<bool, Bands::n> mutes        = { false, false, false };
  std::array<bool, Bands::n> compDisables = { false, false, false };

  std::array<NtFx::Comp::ScSettings, Bands::n> scSettings;
  std::array<NtFx::Comp::PeakSideChainLinear, Bands::n> sc;
  std::array<signal_t, Bands::n> makeup_db { 0, 0, 0 };
  std::array<signal_t, Bands::n> makeup_lin { 1, 1, 1 };
  std::array<Audio, Bands::n> fbState { 0, 0, 0 };
  NtFx::FirstOrder::StereoFilter<NtFx::FirstOrder::Shape::lpf> loFlt;
  NtFx::FirstOrder::StereoFilter<NtFx::FirstOrder::Shape::hpf> loMidFlt;
  NtFx::FirstOrder::StereoFilter<NtFx::FirstOrder::Shape::lpf> hiMidFlt;
  NtFx::FirstOrder::StereoFilter<NtFx::FirstOrder::Shape::hpf> hiFlt;
  ntMultiband3() : sc { scSettings[0], scSettings[1], scSettings[2] } {
    this->uiSpec.maxColumns = 5;
    this->uiSpec.maxRows    = Bands::n;

    for (size_t i = 0; i < Bands::n; i++) {
      this->primaryKnobs.push_back({
          .p_val  = &this->scSettings[i].thresh_db,
          .name   = bandNames[i] + "_Threshold",
          .suffix = " dB",
          .minVal = -60,
          .maxVal = 0,
      });
      this->primaryKnobs.push_back({
          .p_val    = &this->scSettings[i].ratio,
          .name     = bandNames[i] + "_Ratio",
          .suffix   = "",
          .minVal   = 1.0,
          .maxVal   = 20.0,
          .midPoint = 2.0,
      });
      this->primaryKnobs.push_back({
          .p_val    = &this->scSettings[i].tAtt_ms,
          .name     = bandNames[i] + "_Attack",
          .suffix   = " ms",
          .minVal   = 0.01,
          .maxVal   = 50.0,
          .midPoint = 5,
      });
      this->primaryKnobs.push_back({
          .p_val    = &this->scSettings[i].tRel_ms,
          .name     = bandNames[i] + "_Release",
          .suffix   = " ms",
          .minVal   = 10.0,
          .maxVal   = 1000.0,
          .midPoint = 100.0,

      });
      this->primaryKnobs.push_back({
          .p_val  = &this->makeup_db[i],
          .name   = bandNames[i] + "_Makeup",
          .suffix = " dB",
          .minVal = 0.0,
          .maxVal = 24.0,
      });
    }
    this->secondaryKnobs = {
      {
          .p_val    = &this->xOverLo_hz,
          .name     = "Lo Xover",
          .suffix   = " Hz",
          .minVal   = 20,
          .maxVal   = 2000,
          .midPoint = 200,
      },
      {
          .p_val    = &this->xOverHi_hz,
          .name     = "Hi Xover",
          .suffix   = " Hz",
          .minVal   = 200,
          .maxVal   = 20000,
          .midPoint = 2000,
      },
      {
          .p_val  = &this->ouputGain_db,
          .name   = "Out",
          .suffix = " dB",
          .minVal = -12,
          .maxVal = 12,
      },
    };
    this->toggles = {
      { &this->linkEnable, "Link" },
      { &this->feedbackEnable, "Feedback" },
      { &this->bypassEnable, "Bypass" },
      // { &this->noise, "Noise" },
    };
    this->toggleSets = {
      { "Solo", { } },
      { "Mute", { } },
      { "Comp Off", { } },
    };
    for (size_t i = 0; i < Bands::n; i++) {
      this->toggleSets[0].toggles.push_back({
          .p_val = &this->solos[i],
          .name  = bandNames[i],
      });
      this->toggleSets[1].toggles.push_back({
          .p_val = &this->mutesUi[i],
          .name  = bandNames[i],
      });
      this->toggleSets[2].toggles.push_back({
          .p_val = &this->compDisables[i],
          .name  = bandNames[i],
      });
    }
    this->meters = {
      { .name = "IN", .decay_s = 0.75, .addRms = true },
      { .name = "OUT", .hasScale = true, .decay_s = 0.75, .addRms = true },
    };
    for (int i = Bands::n - 1; i >= 0; i--) {
      this->meters.push_back({ .name = bandNames[i], .invert = true });
    }
    this->meters[Bands::n - 1 + 2].hasScale = true;
    this->uiSpec.meterHeight_dots           = 25;
    this->uiSpec.pad                        = 15;
    for (auto& m : this->meters) { m.minVal_db = -50; }
    this->updateDefaults();
  }

  Audio process(Audio x) noexcept override {
    if (this->bypassEnable) { return x; }
    std::array<Audio, 3> xComp;
    xComp[Bands::hi]  = this->hiFlt.process(x);
    auto xLoMidFlt    = this->hiMidFlt.process(x);
    xComp[Bands::mid] = this->loMidFlt.process(xLoMidFlt);
    xComp[Bands::lo]  = this->loFlt.process(x);
    std::array<Audio, 3> xSc;
    if (this->feedbackEnable) {
      for (size_t i = 0; i < Bands::n; i++) { xSc[i] = this->fbState[i]; }
    } else {
      for (size_t i = 0; i < Bands::n; i++) { xSc[i] = xComp[i]; }
    }
    std::array<Audio, Bands::n> gr;
    for (size_t i = 0; i < Bands::n; i++) {
      if (this->compDisables[i]) {
        gr[i] = 1;
        continue;
      }
      gr[i] = this->sc[i].process(xSc[i]);
      if (this->linkEnable) { gr[i] = gr[i].absMin(); }
    }
    Audio yComp;
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
    // if (this->noise) { return NtFx::rand() * 0.125; }
    return y;
  }

  void update() noexcept override {
    this->hiFlt.fc_hz    = this->xOverHi_hz;
    this->hiMidFlt.fc_hz = this->xOverHi_hz;
    this->loMidFlt.fc_hz = this->xOverLo_hz;
    this->loFlt.fc_hz    = this->xOverLo_hz;
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
