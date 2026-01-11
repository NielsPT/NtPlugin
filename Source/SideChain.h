#pragma once
#include <cmath>

#include "NtFx.h"
namespace NtFx {

template <typename signal_t>
struct ScSettings {
  signal_t thresh_db = NTFX_SIGNAL(0);
  signal_t ratio_db  = NTFX_SIGNAL(2);
  signal_t knee_db   = NTFX_SIGNAL(0);
  signal_t tAtt_ms   = NTFX_SIGNAL(10);
  signal_t tRel_ms   = NTFX_SIGNAL(100);
  signal_t tRms_ms   = NTFX_SIGNAL(80);
  bool rmsEnable     = false;
  signal_t tPeak_ms  = NTFX_SIGNAL(20.0);
};

template <typename signal_t>
struct ScCoeffs {
  signal_t thresh_db  = NTFX_SIGNAL(-60.0);
  signal_t ratio_db   = NTFX_SIGNAL(2.0);
  signal_t knee_db    = NTFX_SIGNAL(0.0);
  signal_t thresh_lin = NTFX_SIGNAL(1.0);
  signal_t ratio_lin  = NTFX_SIGNAL(1.0);
  signal_t knee_lin   = NTFX_SIGNAL(1.0);
  signal_t alphaAtt   = NTFX_SIGNAL(0.0);
  signal_t alphaRel   = NTFX_SIGNAL(0.0);
  signal_t alphaPeak  = NTFX_SIGNAL(0.0);
  size_t nRms         = 1;
  bool rmsEnable      = false;
};

template <typename signal_t>
struct ScState {
  signal_t ySensLast   = NTFX_SIGNAL(0.0);
  signal_t yFilterLast = NTFX_SIGNAL(0.0);
  RmsSensorState<signal_t> rms;
};

NTFX_INLINE_TEMPLATE ScCoeffs<signal_t> calcSideChainCoeffs(
    int fs, ScSettings<signal_t>* p_settings) {
  ScCoeffs<signal_t> coeffs;
  coeffs.thresh_db = p_settings->thresh_db;
  coeffs.ratio_db  = p_settings->ratio_db;
  coeffs.knee_db   = p_settings->knee_db;
  coeffs.rmsEnable = p_settings->rmsEnable;
  coeffs.alphaAtt  = std::exp(-2200.0 / (p_settings->tAtt_ms * fs));
  coeffs.alphaRel  = std::exp(-2200.0 / (p_settings->tRel_ms * fs));
  if (coeffs.alphaRel < coeffs.alphaAtt) { coeffs.alphaRel = coeffs.alphaAtt; }
  coeffs.alphaPeak      = std::exp(-2200.0 / (p_settings->tPeak_ms * fs));
  coeffs.thresh_lin     = std::pow(10.0, (p_settings->thresh_db / 20.0));
  coeffs.knee_lin       = std::pow(10.0, (p_settings->knee_db / 20.0));
  signal_t oneOverSqrt2 = 1.0 / std::sqrt(2.0);
  coeffs.ratio_lin      = (1.0 - 1.0 / p_settings->ratio_db)
      * (oneOverSqrt2
          - std::pow(oneOverSqrt2 - (p_settings->ratio_db - 3.0) / 18.0, 5.0));
  coeffs.nRms = std::floor(p_settings->tRms_ms * fs * 0.001);
  return coeffs;
}

NTFX_INLINE_TEMPLATE signal_t sideChain_lin(
    ScCoeffs<signal_t>* p_coeffs, ScState<signal_t>* p_state, signal_t x) noexcept {
  signal_t xAbs;
  if (p_coeffs->rmsEnable) {
    xAbs = rmsSensor(&p_state->rms, p_coeffs->nRms, x);
  } else {
    xAbs = std::abs(x);
  }

  signal_t sensRelease =
      p_coeffs->alphaPeak * p_state->ySensLast + (1 - p_coeffs->alphaPeak) * xAbs;
  signal_t ySens     = std::max(xAbs, sensRelease);
  p_state->ySensLast = ySens;

  signal_t target;
  if (ySens < p_coeffs->thresh_lin / p_coeffs->knee_lin) {
    target = NTFX_SIGNAL(0);
  } else if (ySens < p_coeffs->thresh_lin) {
    target = (ySens / p_coeffs->thresh_lin)
        * p_coeffs->ratio_lin
        * p_coeffs->thresh_lin
        / (p_coeffs->knee_lin * ySens);
  } else {
    target = (ySens / p_coeffs->thresh_lin) * p_coeffs->ratio_lin;
  }

  signal_t alpha = p_coeffs->alphaRel;
  if (target > p_state->yFilterLast) { alpha = p_coeffs->alphaAtt; }

  signal_t yFilter     = p_state->yFilterLast * alpha + target * (1 - alpha);
  p_state->yFilterLast = yFilter;
  return NTFX_SIGNAL(1.0) / (yFilter + 1);
}

NTFX_INLINE_TEMPLATE signal_t sideChain_db(
    ScCoeffs<signal_t>* p_coeffs, ScState<signal_t>* p_state, signal_t x) noexcept {
  signal_t xAbs;
  if (p_coeffs->rmsEnable) {
    xAbs = rmsSensor(&p_state->rms, p_coeffs->nRms, x);
  } else {
    xAbs = std::abs(x);
  }

  signal_t sensRelease =
      p_coeffs->alphaPeak * p_state->ySensLast + (1 - p_coeffs->alphaPeak) * xAbs;
  signal_t ySens = std::max(xAbs, sensRelease);
  if (ySens != ySens) { ySens = NTFX_SIGNAL(0); }
  p_state->ySensLast = ySens;

  signal_t x_db = db(ySens);
  signal_t y_db;
  if ((x_db - p_coeffs->thresh_db) > (p_coeffs->knee_db / 2)) {
    y_db = p_coeffs->thresh_db + (x_db - p_coeffs->thresh_db) / p_coeffs->ratio_db;
  } else if ((x_db - p_coeffs->thresh_db) < -(p_coeffs->knee_db / 2)) {
    y_db = x_db;
  } else {
    signal_t tmp = (x_db - p_coeffs->thresh_db + p_coeffs->knee_db / 2);
    y_db = x_db + (1 / p_coeffs->ratio_db - 1) * tmp * tmp / (2 * p_coeffs->knee_db);
  }

  signal_t target = x_db - y_db;

  signal_t alpha = p_coeffs->alphaRel;
  if (target > p_state->yFilterLast) { alpha = p_coeffs->alphaAtt; }

  signal_t yFilter = p_state->yFilterLast * alpha + target * (1 - alpha);
  if (yFilter != yFilter) { yFilter = NTFX_SIGNAL(0); }
  p_state->yFilterLast = yFilter;
  return invDb(-yFilter);
}
} // namespace NtFx

