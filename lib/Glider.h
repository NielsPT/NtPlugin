#pragma once

#include <cmath>
#include <cstdlib>

namespace NtFx {
template <typename signal_t>
struct ExpGlider {
  signal_t ui = 0; // UI side that is glided to
  signal_t pr = 0; // Processing side
  signal_t a  = 0; // Smoothing coefficient
  ExpGlider(signal_t def = 0) : ui(def), pr(def) { }
  inline signal_t process() noexcept {
    this->pr += this->a * (this->ui - this->pr);
    return this->pr;
  }
  inline void update(signal_t fs, signal_t smoothingTime = 0.1) {
    this->a = 1.0 - std::exp(-1.0 / (fs * smoothingTime));
  }
};

template <typename signal_t>
struct LinGlider {
  signal_t ui = 0.0; // UI side that is glided to
  signal_t pr = 0.0; // Processing side
  signal_t s  = 0.0;
  LinGlider(signal_t def = 0) : ui(def), pr(def) { }
  inline signal_t process() noexcept {
    if (this->ui == this->pr) { return this->pr; }
    if (this->pr < this->ui) {
      this->pr += s;
    } else {
      this->pr -= s;
    }
    if (std::abs(this->pr) < std::abs(this->s)) { this->pr = this->ui; }
    return this->pr;
  }
  inline void update(signal_t fs, signal_t range, signal_t tSmooth = 0.1) {
    this->s = range / (tSmooth * fs);
  }
};
}
