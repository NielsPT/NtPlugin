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

#define NTFX_PLUGIN NtFx::Compressor

namespace NtFx {

constexpr int rmsDelayLineLength     = 16384;
constexpr bool optimizeDb            = false;
constexpr bool checkNotFiniteEnabled = true;
constexpr bool reportNotFiniteState  = false;

namespace MeterIdx {
  constexpr int in  = 0;
  constexpr int out = 1;
  constexpr int gr  = 2;
}

template <typename signal_t>
struct Compressor : public Plugin<signal_t> {
  int errorVarId = 0;

  ScSettings<signal_t> scSettings;
  ScCoeffs<signal_t> scCoeffs;
  std::array<ScState<signal_t>, 2> scState;
  signal_t makeup_db   = 0;
  signal_t mix_percent = NTFX_SIGNAL(100.0);

  bool bypassEnable   = false;
  bool linEnable      = false;
  bool feedbackEnable = false;
  bool linkEnable     = true;

  signal_t mix_lin         = NTFX_SIGNAL(1.0);
  signal_t makeup_lin      = NTFX_SIGNAL(1.0);
  Stereo<signal_t> fbState = NTFX_SIGNAL(0.0);
  std::array<Stereo<signal_t>, 3> peakLevels;
  std::array<signal_t, 3> softClipCoeffs;

  Compressor() {
    this->primaryKnobs = {
      {
          .p_val      = &this->scSettings.thresh_db,
          .name       = "Threshold",
          .suffix     = " dB",
          .minVal     = -60.0,
          .maxVal     = 0.0,
          .defaultVal = 0.0,
      },
      {
          .p_val      = &this->scSettings.ratio_db,
          .name       = "Ratio",
          .suffix     = "",
          .minVal     = 1.0,
          .maxVal     = 20.0,
          .defaultVal = 2.0,
          .skew       = 2.0,
      },

      {
          .p_val      = &this->scSettings.tAtt_ms,
          .name       = "Attack",
          .suffix     = " ms",
          .minVal     = 0.01,
          .maxVal     = 50.0,
          .defaultVal = 10.0,
      },
      {
          .p_val      = &this->scSettings.tRel_ms,
          .name       = "Release",
          .suffix     = " ms",
          .minVal     = 10.0,
          .maxVal     = 1000.0,
          .defaultVal = 100.0,
      },

      {
          .p_val      = &this->makeup_db,
          .name       = "Makeup",
          .suffix     = " dB",
          .minVal     = 0.0,
          .maxVal     = 24.0,
          .defaultVal = 0.0,
      },
    };

    this->secondaryKnobs = {
      {
          .p_val      = &this->scSettings.knee_db,
          .name       = "Knee",
          .suffix     = " dB",
          .minVal     = 0.0,
          .maxVal     = 24.0,
          .defaultVal = 0.0,
      },
      {
          .p_val      = &this->scSettings.tRms_ms,
          .name       = "RMS_time",
          .suffix     = " ms",
          .minVal     = 1.0,
          .maxVal     = 80.0,
          .defaultVal = 20.0,
      },
      {
          .p_val      = &this->mix_percent,
          .name       = "Mix",
          .suffix     = " %",
          .minVal     = 0.0,
          .maxVal     = 100.0,
          .defaultVal = 100.0,
      },
    };

    this->toggles = {
      {
          .p_val = &this->scSettings.rmsEnable,
          .name  = "RMS",
      },
      {
          .p_val = &this->feedbackEnable,
          .name  = "Feedback",
      },
      {
          .p_val      = &this->linEnable,
          .name       = "Linear",
          .defaultVal = true,
      },
      {
          .p_val = &this->linkEnable,
          .name  = "Link",
      },
      {
          .p_val = &this->bypassEnable,
          .name  = "Bypass",
      },
    };

    this->softClipCoeffs = calculateSoftClipCoeffs<signal_t, 2>();
  }

