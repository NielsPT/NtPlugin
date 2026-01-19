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
  int errorVarId = 0;
  float fs;

  NtFx::SideChain::Settings<signal_t> scSettings;
  NtFx::SideChain::Coeffs<signal_t> scCoeffs;
  std::array<NtFx::SideChain::State<signal_t>, 2> scState;
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

  NtFx::Biquad::Settings<signal_t> scHpfSettings;
  NtFx::Biquad::Settings<signal_t> scBoostSettings;
  NtFx::Biquad::Coeffs5<signal_t> scHpfCoeffs;
  NtFx::Biquad::Coeffs5<signal_t> scBoostCoeffs;
  std::array<NtFx::Biquad::State<signal_t>, 4> bqState;

  constexpr ntCompressor() {
    this->primaryKnobs = {
      {
          .p_val  = &this->scSettings.thresh_db,
          .name   = "Threshold",
          .suffix = " dB",
          .minVal = -60.0,
          .maxVal = 0.0,
      },
      {
          .p_val    = &this->scSettings.ratio_db,
          .name     = "Ratio",
          .suffix   = "",
          .minVal   = 1.0,
          .maxVal   = 20.0,
          .midPoint = 2.0,
      },

      {
          .p_val  = &this->scSettings.tAtt_ms,
          .name   = "Attack",
          .suffix = " ms",
          .minVal = 0.01,
          .maxVal = 50.0,
      },
      {
          .p_val  = &this->scSettings.tRel_ms,
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
          .p_val  = &this->scSettings.knee_db,
          .name   = "Knee",
          .suffix = " dB",
          .minVal = 0.0,
          .maxVal = 24.0,
      },
      {
          .p_val  = &this->scSettings.tRms_ms,
          .name   = "RMS_time",
          .suffix = " ms",
          .minVal = 1.0,
          .maxVal = 80.0,
      },
      {
          .p_val  = &this->mix_percent,
          .name   = "Mix",
          .suffix = " %",
          .minVal = 0.0,
          .maxVal = 100.0,
      },
      {
          .p_val    = &this->scHpfSettings.fc_hz,
          .name     = "SC_HPF",
          .suffix   = " hz",
          .minVal   = 20.0,
          .maxVal   = 2000.0,
          .midPoint = 200.0,
      },
      {
          .p_val  = &this->scBoostSettings.gain_db,
          .name   = "SC_Boost",
          .suffix = " dB",
          .minVal = 0.0,
          .maxVal = 24.0,
      },
    };

    this->toggles = {
      { .p_val = &this->scSettings.rmsEnable, .name = "RMS" },
      { .p_val = &this->feedbackEnable, .name = "Feedback" },
      { .p_val = &this->linEnable, .name = "Linear" },
      { .p_val = &this->linkEnable, .name = "Link" },
      { .p_val = &this->scListenEnable, .name = "SC_Listen" },
      { .p_val = &this->bypassEnable, .name = "Bypass" },
    };

    this->guiSpec.meters = {
      { .name = "IN" },
      { .name = "OUT", .hasScale = true },
      { .name = "GR", .invert = true, .hasScale = true },
    };
    this->scHpfSettings.fc_hz   = 20;
    this->scBoostSettings.fc_hz = 3000.0;
    this->softClipCoeffs        = NtFx::calculateSoftClipCoeffs<signal_t, 2>();
    this->scBoostSettings.shape = NtFx::Biquad::Shape::bell;
    this->scHpfSettings.shape   = NtFx::Biquad::Shape::hpf;
    this->guiSpec.foregroundColour = 0xFF000000;
    this->guiSpec.backgroundColour = 0xFFFFFFFF;

    this->updateDefaults();
  }

  NtFx::Stereo<signal_t> processSample(
      NtFx::Stereo<signal_t> x) noexcept override {
    this->template updatePeakLevel<0>(x);
    if (this->bypassEnable) {
      this->template updatePeakLevel<1>(x);
      return x;
    }
    NtFx::ensureFinite(x);
    NtFx::ensureFinite(this->fbState);
    NtFx::Stereo<signal_t> x_hpf = x;
    if (this->feedbackEnable) { x_hpf = this->fbState; }

    NtFx::Stereo<signal_t> x_boost = NtFx::Biquad::biQuad5s(
        &this->scHpfCoeffs, &this->bqState[0], &this->bqState[1], x_hpf);
    NtFx::Stereo<signal_t> x_sc = NtFx::Biquad::biQuad5s(
        &this->scBoostCoeffs, &this->bqState[2], &this->bqState[3], x_boost);

    NtFx::Stereo<signal_t> gr;
    if (this->linEnable) {
      gr.l = NtFx::SideChain::sideChain_lin(
          &this->scCoeffs, &this->scState[0], x_sc.l);
      gr.r = NtFx::SideChain::sideChain_lin(
          &this->scCoeffs, &this->scState[1], x_sc.r);
    } else {
      gr.l = NtFx::SideChain::sideChain_db(
          &this->scCoeffs, &this->scState[0], x_sc.l);
      gr.r = NtFx::SideChain::sideChain_db(
          &this->scCoeffs, &this->scState[1], x_sc.r);
    }
    if (this->linkEnable) { gr = gr.absMin(); }
    this->template updatePeakLevel<2, true>(gr);
    ensureFinite(gr, static_cast<signal_t>(1.0));
    NtFx::Stereo<signal_t> yComp = x * gr;
    this->fbState                = yComp;
    auto ySoftClip               = NtFx::softClip5thStereo<signal_t>(
        this->softClipCoeffs, yComp * this->makeup_lin);
    auto y = this->mix_lin * ySoftClip + (1 - this->mix_lin) * x;
    this->template updatePeakLevel<1>(y);
    if (this->scListenEnable) { return x_sc; }
    return y;
  }

  void updateCoeffs() noexcept override {
    this->scHpfCoeffs =
        NtFx::Biquad::calcCoeffs5<signal_t>(this->scHpfSettings, this->fs);
    this->scBoostCoeffs =
        NtFx::Biquad::calcCoeffs5<signal_t>(this->scBoostSettings, this->fs);
    this->scCoeffs   = NtFx::SideChain::calcCoeffs(this->fs, &this->scSettings);
    this->makeup_lin = NtFx::invDb(this->makeup_db);
    this->mix_lin    = this->mix_percent / 100.0;
  }

  void reset(int fs) noexcept override {
    this->fs = fs;
    std::fill(this->peakLevels.begin(),
        this->peakLevels.end(),
        static_cast<signal_t>(0));
    this->peakLevels[2] = static_cast<signal_t>(1);
    this->fbState       = static_cast<signal_t>(0);
    this->scState[0].reset();
    this->scState[1].reset();
    this->updateCoeffs();
  }
};
