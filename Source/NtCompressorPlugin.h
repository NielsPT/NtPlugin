#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <string>
#include <vector>

#include "NtFx.h"
#include "SideChain.h"
#include "Stereo.h"
namespace NtFx {

// TODO: general parameter class.
template <typename signal_t>
struct FloatParameterSpec {
  signal_t* p_val;
  std::string name;
  std::string suffix;
  signal_t minVal { 0.0 };
  signal_t maxVal { 1.0 };
  signal_t defaultVal { 0.5 };
  signal_t skew { 0.0 };
};

struct BoolParameterSpec {
  bool* p_val;
  std::string name;
  bool defaultVal { false };
};

constexpr int rmsDelayLineLength     = 16384;
constexpr bool optimizeDb            = false;
constexpr bool checkNotFiniteEnabled = true;
constexpr bool reportNotFiniteState  = false;

namespace MeterIdx {
  constexpr int in  = 0;
  constexpr int out = 1;
  constexpr int gr  = 2;

}

enum ErrorVal {
  e_none,
  e_x,
  e_x_sc,
  e_fbState,
  e_ySensLast,
  e_yFilterLast,
  e_rmsSensor,
  e_nRms,
  e_iRms,
  e_rmsAccum,
  e_gr,
  e_meter,
};

// TODO: NtPlug baseclass?
template <typename signal_t>
struct CompressorPlugin {

  ErrorVal errorVal;

  ScSettings<signal_t> scSettings;
  ScCoeffs<signal_t> scCoeffs;
  // TODO: separate state for left and right and link option.
  ScState<signal_t> scState;
  signal_t makeup_db   = 0;
  signal_t mix_percent = NTFX_SIGNAL(100.0);

  bool bypassEnable   = false;
  bool linEnable      = false;
  bool feedbackEnable = false;

  signal_t mix_lin    = NTFX_SIGNAL(1.0);
  signal_t makeup_lin = NTFX_SIGNAL(1.0);

  signal_t fbState = NTFX_SIGNAL(0.0);

  std::array<Stereo<signal_t>, 3> peakLevels;

  int nRms = 441;
  int iRms = 0;
  int fs   = 44100;

