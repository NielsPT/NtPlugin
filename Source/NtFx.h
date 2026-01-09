#pragma once
#include <array>

#include "Stereo.h"

namespace NtFx {
template <typename signal_t>
NTFX_INLINE_STATIC signal_t db(signal_t x) noexcept {
  return SIGNAL(20.0) * std::log10(x);
}

template <typename signal_t>
struct ProcBlock {
  virtual void update()                      = 0;
  virtual signal_t processSample(signal_t x) = 0;
};

template <typename signal_t>
struct SideChain : public ProcBlock<signal_t> {
  signal_t thresh_db = 0;
  signal_t ratio     = 2;
  signal_t knee_db   = 0;
  signal_t tAtt_ms   = 10;
  signal_t tRel_ms   = 100;
  signal_t tRms_ms   = 80;

  signal_t tPeak     = SIGNAL(20.0);
  signal_t alphaAtt  = SIGNAL(0.0);
  signal_t alphaRel  = SIGNAL(0.0);
  signal_t alphaPeak = SIGNAL(0.0);
  std::array<signal_t, 2> scState;

  virtual void update() noexcept override {
    this->alphaAtt = std::exp(-2200.0 / (this->tAtt_ms * this->fs));
    this->alphaRel = std::exp(-2200.0 / (this->tRel_ms * this->fs));
    if (this->alphaRel < this->alphaAtt) { this->alphaRel = this->alphaAtt; }
    this->alphaPeak = std::exp(-2200.0 / (this->tPeak * this->fs));
    this->nRms      = std::floor(this->tRms_ms * this->fs * 0.001);
  }

  virtual signal_t gainComputer(signal_t x) = 0;

  virtual signal_t processSample(signal_t x) noexcept override {
    signal_t ySensLast   = this->scState[0];
    signal_t yFilterLast = this->scState[1];
    signal_t xAbs        = std::abs(x);
    if (this->rmsEnable) { xAbs = this->rmsSensor(x); }

    signal_t sensRelease = this->alphaPeak * ySensLast + (1 - this->alphaPeak) * xAbs;
    signal_t ySens       = std::max(xAbs, sensRelease);
    ySensLast            = ySens;

    signal_t target = this->gainComputer(ySens);
    signal_t alpha  = this->alphaRel;
    if (target > yFilterLast) { alpha = this->alphaAtt; }

    signal_t yFilter = yFilterLast * alpha + target * (1 - alpha);
    yFilterLast      = yFilter;
    this->scState[0] = ySensLast;
    this->scState[1] = yFilterLast;
    return invDb(-yFilter);
  }
};

template <typename signal_t>
struct SideChain_lin : public SideChain<signal_t> {
  signal_t thresh_lin = SIGNAL(1.0);
  signal_t makeup_lin = SIGNAL(1.0);
  signal_t knee_lin   = SIGNAL(1.0);
  signal_t ratio_lin  = SIGNAL(1.0);

  virtual signal_t gainComputer(signal_t x) noexcept override {
    signal_t target = 0;
    if (x < this->thresh_lin / this->knee_lin) {
      target = 0;
    } else if (x < this->thresh_lin) {
      target = (x / this->thresh_lin)
          * this->ratio_lin
          * this->thresh_lin
          / (this->knee_lin * x);
    } else {
      target = (x / this->thresh_lin) * this->ratio_lin;
    }
  }

  virtual void update() {
    this->thresh_lin    = std::pow(10.0, (this->thresh_db / 20.0));
    this->makeup_lin    = std::pow(10.0, (this->makeup_db / 20.0));
    this->knee_lin      = std::pow(10.0, (this->knee_db / 20.0));
    double oneOverSqrt2 = 1.0 / std::sqrt(2.0);
    this->ratio_lin     = (1.0 - 1.0 / this->ratio)
        * (oneOverSqrt2 - std::pow(oneOverSqrt2 - (this->ratio - 3.0) / 18.0, 5.0));
    SideChain<signal_t>::update();
  }
};

template <typename signal_t>
struct SideChain_db : public SideChain<signal_t> {
  virtual signal_t gainComputer(signal_t x) noexcept override {
    signal_t x_db = db(x);
    signal_t y_db;
    if ((x_db - this->thresh_db) > (this->knee_db / 2)) {
      y_db = this->thresh_db + (x_db - this->thresh_db) / this->ratio;
    } else if ((x_db - this->thresh_db) < -(this->knee_db / 2)) {
      y_db = x_db;
    } else {
      signal_t tmp = (x_db - this->thresh_db + this->knee_db / 2);
      y_db         = x_db + (1 / this->ratio - 1) * tmp * tmp / (2 * this->knee_db);
    }
  }
};

} // namespace NtFx
