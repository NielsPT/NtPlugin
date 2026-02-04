/*
 * Copyright (C) 2026 Niels Thøgersen, NTlyd
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Affero General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option) any
 * later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 **/

#pragma once

#include "gcem.hpp"

namespace NtFx {
template <typename signal_t>
struct ExpGlider {
  signal_t ui = 0;
  signal_t pr = 0;
  signal_t a  = 0;
  ExpGlider(signal_t def = 0) : ui(def), pr(def) { }
  inline signal_t process() noexcept {
    this->pr += this->a * (this->ui - this->pr);
    return this->pr;
  }
  inline void update(signal_t fs, signal_t tSmooth) {
    if (tSmooth <= 0) {
      this->a  = 0;
      this->pr = this->ui;
      return;
    }
    this->a = 1.0 - gcem::exp(-1.0 / (fs * tSmooth));
  }
};

template <typename signal_t>
struct LinGlider {
  signal_t ui = 0.0;
  signal_t pr = 0.0;
  signal_t s  = 0.0;
  LinGlider(signal_t def = 0) : ui(def), pr(def) { }
  inline signal_t process() noexcept {
    if (this->ui == this->pr) { return this->pr; }
    if (this->pr < this->ui) {
      this->pr += s;
    } else {
      this->pr -= s;
    }
    if (gcem::abs(this->ui - this->pr) < gcem::abs(this->s)) {
      this->pr = this->ui;
    }
    return this->pr;
  }
  inline void update(signal_t fs, signal_t tSmooth) {
    if (tSmooth <= 0) {
      this->pr = this->ui;
      return;
    }
    this->s = gcem::abs(this->ui - this->pr) / (tSmooth * fs);
  }
};
}
