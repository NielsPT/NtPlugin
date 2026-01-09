#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <string>
#include <vector>

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

// constexpr int rmsDelayLineLength     = 16384;
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

  signal_t thresh_db   = 0;
  signal_t ratio       = 2;
  signal_t knee_db     = 0;
  signal_t tAtt_ms     = 10;
  signal_t tRel_ms     = 100;
  signal_t tRms_ms     = 80;
  signal_t makeup_db   = 0;
  signal_t mix_percent = SIGNAL(100.0);

  bool bypassEnable   = false;
  bool linEnable      = false;
  bool rmsEnable      = false;
  bool feedbackEnable = false;

  // signal_t tPeak      = SIGNAL(20.0);
  // signal_t alphaAtt   = SIGNAL(0.0);
  // signal_t alphaRel   = SIGNAL(0.0);
  // signal_t alphaPeak  = SIGNAL(0.0);
  // signal_t thresh_lin = SIGNAL(1.0);
  signal_t makeup_lin = SIGNAL(1.0);
  // signal_t knee_lin   = SIGNAL(1.0);
  // signal_t ratio_lin  = SIGNAL(1.0);
  signal_t mix_lin = SIGNAL(1.0);
  // signal_t rmsAccum = SIGNAL(0.0);

  signal_t fbState = SIGNAL(0.0);
  signal_t grState = SIGNAL(0.0);

  std::array<Stereo<signal_t>, 3> peakLevels;
  // T peakIn  = SIGNAL(0.0);
  // T peakGr  = SIGNAL(0.0);
  // T peakOut = SIGNAL(0.0);

  int nRms = 441;
  int iRms = 0;
  int fs   = 44100;

  // std::array<signal_t, 2> scState;
  std::array<signal_t, 3> softClipCoeffs;
  // std::array<signal_t, rmsDelayLineLength> rmsDelayLine;
  SideChain_db<signal_t> sideChainL_db;
  SideChain_db<signal_t> sideChainR_db;
  SideChain_lin<signal_t> sideChainL_lin;
  SideChain_lin<signal_t> sideChainR_lin;
  std::vector<SideChain<signal_t>*> allSideChains;

  std::vector<FloatParameterSpec<signal_t>> floatParameters {
    {
        .p_val      = &this->thresh_db,
        .name       = "Threshold",
        .suffix     = " dB",
        .minVal     = -60.0,
        .maxVal     = 0.0,
        .defaultVal = 0.0,
    },
    {
        .p_val      = &this->ratio,
        .name       = "Ratio",
        .suffix     = "",
        .minVal     = 1.0,
        .maxVal     = 20.0,
        .defaultVal = 2.0,
        .skew       = 2.0,
    },

    {
        .p_val      = &this->tAtt_ms,
        .name       = "Attack",
        .suffix     = " ms",
        .minVal     = 0.01,
        .maxVal     = 50.0,
        .defaultVal = 10.0,
    },
    {
        .p_val      = &this->tRel_ms,
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
        .p_val      = &this->knee_db,
        .name       = "Knee",
        .suffix     = " dB",
        .minVal     = 0.0,
        .maxVal     = 24.0,
        .defaultVal = 0.0,
    },
    {
        .p_val      = &this->tRms_ms,
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
        .p_val = &this->rmsEnable,
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

  CompressorPlugin() {
    this->allSideChains.push_back(&this->sideChainL_db);
    this->allSideChains.push_back(&this->sideChainR_db);
    this->allSideChains.push_back(&this->sideChainL_lin);
    this->allSideChains.push_back(&this->sideChainR_lin);
    this->softClipCoeffs = calculateSoftClipCoeffs<2>();
  }

  NTFX_INLINE_MEMBER Stereo<signal_t> processSample(Stereo<signal_t> x) noexcept {
    this->updatePeakLevel(x, MeterIdx::in);
    if (this->bypassEnable) {
      this->updatePeakLevel(x, MeterIdx::out);
      return x;
    }
    // TODO: isfinite in Stereo
    checkNotFinite(x.l, ErrorVal::e_x);
    checkNotFinite(x.r, ErrorVal::e_x);
    checkNotFinite(this->fbState, ErrorVal::e_fbState);
    signal_t x_sc = this->fbState;
    if (!this->feedbackEnable) { x_sc = x.absMax(); }

    signal_t gr;
    if (this->linEnable) {
      // gr = linSideChain(x_sc);
      gr = sideChainL_lin.processSample(x_sc);
    } else {
      // gr = dbSideChain(x_sc);
      gr = sideChainL_db.processSample(x_sc);
    }
    this->grState = gr;
    // TODO: separate gr for left and right/link option
    // TODO: Method for this meter thing. Takes a Stereo?
    // TODO: separate gr meter?
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

  // NTFX_INLINE_MEMBER signal_t linSideChain(signal_t x) noexcept {
  //   signal_t ySensLast   = this->scState[0];
  //   signal_t yFilterLast = this->scState[1];
  //   checkNotFinite(ySensLast, ErrorVal::e_ySensLast);
  //   checkNotFinite(yFilterLast, ErrorVal::e_yFilterLast);

  //   signal_t xAbs;
  //   if (this->rmsEnable) {
  //     xAbs = rmsSensor(x);
  //   } else {
  //     xAbs = std::abs(x);
  //   }

  //   signal_t sensRelease = this->alphaPeak * ySensLast + (1 - this->alphaPeak) *
  //   xAbs; signal_t ySens       = std::max(xAbs, sensRelease); ySensLast            =
  //   ySens;

  //   signal_t target;
  //   if (ySens < this->thresh_lin / this->knee_lin) {
  //     target = SIGNAL(0);
  //   } else if (ySens < this->thresh_lin) {
  //     target = (ySens / this->thresh_lin)
  //         * this->ratio_lin
  //         * this->thresh_lin
  //         / (this->knee_lin * ySens);
  //   } else {
  //     target = (ySens / this->thresh_lin) * this->ratio_lin;
  //   }

  //   signal_t alpha = this->alphaRel;
  //   if (target > yFilterLast) { alpha = this->alphaAtt; }

  //   signal_t yFilter = yFilterLast * alpha + target * (1 - alpha);
  //   yFilterLast      = yFilter;
  //   this->scState[0] = ySensLast;
  //   this->scState[1] = yFilterLast;
  //   return SIGNAL(1.0) / (yFilter + 1);
  // }

  // NTFX_INLINE_MEMBER signal_t dbSideChain(signal_t x) noexcept {
  //   signal_t ySensLast   = this->scState[0];
  //   signal_t yFilterLast = this->scState[1];
  //   checkNotFinite(ySensLast, ErrorVal::e_ySensLast);
  //   checkNotFinite(yFilterLast, ErrorVal::e_yFilterLast);

  //   signal_t xAbs;
  //   if (this->rmsEnable) {
  //     xAbs = rmsSensor(x);
  //   } else {
  //     xAbs = std::abs(x);
  //   }

  //   signal_t sensRelease = this->alphaPeak * ySensLast + (1 - this->alphaPeak) *
  //   xAbs; signal_t ySens       = std::max(xAbs, sensRelease); ySensLast            =
  //   ySens;

  //   signal_t x_db = db(ySens);
  //   signal_t y_db;
  //   if ((x_db - this->thresh_db) > (this->knee_db / 2)) {
  //     y_db = this->thresh_db + (x_db - this->thresh_db) / this->ratio;
  //   } else if ((x_db - this->thresh_db) < -(this->knee_db / 2)) {
  //     y_db = x_db;
  //   } else {
  //     signal_t tmp = (x_db - this->thresh_db + this->knee_db / 2);
  //     y_db         = x_db + (1 / this->ratio - 1) * tmp * tmp / (2 * this->knee_db);
  //   }

  //   signal_t target = x_db - y_db;

  //   signal_t alpha = this->alphaRel;
  //   if (target > yFilterLast) { alpha = this->alphaAtt; }

  //   signal_t yFilter = yFilterLast * alpha + target * (1 - alpha);
  //   yFilterLast      = yFilter;
  //   this->scState[0] = ySensLast;
  //   this->scState[1] = yFilterLast;
  //   return invDb(-yFilter);
  // }

  void update() noexcept {
    // this->alphaAtt = std::exp(-2200.0 / (this->tAtt_ms * this->fs));
    // this->alphaRel = std::exp(-2200.0 / (this->tRel_ms * this->fs));
    // if (this->alphaRel < this->alphaAtt) { this->alphaRel = this->alphaAtt; }
    // this->alphaPeak     = std::exp(-2200.0 / (this->tPeak * this->fs));
    // this->thresh_lin    = std::pow(10.0, (this->thresh_db / 20.0));
    this->makeup_lin = std::pow(10.0, (this->makeup_db / SIGNAL(20.0)));
    // this->knee_lin      = std::pow(10.0, (this->knee_db / 20.0));
    // double oneOverSqrt2 = 1.0 / std::sqrt(2.0);
    // this->ratio_lin     = (1.0 - 1.0 / this->ratio)
    // * (oneOverSqrt2 - std::pow(oneOverSqrt2 - (this->ratio - 3.0) / 18.0, 5.0));
    // this->nRms    = std::floor(this->tRms_ms * this->fs * 0.001);
    this->mix_lin = this->mix_percent / 100.0;
    // this->sideChainL_db.thresh_db  = this->thresh_db;
    // this->sideChainL_db.ratio      = this->ratio;
    // this->sideChainL_db.knee_db    = this->knee_db;
    // this->sideChainL_db.tAtt_ms    = this->tAtt_ms;
    // this->sideChainL_db.tRel_ms    = this->tRel_ms;
    // this->sideChainL_db.tRms_ms    = this->tRms_ms;
    // this->sideChainR_db.thresh_db  = this->thresh_db;
    // this->sideChainR_db.ratio      = this->ratio;
    // this->sideChainR_db.knee_db    = this->knee_db;
    // this->sideChainR_db.tAtt_ms    = this->tAtt_ms;
    // this->sideChainR_db.tRel_ms    = this->tRel_ms;
    // this->sideChainR_db.tRms_ms    = this->tRms_ms;
    // this->sideChainL_lin.thresh_db = this->thresh_db;
    // this->sideChainL_lin.ratio     = this->ratio;
    // this->sideChainL_lin.knee_db   = this->knee_db;
    // this->sideChainL_lin.tAtt_ms   = this->tAtt_ms;
    // this->sideChainL_lin.tRel_ms   = this->tRel_ms;
    // this->sideChainL_lin.tRms_ms   = this->tRms_ms;
    // this->sideChainR_lin.thresh_db = this->thresh_db;
    // this->sideChainR_lin.ratio     = this->ratio;
    // this->sideChainR_lin.knee_db   = this->knee_db;
    // this->sideChainR_lin.tAtt_ms   = this->tAtt_ms;
    // this->sideChainR_lin.tRel_ms   = this->tRel_ms;
    // this->sideChainR_lin.tRms_ms   = this->tRms_ms;

    for (size_t i = 0; i < this->allSideChains.size(); i++) {
      // TODO: SideChainSettings struct
      allSideChains[i]->thresh_db = this->thresh_db;
      allSideChains[i]->ratio     = this->ratio;
      allSideChains[i]->knee_db   = this->knee_db;
      allSideChains[i]->tAtt_ms   = this->tAtt_ms;
      allSideChains[i]->tRel_ms   = this->tRel_ms;
      allSideChains[i]->tRms_ms   = this->tRms_ms;
      allSideChains[i]->update();
    }
  }

  NTFX_INLINE_MEMBER void reset() noexcept {
    // std::fill(this->scState.begin(), this->scState.end(), SIGNAL(0));
    // std::fill(this->rmsDelayLine.begin(), this->rmsDelayLine.end(), SIGNAL(0));
    std::fill(this->peakLevels.begin(), this->peakLevels.end(), SIGNAL(0));
    // this->rmsAccum = SIGNAL(0);
    this->fbState = SIGNAL(0);
    // this->peakIn   = SIGNAL(0);
    this->peakLevels[MeterIdx::gr] = SIGNAL(1);
    // this->peakOut  = SIGNAL(0);
  }

  // NTFX_INLINE_MEMBER signal_t rmsSensor(signal_t x) noexcept {
  //   signal_t _x = x * x;
  //   this->rmsAccum += _x - this->rmsDelayLine[this->iRms];
  //   this->rmsDelayLine[this->iRms] = _x;
  //   this->iRms++;
  //   if (this->iRms >= this->nRms) { this->iRms = 0; }
  //   if (checkNotFiniteEnabled && this->rmsAccum < 0) {
  //     this->errorVal = ErrorVal::e_rmsAccum;
  //     return 0;
  //   }
  //   signal_t y = std::sqrt(2.0 * this->rmsAccum / this->nRms);
  //   checkNotFinite(y, ErrorVal::e_rmsSensor);
  //   return y;
  // }

  NTFX_INLINE_MEMBER Stereo<signal_t> softClip5thStereo(Stereo<signal_t> x) noexcept {
    return { softClip5thMono(x.l), softClip5thMono(x.r) };
  }

  NTFX_INLINE_MEMBER signal_t softClip5thMono(signal_t x) noexcept {
    signal_t x_ = x / this->softClipCoeffs[0];
    if (x_ > 1.0) { return SIGNAL(1.0); }
    if (x_ < -1.0) { return SIGNAL(-1.0); }
    return this->softClipCoeffs[0] * x_
        + this->softClipCoeffs[1] * x_ * x_ * x_
        + this->softClipCoeffs[2] * x_ * x_ * x_ * x_ * x_;
  }

  NTFX_INLINE_STATIC signal_t invDb(signal_t x) noexcept {
    // if (optimizeDb) { return pow10Opt(x * SIGNAL(0.05)); }
    return std::pow(SIGNAL(10.0), x * SIGNAL(0.05));
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
      signal_t& p_val, ErrorVal var, signal_t def = SIGNAL(0)) noexcept {
    if (!checkNotFiniteEnabled) { return true; }
    if (p_val == p_val) { return true; } // Float hack.
    p_val = def;
    // reset();
    this->errorVal = var;
    return false;
  }

  NTFX_INLINE_MEMBER bool checkNotFinite(
      Stereo<signal_t>& p_val, ErrorVal var, signal_t def = SIGNAL(0)) noexcept {
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
    this->peakLevels[idx] = SIGNAL(def);
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

  // T getAndResetPeakIn() noexcept {
  //   T tmp = this->peakIn;
  //   checkNotFinite(tmp, e_meterIn);
  //   this->peakIn = SIGNAL(0);
  //   return tmp;
  // }

  // T getAndResetPeakGr() noexcept {
  //   T tmp = this->peakGr;
  //   checkNotFinite(tmp, e_meterGr);
  //   this->peakGr = SIGNAL(0);
  //   return tmp;
  // }

  // T getAndResetPeakOut() noexcept {
  //   T tmp = this->peakOut;
  //   checkNotFinite(tmp, e_meterOut);
  //   this->peakOut = SIGNAL(0);
  //   return tmp;
  // }

  // NTFX_INLINE_STATIC T log10Opt(T x) noexcept {
  //   if (x < SIGNAL(1e-4)) { return -4; }
  //   if (x < SIGNAL(1e-3)) { return log10Taylor(x * SIGNAL(1e3)); }
  //   if (x < SIGNAL(1e-2)) { return log10Taylor(x * SIGNAL(1e2)); }
  //   if (x < SIGNAL(1e-1)) { return log10Taylor(x * SIGNAL(1e1)); }
  //   return log10Taylor(x);
  // }

  // NTFX_INLINE_STATIC T log10Taylor(T x) noexcept {
  //   if (x < 0.2) {
  //     T n = SIGNAL(0.325443314844276);
  //     return x * 10 * n - 1 - n;
  //   }
  //   T x_    = x - 1;
  //   T mult  = x_ * x_;
  //   T accum = x_ - mult * SIGNAL(1 / 2);
  //   mult *= x_;
  //   accum += mult * SIGNAL(1 / 3);
  //   mult *= x_;
  //   accum -= mult * SIGNAL(1 / 4);
  //   mult *= x_;
  //   accum += mult * SIGNAL(1 / 5);
  //   mult *= x_;
  //   accum -= mult * SIGNAL(1 / 6);
  //   mult *= x_;
  //   accum += mult * SIGNAL(1 / 7);
  //   mult *= x_;
  //   accum -= mult * SIGNAL(1 / 8);
  //   return accum * SIGNAL(0.434294481903252);
  // }

  // NTFX_INLINE_STATIC T pow10Opt(T x) noexcept {
  //   if (x < -4) { return 0; }
  //   if (x < -3) { return pow10Taylor(x + 3) * SIGNAL(0.001); }
  //   if (x < -2) { return pow10Taylor(x + 2) * SIGNAL(0.01); }
  //   if (x < -1) { return pow10Taylor(x + 1) * SIGNAL(0.1); }
  //   return pow10Taylor(x);
  // }

  // NTFX_INLINE_STATIC T pow10Taylor(T x) noexcept {
  //   T x_    = -x * SIGNAL(2.302585092994046);
  //   T mult  = x_ * x_;
  //   T accum = mult * SIGNAL(0.5);
  //   mult *= x_;
  //   accum -= mult * SIGNAL(0.166666666666667);
  //   mult *= x_;
  //   accum += mult * SIGNAL(0.041666666666667);
  //   mult *= x_;
  //   accum -= mult * SIGNAL(0.008333333333333);
  //   mult *= x_;
  //   accum += mult * SIGNAL(0.001388888888889);
  //   mult *= x_;
  //   accum -= mult * SIGNAL(1.984126984126984e-04);
  //   mult *= x_;
  //   accum += mult * SIGNAL(2.480158730158730e-05);
  //   mult *= x_;
  //   accum -= mult * SIGNAL(2.755731922398589e-06);
  //   mult *= x_;
  //   accum += mult * SIGNAL(2.755731922398589e-07);
  //   mult *= x_;
  //   accum -= mult * SIGNAL(2.505210838544172e-08);
  //   mult *= x_;
  //   accum += mult * SIGNAL(2.087675698786810e-09);
  //   return 1 - x_ * accum;
  // }
};
}