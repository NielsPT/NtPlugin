
#pragma once
#include <algorithm> // std::max
#include <array>
#include <cmath> // std::log10, std::exp, std::floor, std::pow, std::sqrt
#define NTFX_INLINE_TEMPLATE                                                           \
  template <typename signal_t>                                                         \
  __attribute__((always_inline)) static inline
#define NTFX_INLINE_MEMBER __attribute__((always_inline)) inline
#define NTFX_INLINE_STATIC __attribute__((always_inline)) inline static
#define SIGNAL(v) static_cast<signal_t>(v)

namespace NtFx {
constexpr int rmsDelayLineLength = 16384;
template <typename signal_t>
NTFX_INLINE_STATIC signal_t db(signal_t x) noexcept {
  return SIGNAL(20.0) * std::log10(x);
}

template <typename signal_t>
NTFX_INLINE_STATIC signal_t invDb(signal_t x) noexcept {
  // if (optimizeDb) { return pow10Opt(x * SIGNAL(0.05)); }
  return std::pow(SIGNAL(10.0), x * SIGNAL(0.05));
}
// template <typename signal_t>
// struct ProcBlock {
//   virtual void update()                      = 0;
//   virtual signal_t processSample(signal_t x) = 0;
// };

template <typename signal_t>
struct SideChain { // : public ProcBlock<signal_t> {
  signal_t thresh_db = 0;
  signal_t ratio     = 2;
  signal_t knee_db   = 0;
  signal_t tAtt_ms   = 10;
  signal_t tRel_ms   = 100;
  signal_t tRms_ms   = 80;
  bool rmsEnable     = false;
  // signal_t fs        = 0;
  int nRms = 441;
  int iRms = 0;
  int fs   = 44100;

  signal_t tPeak     = SIGNAL(20.0);
  signal_t alphaAtt  = SIGNAL(0.0);
  signal_t alphaRel  = SIGNAL(0.0);
  signal_t alphaPeak = SIGNAL(0.0);
  signal_t rmsAccum  = SIGNAL(0.0);
  std::array<signal_t, 2> scState;
  std::array<signal_t, rmsDelayLineLength> rmsDelayLine;

  virtual void update() noexcept {
    this->alphaAtt = std::exp(-2200.0 / (this->tAtt_ms * this->fs));
    this->alphaRel = std::exp(-2200.0 / (this->tRel_ms * this->fs));
    if (this->alphaRel < this->alphaAtt) { this->alphaRel = this->alphaAtt; }
    this->alphaPeak = std::exp(-2200.0 / (this->tPeak * this->fs));
    this->nRms      = std::floor(this->tRms_ms * this->fs * 0.001);
  }

  virtual signal_t gainComputer(signal_t x) = 0;

  virtual signal_t processSample(signal_t x) noexcept {
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

  NTFX_INLINE_MEMBER signal_t rmsSensor(signal_t x) noexcept {
    signal_t _x = x * x;
    this->rmsAccum += _x - this->rmsDelayLine[this->iRms];
    this->rmsDelayLine[this->iRms] = _x;
    this->iRms++;
    if (this->iRms >= this->nRms) { this->iRms = 0; }
    // if (checkNotFiniteEnabled && this->rmsAccum < 0) {
    //   this->errorVal = ErrorVal::;
    //   return 0;
    // }
    signal_t y = std::sqrt(2.0 * this->rmsAccum / this->nRms);
    // checkNotFinite(y, ErrorVal::e_rmsSensor);
    if (y != y) { return SIGNAL(0.0); }
    return y;
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
    return target;
  }

  virtual void update() noexcept override {
    this->thresh_lin    = std::pow(10.0, (this->thresh_db / 20.0));
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
    return x_db - y_db;
  }
};

} // namespace NtFx
