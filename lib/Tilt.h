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
#include "lib/Transformer.h"
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
  signal_t tilt_db { 0 };
  signal_t gain_lin { 1 };
  Tilt() {
    auto freqs =
        logspace<signal_t>(gcem::log10(20), gcem::log10(20e3), nStages);
    for (size_t i = 0; i < nStages; i++) { filters[i].fc_hz = freqs[i]; }
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
    this->a_lin    = invDb(-2 * this->tilt_db / nStages) - 1;
    this->gain_lin = invDb(this->tilt_db);
    for (size_t i = 0; i < nStages; i++) { filters[i].update(); }
  }
  void reset(signal_t fs) noexcept override {
    this->_fs = fs;
    for (size_t i = 0; i < nStages; i++) { filters[i].reset(fs); }
    this->update();
  }
};
}