// // template <typename signal_t>
// // struct SideChain {
// //   signal_t thresh_db = -60;
// //   signal_t ratio     = 2;
// //   signal_t knee_db   = 0;
// //   signal_t tAtt_ms   = 10;
// //   signal_t tRel_ms   = 100;
// //   signal_t tRms_ms   = 80;
// //   bool rmsEnable     = false;
// //   // signal_t fs        = 0;
// //   int nRms = 441;
// //   int iRms = 0;
// //   int fs   = 44100;

// //   signal_t tPeak       = SIGNAL(20.0);
// //   signal_t alphaAtt    = SIGNAL(0.0);
// //   signal_t alphaRel    = SIGNAL(0.0);
// //   signal_t alphaPeak   = SIGNAL(0.0);
// //   signal_t rmsAccum    = SIGNAL(0.0);
// //   signal_t ySensLast   = SIGNAL(0.0);
// //   signal_t yFilterLast = SIGNAL(0.0);
// //   std::array<signal_t, rmsDelayLineLength> rmsDelayLine;

// //   virtual void reset(int fs) noexcept {
// //     this->fs = fs;
// //     std::fill(this->rmsDelayLine.begin(), this->rmsDelayLine.end(), SIGNAL(0));
// //     this->rmsAccum    = SIGNAL(0);
// //     this->ySensLast   = SIGNAL(0.0);
// //     this->yFilterLast = SIGNAL(0.0);
// //     this->update();
// //   }

// //   virtual void update() noexcept {
// //     this->alphaAtt = std::exp(-2200.0 / (this->tAtt_ms * this->fs));
// //     this->alphaRel = std::exp(-2200.0 / (this->tRel_ms * this->fs));
// //     if (this->alphaRel < this->alphaAtt) { this->alphaRel = this->alphaAtt; }
// //     this->alphaPeak = std::exp(-2200.0 / (this->tPeak * this->fs));
// //     this->nRms      = std::floor(this->tRms_ms * this->fs * 0.001);
// //   }

// //   virtual signal_t gainComputer(signal_t x) = 0;

// //   virtual signal_t scaleResult(signal_t x) = 0;

// //   NTFX_INLINE_MEMBER virtual signal_t processSample(signal_t x) noexcept {
// //     signal_t xAbs = std::abs(x);
// //     if (this->rmsEnable) { xAbs = this->rmsSensor(x); }

