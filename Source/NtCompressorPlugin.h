#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <string>
#include <vector>

#include "Stereo.h"

#define _T(v) static_cast<T>(v)
namespace NtFx {
// TODO: general parameter class.
template <typename T>
struct FloatParameterSpec {
  T* p_val;
  std::string name;
  std::string suffix;
  T minVal { 0.0 };
  T maxVal { 1.0 };
  T defaultVal { 0.5 };
  T skew { 0.0 };
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
  constexpr int xL  = 0;
  constexpr int xR  = 1;
  constexpr int yL  = 2;
  constexpr int yR  = 3;
  constexpr int grL = 4;
  constexpr int grR = 5;

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
template <typename T>
struct CompressorPlugin {

  ErrorVal errorVal;

  T thresh_db   = 0;
  T ratio       = 2;
  T knee_db     = 0;
  T tAtt_ms     = 10;
  T tRel_ms     = 100;
  T tRms_ms     = 80;
  T makeup_db   = 0;
  T mix_percent = _T(100.0);

  bool bypassEnable   = false;
  bool linEnable      = false;
  bool rmsEnable      = false;
  bool feedbackEnable = false;

  T tPeak      = _T(20.0);
  T alphaAtt   = _T(0.0);
  T alphaRel   = _T(0.0);
  T alphaPeak  = _T(0.0);
  T thresh_lin = _T(1.0);
  T makeup_lin = _T(1.0);
  T knee_lin   = _T(1.0);
  T ratio_lin  = _T(1.0);
  T mix_lin    = _T(1.0);

  T fbState  = _T(0.0);
  T grState  = _T(0.0);
  T rmsAccum = _T(0.0);

  std::array<T, 6> peakLevels;
  // T peakIn  = _T(0.0);
  // T peakGr  = _T(0.0);
  // T peakOut = _T(0.0);

  int nRms = 441;
  int iRms = 0;
  int fs   = 44100;

  std::array<T, 2> scState;
  std::array<T, 3> softClipCoeffs;
  std::array<T, rmsDelayLineLength> rmsDelayLine;

