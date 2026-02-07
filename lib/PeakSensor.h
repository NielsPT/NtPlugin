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
#include "lib/Component.h"
#include "lib/Stereo.h"
#include "lib/utils.h"

namespace NtFx {

template <typename signal_t>
struct PeakSensor : public Component<signal_t> {
  signal_t tPeak_ms;
  signal_t alpha;
  signal_t state;

  virtual signal_t process(signal_t x) noexcept override {
    return this->_peakSensor(this->alpha, this->state, x);
  }

  virtual void update() noexcept override {
    this->alpha = gcem::exp(-2200.0 / (this->tPeak_ms * this->fs));
  }

  static inline signal_t _peakSensor(
      signal_t alpha, signal_t& p_state, signal_t x) {
    auto xAbs            = gcem::abs(x);
    signal_t sensRelease = alpha * p_state + (1 - alpha) * xAbs;
    signal_t ySens       = gcem::max(xAbs, sensRelease);
    ensureFinite(ySens);
    p_state = ySens;
    return ySens;
  }
};

template <typename signal_t>
struct PeakSensorStereo
    : public StereoComponent<signal_t, PeakSensor<signal_t>> {
  void setT_ms(signal_t t_ms) {
    this->l.tPeak_ms = t_ms;
    this->r.tPeak_ms = t_ms;
  }
};
}