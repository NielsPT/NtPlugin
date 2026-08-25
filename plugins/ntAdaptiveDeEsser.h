#pragma once

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
 *
 * You are free to download, build and use this code for commercial
 * purposes. Just don't resell it or a build of it, modified or otherwise.
 **/

#include "lib/AdaptiveDeEssSc.h"
#include "lib/Audio.h"
#include "lib/DynamicFilter.h"
#include "lib/Plugin.h"

struct ntAdaptiveDeEsser final : public NtFx::Plugin {
  NtFx::AdaptiveDeEssSc sc;
  NtFx::DynamicFilter::ShelfFixedPoles shelf;
  signal_t red_p { 100 };
  signal_t red_lin { 100 };
  bool bypassEnable { false };

  ntAdaptiveDeEsser() {
    this->primaryKnobs = {
      {
          .p_val    = &this->sc.fc_hz,
          .name     = "Frequency",
          .suffix   = " Hz",
          .minVal   = 2e3,
          .maxVal   = 20e3,
          .logScale = true,
      },
      {
          .p_val  = &this->red_p,
          .name   = "Reduction",
          .suffix = " %",
          .minVal = 0,
          .maxVal = 100,
      },
    };
    this->toggles = {
      { .p_val = &this->bypassEnable, .name = "Bypass" },
    };
    this->meters.push_back({ .name = "GR", .invert = true });
    this->shelf.q1 = 0.508;
    this->shelf.q2 = 0.508;
    this->updateDefaults();
  }

  Audio process(Audio x) noexcept override {
    this->template updatePeakLevel<0>(x);
    if (this->bypassEnable) {
      this->template updatePeakLevel<1>(x);
      return x;
    }
    this->shelf.gain_lin =
        (this->sc.process(x) * this->red_lin - this->red_lin + 1).absMin();
    auto y = this->shelf.process(x);
    this->template updatePeakLevel<1>(y);
    this->template updatePeakLevel<2, true>(this->shelf.gain_lin);
    return y;
  }

  void update() noexcept override {
    this->red_lin     = this->red_p / 100;
    this->shelf.fc_hz = this->sc.fc_hz;
    this->shelf.update();
    this->sc.update();
  }

  void reset(signal_t fs) noexcept override {
    this->_fs = fs;
    this->sc.reset(fs);
    this->shelf.reset(fs);
    this->update();
  }
};