  NTFX_INLINE_MEMBER Stereo<signal_t> processSample(
      Stereo<signal_t> x) noexcept override {
    this->updatePeakLevel(x, MeterIdx::in);
    if (this->bypassEnable) {
      this->updatePeakLevel(x, MeterIdx::out);
      return x;
    }
    // TODO: isfinite in Stereo
    checkNotFinite(x.l, 1);
    checkNotFinite(x.r, 2);
    // TODO: Make fbState atomic
    checkNotFinite(this->fbState, 3);
    Stereo<signal_t> x_sc = x;
    if (this->feedbackEnable) { x_sc = this->fbState; }

    Stereo<signal_t> gr;
    if (this->linEnable) {
      gr.l = sideChain_lin(&this->scCoeffs, &this->scState[0], x_sc.l);
      gr.r = sideChain_lin(&this->scCoeffs, &this->scState[1], x_sc.r);
    } else {
      gr.l = sideChain_db(&this->scCoeffs, &this->scState[0], x_sc.l);
      gr.r = sideChain_db(&this->scCoeffs, &this->scState[1], x_sc.r);
    }
    if (this->linkEnable) { gr = gr.absMin(); }
    this->updatePeakLevel(gr, MeterIdx::gr, true);
    checkNotFinite(gr, 4, 1);
    Stereo<signal_t> yComp = x * gr;
    this->fbState          = yComp;
    auto ySoftClip =
        softClip5thStereo<signal_t>(this->softClipCoeffs, yComp * this->makeup_lin);
    this->updatePeakLevel(ySoftClip, MeterIdx::out);
    auto y = this->mix_lin * ySoftClip + (1 - this->mix_lin) * x;
    return y;
  }

  NTFX_INLINE_MEMBER void updatePeakLevel(
      Stereo<signal_t> val, size_t idx, bool invert = false) noexcept {
    if ((!invert && val > this->peakLevels[idx])
        || (invert && val < this->peakLevels[idx])) {
      this->peakLevels[idx] = val;
    }
  }

  void updateCoeffs() noexcept override {
    this->scCoeffs   = calcSideChainCoeffs(this->fs, &this->scSettings);
    this->makeup_lin = std::pow(10.0, (this->makeup_db / NTFX_SIGNAL(20.0)));
    this->mix_lin    = this->mix_percent / 100.0;
  }

  NTFX_INLINE_MEMBER void reset(int fs) noexcept override {
    this->fs = fs;
    std::fill(this->peakLevels.begin(), this->peakLevels.end(), NTFX_SIGNAL(0));
    this->peakLevels[MeterIdx::gr] = NTFX_SIGNAL(1);
    this->fbState                  = NTFX_SIGNAL(0);
    this->scState[0].reset();
    this->scState[1].reset();
    this->updateCoeffs();
  }

  NTFX_INLINE_MEMBER bool checkNotFinite(
      signal_t& p_val, int varId, signal_t def = NTFX_SIGNAL(0)) noexcept {
    if (!checkNotFiniteEnabled) { return true; }
    if (p_val == p_val) { return true; }
    p_val            = def;
    this->errorVarId = varId;
    return false;
  }

  NTFX_INLINE_MEMBER bool checkNotFinite(
      Stereo<signal_t>& p_val, int varId, signal_t def = NTFX_SIGNAL(0)) noexcept {
    return this->checkNotFinite(p_val.l, varId, def)
        && this->checkNotFinite(p_val.r, varId, def);
  }

  NTFX_INLINE_MEMBER int getAndResetErrorVal() noexcept {
    auto tmp         = this->errorVarId;
    this->errorVarId = 0;
    return tmp;
  }

  NTFX_INLINE_MEMBER Stereo<signal_t> getAndResetPeakLevel(size_t idx) noexcept {
    Stereo<signal_t> tmp = this->peakLevels[idx];
    signal_t def         = NTFX_SIGNAL(0);
    if (idx == MeterIdx::gr) { def = NTFX_SIGNAL(1); }
    checkNotFinite(tmp, 6, def);
    this->peakLevels[idx] = NTFX_SIGNAL(def);
    return tmp;
  }
};
}