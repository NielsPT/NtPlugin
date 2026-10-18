#pragma once

/**
 * @file Tilt.h
 * @author Niels Thøgersen (niels.thoegersen@gmail.com)
 * @brief Tilting filter.
 *
 * @copyright Copyright (c) 2026
 *
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
 */

#include "lib/Audio.h"
#include "lib/Component.h"
#include "lib/FirstOrder.h"
#include "lib/gcem.h"
#include "lib/utils.h"
#include <array>
#include <cstddef>

namespace NtFx {
template <size_t nStages = 8>
struct Tilt : public ComponentBase<Audio> {
  std::array<NtFx::FirstOrder::StereoFilter<NtFx::FirstOrder::Shape::lpfZero>,
      nStages>
      filters;
  signal_t a_lin { 0 };
  signal_t tilt_db { 0 }; /** @brief Tilt in dB / decade. */
  signal_t gain_lin { 1 };
  signal_t calcCorr() {
    auto tiltAbs = gcem::abs(this->tilt_db);
    signal_t corr_db { 0 };
    signal_t corr_lin { 1 };
    if (tiltAbs < 1) {
      corr_db = 0;
    } else if (tiltAbs < 10) {
      corr_db = 0.06 * this->tilt_db;
    } else {
      if (this->tilt_db > 0) {
        corr_db = 0.58 * this->tilt_db - 5;
      } else {
        corr_db = 0.58 * this->tilt_db + 5;
      }
    }
    if (this->tilt_db > 0) {
      corr_lin = NtFx::invDb(-corr_db / 2);
    } else {
      corr_lin = NtFx::invDb(corr_db / 2);
    }
    return corr_lin;
  }
  Audio process(Audio x) noexcept override {
    auto tmp = x;
    for (size_t i = 0; i < nStages; i++) {
      auto yLpf = filters[i].process(tmp);
      tmp       = yLpf * a_lin + tmp;
    }
    return tmp * gain_lin;
  }
  void update() noexcept override {
    this->a_lin    = invDb(-this->tilt_db * 3 / nStages) - 1;
    this->gain_lin = invDb(this->tilt_db * 3 / 2) * calcCorr();
    for (size_t i = 0; i < nStages; i++) { filters[i].update(); }
  }
  void reset(signal_t fs) noexcept override {
    this->_fs    = fs;
    signal_t fLo = 20;   // this->_fs / 8000;
    signal_t fHi = 20e3; // this->_fs / 2;
    auto freqs =
        logspace<signal_t>(gcem::log10(fLo), gcem::log10(fHi), nStages);
    for (size_t i = 0; i < nStages; i++) { filters[i].fc_hz = freqs[i]; }
    for (size_t i = 0; i < nStages; i++) { filters[i].reset(fs); }
    this->update();
  }
};
}