  std::vector<FloatParameterSpec<T>> floatParameters {
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
        .p_val      = &this->knee_db,
        .name       = "Knee",
        .suffix     = " dB",
        .minVal     = 0.0,
        .maxVal     = 24.0,
        .defaultVal = 0.0,
    },
    {
        .p_val      = &this->makeup_db,
        .name       = "Makeup",
        .suffix     = " dB",
        .minVal     = 0.0,
        .maxVal     = 24.0,
        .defaultVal = 0.0,
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

  std::vector<FloatParameterSpec<T>> floatParametersSmall = {
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

  // CompressorPlugin() { this->reset(); }

  NTFX_INLINE_MEMBER Stereo<T> processSample(Stereo<T> x) noexcept {
    if (x.l > this->peakLevels[MeterIdx::xL]) { this->peakLevels[MeterIdx::xL] = x.l; }
    if (x.r > this->peakLevels[MeterIdx::xR]) { this->peakLevels[MeterIdx::xR] = x.r; }
    if (this->bypassEnable) { return x; }
    // TODO: isfinite in Stereo
    checkNotFinite(x.l, ErrorVal::e_x);
    checkNotFinite(x.r, ErrorVal::e_x);
    checkNotFinite(this->fbState, ErrorVal::e_fbState);
    T x_sc = this->fbState;
    if (!this->feedbackEnable) { x_sc = x.absMax(); }

    T gr;
    if (this->linEnable) {
      gr = linSideChain(x_sc);
    } else {
      gr = dbSideChain(x_sc);
    }
    this->grState = gr;
    // TODO: separate gr for left and right/link option
    // TODO: Method for thus meter thing. Takes a Stereo?
    // TODO: separate gr meter?
    if (gr < this->peakLevels[MeterIdx::grL]) { this->peakLevels[MeterIdx::grL] = gr; }
    if (gr < this->peakLevels[MeterIdx::grR]) { this->peakLevels[MeterIdx::grR] = gr; }
    checkNotFinite(gr, ErrorVal::e_gr, 1);
    Stereo<T> yComp = x * gr;
    this->fbState   = yComp.absMax();
    auto ySoftClip  = softClip5thStereo(yComp * this->makeup_lin);
    auto y          = this->mix_lin * ySoftClip + (1 - this->mix_lin) * x;
    if (y.l > this->peakLevels[MeterIdx::yL]) { this->peakLevels[MeterIdx::yL] = y.l; }
    if (y.r > this->peakLevels[MeterIdx::yR]) { this->peakLevels[MeterIdx::yR] = y.r; }
    return y;
  }

  NTFX_INLINE_MEMBER T linSideChain(T x) noexcept {
    T ySensLast   = this->scState[0];
    T yFilterLast = this->scState[1];
    checkNotFinite(ySensLast, ErrorVal::e_ySensLast);
    checkNotFinite(yFilterLast, ErrorVal::e_yFilterLast);

    T xAbs;
    if (this->rmsEnable) {
      xAbs = rmsSensor(x);
    } else {
      xAbs = std::abs(x);
    }

    T sensRelease = this->alphaPeak * ySensLast + (1 - this->alphaPeak) * xAbs;
    T ySens       = std::max(xAbs, sensRelease);
    ySensLast     = ySens;

    T target = 0;
    if (ySens < this->thresh_lin / this->knee_lin) {
      target = 0;
    } else if (ySens < this->thresh_lin) {
      target = (ySens / this->thresh_lin)
          * this->ratio_lin
          * this->thresh_lin
          / (this->knee_lin * ySens);
    } else {
      target = (ySens / this->thresh_lin) * this->ratio_lin;
    }

    T alpha = this->alphaRel;
    if (target > yFilterLast) { alpha = this->alphaAtt; }

    T yFilter        = yFilterLast * alpha + target * (1 - alpha);
    yFilterLast      = yFilter;
    this->scState[0] = ySensLast;
    this->scState[1] = yFilterLast;
    return 1.0 / (yFilter + 1);
  }

  NTFX_INLINE_MEMBER T dbSideChain(T x) noexcept {
    T ySensLast   = this->scState[0];
    T yFilterLast = this->scState[1];
    checkNotFinite(ySensLast, ErrorVal::e_ySensLast);
    checkNotFinite(yFilterLast, ErrorVal::e_yFilterLast);

    T xAbs;
    if (this->rmsEnable) {
      xAbs = rmsSensor(x);
    } else {
      xAbs = std::abs(x);
    }

    T sensRelease = this->alphaPeak * ySensLast + (1 - this->alphaPeak) * xAbs;
    T ySens       = std::max(xAbs, sensRelease);
    ySensLast     = ySens;

    T x_db = db(ySens);
    T y_db;
    if ((x_db - this->thresh_db) > (this->knee_db / 2)) {
      y_db = this->thresh_db + (x_db - this->thresh_db) / this->ratio;
    } else if ((x_db - this->thresh_db) < -(this->knee_db / 2)) {
      y_db = x_db;
    } else {
      T tmp = (x_db - this->thresh_db + this->knee_db / 2);
      y_db  = x_db + (1 / this->ratio - 1) * tmp * tmp / (2 * this->knee_db);
    }

    T target = x_db - y_db;

    T alpha = this->alphaRel;
    if (target > yFilterLast) { alpha = this->alphaAtt; }

    T yFilter        = yFilterLast * alpha + target * (1 - alpha);
    yFilterLast      = yFilter;
    this->scState[0] = ySensLast;
    this->scState[1] = yFilterLast;
    return invDb(-yFilter);
  }

  void update() noexcept {
    this->alphaAtt = std::exp(-2200.0 / (this->tAtt_ms * this->fs));
    this->alphaRel = std::exp(-2200.0 / (this->tRel_ms * this->fs));
    if (this->alphaRel < this->alphaAtt) { this->alphaRel = this->alphaAtt; }
    this->alphaPeak     = std::exp(-2200.0 / (this->tPeak * this->fs));
    this->thresh_lin    = std::pow(10.0, (this->thresh_db / 20.0));
    this->makeup_lin    = std::pow(10.0, (this->makeup_db / 20.0));
    this->knee_lin      = std::pow(10.0, (this->knee_db / 20.0));
    double oneOverSqrt2 = 1.0 / std::sqrt(2.0);
    this->ratio_lin     = (1.0 - 1.0 / this->ratio)
        * (oneOverSqrt2 - std::pow(oneOverSqrt2 - (this->ratio - 3.0) / 18.0, 5.0));
    this->nRms           = std::floor(this->tRms_ms * this->fs * 0.001);
    this->softClipCoeffs = calculateSoftClipCoeffs<2>();
    this->mix_lin        = this->mix_percent / 100.0;
  }

  NTFX_INLINE_MEMBER void reset() noexcept {
    std::fill(this->scState.begin(), this->scState.end(), _T(0));
    std::fill(this->rmsDelayLine.begin(), this->rmsDelayLine.end(), _T(0));
    std::fill(this->peakLevels.begin(), this->peakLevels.end(), _T(0));
    this->rmsAccum = _T(0);
    this->fbState  = _T(0);
    // this->peakIn   = _T(0);
    this->peakLevels[1] = _T(1);
    // this->peakOut  = _T(0);
  }

  NTFX_INLINE_MEMBER T rmsSensor(T x) noexcept {
    if (checkNotFiniteEnabled && this->nRms < 1) {
      this->errorVal = ErrorVal::e_nRms;
      return 0;
    }
    if (checkNotFiniteEnabled && this->iRms < 0) {
      this->errorVal = ErrorVal::e_iRms;
      return 0;
    }

    T _x = x * x;
    this->rmsAccum += _x - this->rmsDelayLine[this->iRms];
    this->rmsDelayLine[this->iRms] = _x;
    this->iRms++;
    if (this->iRms >= this->nRms) { this->iRms = 0; }
    if (checkNotFiniteEnabled && this->rmsAccum < 0) {
      this->errorVal = ErrorVal::e_rmsAccum;
      return 0;
    }
    T y = std::sqrt(2.0 * this->rmsAccum / this->nRms);
    checkNotFinite(y, ErrorVal::e_rmsSensor);
    return y;
  }

  NTFX_INLINE_MEMBER Stereo<T> softClip5thStereo(Stereo<T> x) noexcept {
    return { softClip5thMono(x.l), softClip5thMono(x.r) };
  }

  NTFX_INLINE_MEMBER T softClip5thMono(T x) noexcept {
    T x_ = x / this->softClipCoeffs[0];
    if (x_ > 1.0) { return _T(1.0); }
    if (x_ < -1.0) { return _T(-1.0); }
    return this->softClipCoeffs[0] * x_
        + this->softClipCoeffs[1] * x_ * x_ * x_
        + this->softClipCoeffs[2] * x_ * x_ * x_ * x_ * x_;
  }

  NTFX_INLINE_STATIC T db(T x) noexcept {
    // if (optimizeDb) { return _T(20.0) * log10Opt(x); }
    return _T(20.0) * std::log10(x);
  }

  NTFX_INLINE_STATIC T invDb(T x) noexcept {
    // if (optimizeDb) { return pow10Opt(x * _T(0.05)); }
    return std::pow(_T(10.0), x * _T(0.05));
  }

  template <size_t N>
  NTFX_INLINE_STATIC std::array<T, N + 1> calculateSoftClipCoeffs() noexcept {
    // order = 2 * N + 1
    std::array<T, N + 1> a_n;
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
      T& p_val, ErrorVal var, T def = _T(0)) noexcept {
    if (!checkNotFiniteEnabled) { return true; }
    if (p_val == p_val) { return true; } // Float hack.
    p_val = def;
    // reset();
    this->errorVal = var;
    return false;
  }

  ErrorVal getAndResetErrorVal() noexcept {
    auto tmp       = this->errorVal;
    this->errorVal = ErrorVal::e_none;
    return tmp;
  }

  T getAndResetPeakLevel(size_t idx) noexcept {
    T tmp   = this->peakLevels[idx];
    int def = static_cast<int>(idx == MeterIdx::grL || idx == MeterIdx::grR);
    checkNotFinite(tmp, e_meter, def);
    this->peakLevels[idx] = _T(def);
    return tmp;
  }

  bool* getBoolValByName(std::string name) {
    for (auto param : this->boolParameters) {
      if (param.name == name) { return param.p_val; }
    }
    return nullptr;
  }

  T* getFloatValByName(std::string name) {
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
  //   this->peakIn = _T(0);
  //   return tmp;
  // }

  // T getAndResetPeakGr() noexcept {
  //   T tmp = this->peakGr;
  //   checkNotFinite(tmp, e_meterGr);
  //   this->peakGr = _T(0);
  //   return tmp;
  // }

  // T getAndResetPeakOut() noexcept {
  //   T tmp = this->peakOut;
  //   checkNotFinite(tmp, e_meterOut);
  //   this->peakOut = _T(0);
  //   return tmp;
  // }

  // NTFX_INLINE_STATIC T log10Opt(T x) noexcept {
  //   if (x < _T(1e-4)) { return -4; }
  //   if (x < _T(1e-3)) { return log10Taylor(x * _T(1e3)); }
  //   if (x < _T(1e-2)) { return log10Taylor(x * _T(1e2)); }
  //   if (x < _T(1e-1)) { return log10Taylor(x * _T(1e1)); }
  //   return log10Taylor(x);
  // }

  // NTFX_INLINE_STATIC T log10Taylor(T x) noexcept {
  //   if (x < 0.2) {
  //     T n = _T(0.325443314844276);
  //     return x * 10 * n - 1 - n;
  //   }
  //   T x_    = x - 1;
  //   T mult  = x_ * x_;
  //   T accum = x_ - mult * _T(1 / 2);
  //   mult *= x_;
  //   accum += mult * _T(1 / 3);
  //   mult *= x_;
  //   accum -= mult * _T(1 / 4);
  //   mult *= x_;
  //   accum += mult * _T(1 / 5);
  //   mult *= x_;
  //   accum -= mult * _T(1 / 6);
  //   mult *= x_;
  //   accum += mult * _T(1 / 7);
  //   mult *= x_;
  //   accum -= mult * _T(1 / 8);
  //   return accum * _T(0.434294481903252);
  // }

  // NTFX_INLINE_STATIC T pow10Opt(T x) noexcept {
  //   if (x < -4) { return 0; }
  //   if (x < -3) { return pow10Taylor(x + 3) * _T(0.001); }
  //   if (x < -2) { return pow10Taylor(x + 2) * _T(0.01); }
  //   if (x < -1) { return pow10Taylor(x + 1) * _T(0.1); }
  //   return pow10Taylor(x);
  // }

  // NTFX_INLINE_STATIC T pow10Taylor(T x) noexcept {
  //   T x_    = -x * _T(2.302585092994046);
  //   T mult  = x_ * x_;
  //   T accum = mult * _T(0.5);
  //   mult *= x_;
  //   accum -= mult * _T(0.166666666666667);
  //   mult *= x_;
  //   accum += mult * _T(0.041666666666667);
  //   mult *= x_;
  //   accum -= mult * _T(0.008333333333333);
  //   mult *= x_;
  //   accum += mult * _T(0.001388888888889);
  //   mult *= x_;
  //   accum -= mult * _T(1.984126984126984e-04);
  //   mult *= x_;
  //   accum += mult * _T(2.480158730158730e-05);
  //   mult *= x_;
  //   accum -= mult * _T(2.755731922398589e-06);
  //   mult *= x_;
  //   accum += mult * _T(2.755731922398589e-07);
  //   mult *= x_;
  //   accum -= mult * _T(2.505210838544172e-08);
  //   mult *= x_;
  //   accum += mult * _T(2.087675698786810e-09);
  //   return 1 - x_ * accum;
  // }
};
}