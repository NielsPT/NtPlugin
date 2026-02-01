#pragma once

#include <cmath>

namespace NtFx {
template <typename T>
struct Glider {
  T ui = 0; // UI side that is glided to
  T pr = 0; // Processing side
  T a  = 0; // Smoothing coefficient
  Glider(T def = 0) : ui(def), pr(def) { }
  inline T process() noexcept {
    this->pr = this->pr + this->a * (this->ui - this->pr);
    return this->pr;
  }
  inline void update(T fs, T smoothingTime = 1) {
    this->a = 1.0 - std::exp(-1.0 / (fs * smoothingTime));
  }
};
}
