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
#include <array>

enum Band { hi, mid, lo };

template <typename signal_t>
struct ntMultiband3 : public NtFx::NtPlugin<signal_t> {
  signal_t fs;

  signal_t xOverLo_hz { 200 };
  signal_t xOverHi_hz { 4000 };
  signal_t ouputGain_db { 0 };
  signal_t ouputGain_lin { 1 };
  bool bypass;

  std::array<NtFx::SideChain::Settings<signal_t>, 3> scSettings;
  std::array<NtFx::SideChain::Coeffs<signal_t>, 3> scCoeffs;
  std::array<NtFx::SideChain::State<signal_t>, 3> scState;
  std::array<signal_t, 3> makeup_db;
  std::array<signal_t, 3> makeup_lin;

  NtFx::FirstOrder::FilterStereo<signal_t, NtFx::FirstOrder::Shape::lpf> loFlt;
  NtFx::FirstOrder::FilterStereo<signal_t, NtFx::FirstOrder::Shape::hpf>
      loMidFlt;
  NtFx::FirstOrder::FilterStereo<signal_t, NtFx::FirstOrder::Shape::lpf>
      hiMidFlt;
  NtFx::FirstOrder::FilterStereo<signal_t, NtFx::FirstOrder::Shape::hpf> hiFlt;
  ntMultiband3() {
    this->uiSpec.maxColumns = 5;
    this->uiSpec.maxRows    = 3;

    this->primaryKnobs = {
      {
          {
              .p_val  = &this->scSettings[Band::hi].thresh_db,
              .name   = "Hi_Threshold",
              .suffix = " dB",
              .minVal = -60.0,
              .maxVal = 0.0,
          },
          {
              .p_val    = &this->scSettings[Band::hi].ratio_db,
              .name     = "Hi_Ratio",
              .suffix   = "",
              .minVal   = 1.0,
              .maxVal   = 20.0,
              .midPoint = 2.0,
          },
          {
              .p_val  = &this->scSettings[Band::hi].tAtt_ms,
              .name   = "Hi_Attack",
              .suffix = " ms",
              .minVal = 0.01,
              .maxVal = 50.0,
          },
          {
              .p_val  = &this->scSettings[Band::hi].tRel_ms,
              .name   = "Hi_Release",
              .suffix = " ms",
              .minVal = 10.0,
              .maxVal = 1000.0,
          },
          {
              .p_val  = &this->makeup_db[Band::hi],
              .name   = "Hi_Makeup",
              .suffix = " dB",
              .minVal = 0.0,
              .maxVal = 24.0,
          },
          {
              .p_val  = &this->scSettings[Band::mid].thresh_db,
              .name   = "Mid_Threshold",
              .suffix = " dB",
              .minVal = -60.0,
              .maxVal = 0.0,
          },
          {
              .p_val    = &this->scSettings[Band::mid].ratio_db,
              .name     = "Mid_Ratio",
              .suffix   = "",
              .minVal   = 1.0,
              .maxVal   = 20.0,
              .midPoint = 2.0,
          },
          {
              .p_val  = &this->scSettings[Band::mid].tAtt_ms,
              .name   = "Mid_Attack",
              .suffix = " ms",
              .minVal = 0.01,
              .maxVal = 50.0,
          },
          {
              .p_val  = &this->scSettings[Band::mid].tRel_ms,
              .name   = "Mid_Release",
              .suffix = " ms",
              .minVal = 10.0,
              .maxVal = 1000.0,
          },
          {
              .p_val  = &this->makeup_db[Band::mid],
              .name   = "Mid_Makeup",
              .suffix = " dB",
              .minVal = 0.0,
              .maxVal = 24.0,
          },
          {
              .p_val  = &this->scSettings[Band::lo].thresh_db,
              .name   = "Low_Threshold",
              .suffix = " dB",
              .minVal = -60.0,
              .maxVal = 0.0,
          },
          {
              .p_val    = &this->scSettings[Band::lo].ratio_db,
              .name     = "Low_Ratio",
              .suffix   = "",
              .minVal   = 1.0,
              .maxVal   = 20.0,
              .midPoint = 2.0,
          },
          {
              .p_val  = &this->scSettings[Band::lo].tAtt_ms,
              .name   = "Low_Attack",
              .suffix = " ms",
              .minVal = 0.01,
              .maxVal = 50.0,
          },
          {
              .p_val  = &this->scSettings[Band::lo].tRel_ms,
              .name   = "Low_Release",
              .suffix = " ms",
              .minVal = 10.0,
              .maxVal = 1000.0,
          },
          {
              .p_val  = &this->makeup_db[Band::lo],
              .name   = "Low_Makeup",
              .suffix = " dB",
              .minVal = 0.0,
              .maxVal = 24.0,
          },
      },
    };
    this->secondaryKnobs = {
      { &this->xOverLo_hz, "Lo_Xover", " Hz", 20, 2000, 200 },
      { &this->xOverHi_hz, "Hi_Xover", " Hz", 200, 20000, 2000 },
      { &this->ouputGain_db, "Out", " dB", -12, 12 },
    };
    this->toggles       = { { &this->bypass, "Bypass" } };
    this->uiSpec.meters = {
      { .name = "IN" },
      { .name = "OUT", .hasScale = true },
      { .name = "LOW", .invert = true, .hasScale = true },
      { .name = "MID", .invert = true },
      { .name = "HI", .invert = true, .hasScale = true },
    };
    this->updateDefaults();
  }