// //     signal_t sensRelease =
// //         this->alphaPeak * this->ySensLast + (1 - this->alphaPeak) * xAbs;
// //     signal_t ySens  = std::max(xAbs, sensRelease);
// //     this->ySensLast = ySens;

// //     signal_t target = this->gainComputer(ySens);
// //     signal_t alpha  = this->alphaRel;
// //     if (target > yFilterLast) { alpha = this->alphaAtt; }

// //     signal_t yFilter  = this->yFilterLast * alpha + target * (1 - alpha);
// //     this->yFilterLast = yFilter;
// //     return this->scaleResult(yFilter);
// //   }

// //   NTFX_INLINE_MEMBER signal_t rmsSensor(signal_t x) noexcept {
// //     signal_t _x = x * x;
// //     this->rmsAccum += _x - this->rmsDelayLine[this->iRms];
// //     this->rmsDelayLine[this->iRms] = _x;
// //     this->iRms++;
// //     if (this->iRms >= this->nRms) { this->iRms = 0; }
// //     // if (checkNotFiniteEnabled && this->rmsAccum < 0) {
// //     //   this->errorVal = ErrorVal::;
// //     //   return 0;
// //     // }
// //     signal_t y = std::sqrt(2.0 * this->rmsAccum / this->nRms);
// //     // checkNotFinite(y, ErrorVal::e_rmsSensor);
// //     if (y != y) { return SIGNAL(0.0); }
// //     return y;
// //   }
// // };

// // template <typename signal_t>
// // struct SideChain_lin : public SideChain<signal_t> {
// //   signal_t thresh_lin = SIGNAL(1.0);
// //   signal_t makeup_lin = SIGNAL(1.0);
// //   signal_t knee_lin   = SIGNAL(1.0);
// //   signal_t ratio_lin  = SIGNAL(1.0);

// //   NTFX_INLINE_MEMBER virtual signal_t gainComputer(signal_t x) noexcept override {
// //     signal_t target = 0;
// //     if (x < this->thresh_lin / this->knee_lin) {
// //       target = 0;
// //     } else if (x < this->thresh_lin) {
// //       target = (x / this->thresh_lin)
// //           * this->ratio_lin
// //           * this->thresh_lin
// //           / (this->knee_lin * x);
// //     } else {
// //       target = (x / this->thresh_lin) * this->ratio_lin;
// //     }
// //     return target;
// //   }

// //   NTFX_INLINE_MEMBER virtual signal_t scaleResult(signal_t x) noexcept override {
// //     return SIGNAL(1.0) / (x + 1);
// //   }

// //   virtual void update() noexcept override {
// //     this->thresh_lin      = std::pow(10.0, (this->thresh_db / 20.0));
// //     this->knee_lin        = std::pow(10.0, (this->knee_db / 20.0));
// //     signal_t oneOverSqrt2 = 1.0 / std::sqrt(2.0);
// //     this->ratio_lin       = (1.0 - 1.0 / this->ratio)
// //         * (oneOverSqrt2 - std::pow(oneOverSqrt2 - (this->ratio - 3.0)
// / 18.0, 5.0));
// //     SideChain<signal_t>::update();
// //   }
// // };

// // template <typename signal_t>
// // struct SideChain_db : public SideChain<signal_t> {
// //   NTFX_INLINE_STATIC signal_t db(signal_t x) noexcept {
// //     return SIGNAL(20.0) * std::log10(x);
// //   }

// //   NTFX_INLINE_STATIC signal_t invDb(signal_t x) noexcept {
// //     return std::pow(SIGNAL(10.0), x * SIGNAL(0.05));
// //   }

// //   NTFX_INLINE_MEMBER virtual signal_t gainComputer(signal_t x) noexcept override {
// //     signal_t x_db = this->db(x);
// //     signal_t y_db;
// //     if ((x_db - this->thresh_db) > (this->knee_db / 2)) {
// //       y_db = this->thresh_db + (x_db - this->thresh_db) / this->ratio;
// //     } else if ((x_db - this->thresh_db) < -(this->knee_db / 2)) {
// //       y_db = x_db;
// //     } else {
// //       signal_t tmp = (x_db - this->thresh_db + this->knee_db / 2);
// //       y_db         = x_db + (1 / this->ratio - 1) * tmp * tmp / (2 *
// this->knee_db);
// //     }
// //     return x_db - y_db;
// //   }

// //   NTFX_INLINE_MEMBER virtual signal_t scaleResult(signal_t x) noexcept override {
// //     return this->invDb(-x);
// //   }
// // };
// } // namespace NtFx
