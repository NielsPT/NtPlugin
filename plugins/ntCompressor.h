#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <string>
#include <vector>

#include "lib/Plugin.h"
#include "lib/SideChain.h"
#include "lib/SoftClip.h"
#include "lib/Stereo.h"
#include "lib/utils.h"

template <typename signal_t>
struct ntCompressor : public NtFx::Plugin<signal_t> {
  int errorVarId = 0;

  NtFx::ScSettings<signal_t> scSettings;
  NtFx::ScCoeffs<signal_t> scCoeffs;
  std::array<NtFx::ScState<signal_t>, 2> scState;
  signal_t makeup_db   = 0;
  signal_t mix_percent = NTFX_SIG_T(100.0);

  bool bypassEnable   = false;
  bool linEnable      = false;
  bool feedbackEnable = false;
  bool linkEnable     = false;

  signal_t mix_lin               = NTFX_SIG_T(1.0);
  signal_t makeup_lin            = NTFX_SIG_T(1.0);
  NtFx::Stereo<signal_t> fbState = NTFX_SIG_T(0.0);
  std::array<signal_t, 3> softClipCoeffs;

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
          .p_val  = &this->scSettings.ratio_db,
          .name   = "Ratio",
          .suffix = "",
          .minVal = 1.0,
          .maxVal = 20.0,
          .skew   = 2.0,
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
    };

    this->toggles = {
      { .p_val = &this->scSettings.rmsEnable, .name = "RMS" },
      { .p_val = &this->feedbackEnable, .name = "Feedback" },
      { .p_val = &this->linEnable, .name = "Linear" },
      { .p_val = &this->linkEnable, .name = "Link" },
      { .p_val = &this->bypassEnable, .name = "Bypass" },
    };
    this->updateDefaults();
    this->softClipCoeffs = NtFx::calculateSoftClipCoeffs<signal_t, 2>();
  }

  NTFX_INLINE_MEMBER NtFx::Stereo<signal_t> processSample(
      NtFx::Stereo<signal_t> x) noexcept override {
    this->updatePeakLevel(x, NtFx::MeterIdx::in);
    if (this->bypassEnable) {
      this->updatePeakLevel(x, NtFx::MeterIdx::out);
      return x;
    }
    NtFx::ensureFinite(x);
    NtFx::ensureFinite(this->fbState);
    NtFx::Stereo<signal_t> x_sc = x;
    if (this->feedbackEnable) { x_sc = this->fbState; }

    NtFx::Stereo<signal_t> gr;
    if (this->linEnable) {
      gr.l = sideChain_lin(&this->scCoeffs, &this->scState[0], x_sc.l);
      gr.r = sideChain_lin(&this->scCoeffs, &this->scState[1], x_sc.r);
    } else {
      gr.l = sideChain_db(&this->scCoeffs, &this->scState[0], x_sc.l);
      gr.r = sideChain_db(&this->scCoeffs, &this->scState[1], x_sc.r);
    }
    if (this->linkEnable) { gr = gr.absMin(); }
    this->updatePeakLevel(gr, NtFx::MeterIdx::gr, true);
    ensureFinite(gr, NTFX_SIG_T(1.0));
    NtFx::Stereo<signal_t> yComp = x * gr;
    this->fbState                = yComp;
    auto ySoftClip               = NtFx::softClip5thStereo<signal_t>(
        this->softClipCoeffs, yComp * this->makeup_lin);
    auto y = this->mix_lin * ySoftClip + (1 - this->mix_lin) * x;
    this->updatePeakLevel(y, NtFx::MeterIdx::out);
    return y;
  }

  void updateCoeffs() noexcept override {
    this->scCoeffs   = NtFx::calcSideChainCoeffs(this->fs, &this->scSettings);
    this->makeup_lin = std::pow(10.0, (this->makeup_db / NTFX_SIG_T(20.0)));
    this->mix_lin    = this->mix_percent / 100.0;
  }

  void reset(int fs) noexcept override {
    this->fs = fs;
    std::fill(this->peakLevels.begin(), this->peakLevels.end(), NTFX_SIG_T(0));
    this->peakLevels[NtFx::MeterIdx::gr] = NTFX_SIG_T(1);
    this->fbState                        = NTFX_SIG_T(0);
    this->scState[0].reset();
    this->scState[1].reset();
    this->updateCoeffs();
  }
};