  NtFx::Stereo<signal_t> process(NtFx::Stereo<signal_t> x) noexcept override {
    if (this->bypass) { return x; }
    auto xHi       = this->hiFlt.process(x);
    auto xLoMidFlt = this->hiMidFlt.process(x);
    auto xMid      = this->loMidFlt.process(xLoMidFlt);
    auto xLo       = this->loFlt.process(x);
    // TODO: What we want here is to call sc[Band::hi].process(xScHi)
    NtFx::Stereo<signal_t> grHi = {
      NtFx::SideChain::sideChain_lin(
          &this->scCoeffs[Band::hi], &this->scState[Band::hi], xHi.l),
      NtFx::SideChain::sideChain_lin(
          &this->scCoeffs[Band::hi], &this->scState[Band::hi], xHi.r)
    };
    NtFx::Stereo<signal_t> grMid = {
      NtFx::SideChain::sideChain_lin(
          &this->scCoeffs[Band::mid], &this->scState[Band::mid], xMid.l),
      NtFx::SideChain::sideChain_lin(
          &this->scCoeffs[Band::mid], &this->scState[Band::mid], xMid.r)
    };
    NtFx::Stereo<signal_t> grLo = {
      NtFx::SideChain::sideChain_lin(
          &this->scCoeffs[Band::lo], &this->scState[Band::lo], xLo.l),
      NtFx::SideChain::sideChain_lin(
          &this->scCoeffs[Band::lo], &this->scState[Band::lo], xLo.r)
    };
    auto y = xHi * grHi + xMid * grMid + xLo * grLo;

    this->template updatePeakLevel<0>(x);
    this->template updatePeakLevel<1>(y);
    this->template updatePeakLevel<2, true>(grLo);
    this->template updatePeakLevel<3, true>(grMid);
    this->template updatePeakLevel<4, true>(grHi);
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
    for (size_t i = 0; i < this->scSettings.size(); i++) {
      this->scCoeffs[i] =
          NtFx::SideChain::calcCoeffs(this->fs, &this->scSettings[i]);
    }
  }

  void reset(int fs) noexcept override {
    this->fs = fs;
    this->hiFlt.reset(this->fs);
    this->hiMidFlt.reset(this->fs);
    this->loMidFlt.reset(this->fs);
    this->loFlt.reset(this->fs);
  }
};