  std::array<signal_t, 3> softClipCoeffs;
  std::vector<FloatParameterSpec<signal_t>> floatParameters {
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

  std::vector<FloatParameterSpec<signal_t>> floatParametersSmall = {
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

  std::vector<BoolParameterSpec> boolParameters {
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
        .p_val = &this->bypassEnable,
        .name  = "Bypass",
    },
  };

  CompressorPlugin() { this->softClipCoeffs = calculateSoftClipCoeffs<2>(); }

  NTFX_INLINE_MEMBER Stereo<signal_t> processSample(Stereo<signal_t> x) noexcept {
    this->updatePeakLevel(x, MeterIdx::in);
    if (this->bypassEnable) {
      this->updatePeakLevel(x, MeterIdx::out);
      return x;
    }
    // TODO: isfinite in Stereo
    checkNotFinite(x.l, ErrorVal::e_x);
    checkNotFinite(x.r, ErrorVal::e_x);
    // TODO: Make fbstate atomic
    checkNotFinite(this->fbState, ErrorVal::e_fbState);
    signal_t x_sc = this->fbState;
    if (!this->feedbackEnable) { x_sc = x.absMax(); }

    signal_t gr;
    if (this->linEnable) {
      gr = sideChain_lin(&this->scCoeffs, &this->scState, x_sc);
    } else {
      gr = sideChain_db(&this->scCoeffs, &this->scState, x_sc);
    }
    this->updatePeakLevel(gr, MeterIdx::gr, true);
    checkNotFinite(gr, ErrorVal::e_gr, 1);
    Stereo<signal_t> yComp = x * gr;
    this->fbState          = yComp.absMax();
    auto ySoftClip         = softClip5thStereo(yComp * this->makeup_lin);
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

  void update() noexcept {
    this->scCoeffs   = calcSideChainCoeffs(this->fs, &this->scSettings);
    this->makeup_lin = std::pow(10.0, (this->makeup_db / NTFX_SIGNAL(20.0)));
    this->mix_lin    = this->mix_percent / 100.0;
  }

  NTFX_INLINE_MEMBER void reset(int fs) noexcept {
    this->fs = fs;
    std::fill(this->peakLevels.begin(), this->peakLevels.end(), NTFX_SIGNAL(0));
    this->peakLevels[MeterIdx::gr] = NTFX_SIGNAL(1);
    this->fbState                  = NTFX_SIGNAL(0);
    this->update();
  }

  NTFX_INLINE_MEMBER Stereo<signal_t> softClip5thStereo(Stereo<signal_t> x) noexcept {
    return { softClip5thMono(x.l), softClip5thMono(x.r) };
  }

  NTFX_INLINE_MEMBER signal_t softClip5thMono(signal_t x) noexcept {
    signal_t x_ = x / this->softClipCoeffs[0];
    if (x_ > 1.0) { return NTFX_SIGNAL(1.0); }
    if (x_ < -1.0) { return NTFX_SIGNAL(-1.0); }
    return this->softClipCoeffs[0] * x_
        + this->softClipCoeffs[1] * x_ * x_ * x_
        + this->softClipCoeffs[2] * x_ * x_ * x_ * x_ * x_;
  }

  NTFX_INLINE_STATIC signal_t invDb(signal_t x) noexcept {
    // if (optimizeDb) { return pow10Opt(x * NTFX_SIGNAL(0.05)); }
    return std::pow(NTFX_SIGNAL(10.0), x * NTFX_SIGNAL(0.05));
  }

  template <size_t N>
  NTFX_INLINE_STATIC std::array<signal_t, N + 1> calculateSoftClipCoeffs() noexcept {
    // order = 2 * N + 1
    std::array<signal_t, N + 1> a_n;
    for (int n = 0; n < N + 1; n++) {
      a_n[n] = (std::pow(-1, n)
          * std::tgamma((2 * N + 1) + 1)
          / (std::pow(4, N)
              * std::tgamma(N + 1)
              * (2 * n + 1)
              * std::tgamma(n + 1)
              * std::tgamma(N - n + 1)));
    }
    return a_n;
  }

  NTFX_INLINE_MEMBER bool checkNotFinite(
      signal_t& p_val, ErrorVal var, signal_t def = NTFX_SIGNAL(0)) noexcept {
    if (!checkNotFiniteEnabled) { return true; }
    if (p_val == p_val) { return true; }
    p_val          = def;
    this->errorVal = var;
    return false;
  }

  NTFX_INLINE_MEMBER bool checkNotFinite(
      Stereo<signal_t>& p_val, ErrorVal var, signal_t def = NTFX_SIGNAL(0)) noexcept {
    return this->checkNotFinite(p_val.l, var, def)
        && this->checkNotFinite(p_val.r, var, def);
  }

  ErrorVal getAndResetErrorVal() noexcept {
    auto tmp       = this->errorVal;
    this->errorVal = ErrorVal::e_none;
    return tmp;
  }

  Stereo<signal_t> getAndResetPeakLevel(size_t idx) noexcept {
    Stereo<signal_t> tmp = this->peakLevels[idx];
    int def              = static_cast<int>(idx == MeterIdx::gr);
    checkNotFinite(tmp, e_meter, def);
    this->peakLevels[idx] = NTFX_SIGNAL(def);
    return tmp;
  }

  bool* getBoolValByName(std::string name) {
    for (auto param : this->boolParameters) {
      if (param.name == name) { return param.p_val; }
    }
    return nullptr;
  }

  signal_t* getFloatValByName(std::string name) {
    for (auto param : this->floatParameters) {
      if (param.name == name) { return param.p_val; }
    }
    for (auto param : this->floatParametersSmall) {
      if (param.name == name) { return param.p_val; }
    }
    return nullptr;
  }
};
}