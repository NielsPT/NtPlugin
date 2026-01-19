#pragma once

#include "lib/utils.h"

#include <cmath>
namespace NtFx {
namespace SideChain {
  template <typename signal_t>
  struct Settings {
    signal_t thresh_db = static_cast<signal_t>(0);
    signal_t ratio_db  = static_cast<signal_t>(2);
    signal_t knee_db   = static_cast<signal_t>(12);
    signal_t tAtt_ms   = static_cast<signal_t>(1);
    signal_t tRel_ms   = static_cast<signal_t>(100);
    signal_t tRms_ms   = static_cast<signal_t>(80);
    bool rmsEnable     = false;
    signal_t tPeak_ms  = static_cast<signal_t>(20.0);
  };

  template <typename signal_t>
  struct Coeffs {
    signal_t thresh_db  = static_cast<signal_t>(0);
    signal_t ratio_db   = static_cast<signal_t>(1);
    signal_t knee_db    = static_cast<signal_t>(0);
    signal_t thresh_lin = static_cast<signal_t>(1);
    signal_t ratio_lin  = static_cast<signal_t>(1);
    signal_t knee_lin   = static_cast<signal_t>(1);
    signal_t alphaAtt   = static_cast<signal_t>(0);
    signal_t alphaRel   = static_cast<signal_t>(0);
    signal_t alphaPeak  = static_cast<signal_t>(0);
    size_t nRms         = 1;
    bool rmsEnable      = false;
  };

  template <typename signal_t>
  struct State {
    signal_t ySensLast   = static_cast<signal_t>(0.0);
    signal_t yFilterLast = static_cast<signal_t>(0.0);
    RmsSensorState<signal_t> rms;
    void reset() {
      this->ySensLast   = static_cast<signal_t>(0.0);
      this->yFilterLast = static_cast<signal_t>(0.0);
      rms.reset();
    }
  };

  template <typename signal_t>
  static inline Coeffs<signal_t> calcCoeffs(
      int fs, Settings<signal_t>* p_settings) {
    Coeffs<signal_t> coeffs;
    coeffs.thresh_db = p_settings->thresh_db;
    coeffs.ratio_db  = p_settings->ratio_db;
    coeffs.knee_db   = p_settings->knee_db;
    coeffs.rmsEnable = p_settings->rmsEnable;
    coeffs.alphaAtt  = std::exp(-2200.0 / (p_settings->tAtt_ms * fs));
    coeffs.alphaRel  = std::exp(-2200.0 / (p_settings->tRel_ms * fs));
    if (coeffs.alphaRel < coeffs.alphaAtt) {
      coeffs.alphaRel = coeffs.alphaAtt;
    }
    coeffs.alphaPeak      = std::exp(-2200.0 / (p_settings->tPeak_ms * fs));
    coeffs.thresh_lin     = std::pow(10.0, (p_settings->thresh_db / 20.0));
    coeffs.knee_lin       = std::pow(10.0, (p_settings->knee_db / 20.0));
    signal_t oneOverSqrt2 = 1.0 / std::sqrt(2.0);
    coeffs.ratio_lin      = (1.0 - 1.0 / p_settings->ratio_db)
        * (oneOverSqrt2
            - std::pow(
                oneOverSqrt2 - (p_settings->ratio_db - 3.0) / 18.0, 5.0));
    coeffs.nRms = std::floor(p_settings->tRms_ms * fs * 0.001);
    return coeffs;
  }

  template <typename signal_t>
  static inline signal_t sideChain_lin(Coeffs<signal_t>* p_coeffs,
      State<signal_t>* p_state, signal_t x) noexcept {
    signal_t xAbs;
    if (p_coeffs->rmsEnable) {
      xAbs = rmsSensor(&p_state->rms, p_coeffs->nRms, x);
    } else {
      xAbs = std::abs(x);
    }

    signal_t sensRelease = p_coeffs->alphaPeak * p_state->ySensLast
        + (1 - p_coeffs->alphaPeak) * xAbs;
    signal_t ySens     = std::max(xAbs, sensRelease);
    p_state->ySensLast = ySens;

    signal_t target;
    if (ySens < p_coeffs->thresh_lin / p_coeffs->knee_lin) {
      target = static_cast<signal_t>(0);
    } else if (ySens < p_coeffs->thresh_lin) {
      target = (ySens / p_coeffs->thresh_lin) * p_coeffs->ratio_lin
          * p_coeffs->thresh_lin / (p_coeffs->knee_lin * ySens);
    } else {
      target = (ySens / p_coeffs->thresh_lin) * p_coeffs->ratio_lin;
    }

    signal_t alpha = p_coeffs->alphaRel;
    if (target > p_state->yFilterLast) { alpha = p_coeffs->alphaAtt; }

    signal_t yFilter     = p_state->yFilterLast * alpha + target * (1 - alpha);
    p_state->yFilterLast = yFilter;
    return static_cast<signal_t>(1.0) / (yFilter + 1);
  }

  template <typename signal_t>
  static inline signal_t sideChain_db(Coeffs<signal_t>* p_coeffs,
      State<signal_t>* p_state, signal_t x) noexcept {
    signal_t xAbs;
    if (p_coeffs->rmsEnable) {
      xAbs = rmsSensor(&p_state->rms, p_coeffs->nRms, x);
    } else {
      xAbs = std::abs(x);
    }

    signal_t sensRelease = p_coeffs->alphaPeak * p_state->ySensLast
        + (1 - p_coeffs->alphaPeak) * xAbs;
    signal_t ySens = std::max(xAbs, sensRelease);
    if (ySens != ySens) { ySens = static_cast<signal_t>(0); }
    p_state->ySensLast = ySens;

    signal_t x_db = db(ySens);
    signal_t y_db;
    if ((x_db - p_coeffs->thresh_db) > (p_coeffs->knee_db / 2)) {
      y_db = p_coeffs->thresh_db
          + (x_db - p_coeffs->thresh_db) / p_coeffs->ratio_db;
    } else if ((x_db - p_coeffs->thresh_db) < -(p_coeffs->knee_db / 2)) {
      y_db = x_db;
    } else {
      signal_t tmp = (x_db - p_coeffs->thresh_db + p_coeffs->knee_db / 2);
      y_db         = x_db
          + (1 / p_coeffs->ratio_db - 1) * tmp * tmp / (2 * p_coeffs->knee_db);
    }

    signal_t target = x_db - y_db;

    signal_t alpha = p_coeffs->alphaRel;
    if (target > p_state->yFilterLast) { alpha = p_coeffs->alphaAtt; }

    signal_t yFilter = p_state->yFilterLast * alpha + target * (1 - alpha);
    if (yFilter != yFilter) { yFilter = static_cast<signal_t>(0); }
    p_state->yFilterLast = yFilter;
    return invDb(-yFilter);
  }
} // namespace SideChain
} // namespace NtFx
