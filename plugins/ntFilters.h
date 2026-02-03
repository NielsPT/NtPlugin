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
 *
 **/

#pragma once

#include "lib/FirstOrder.h"
#include "lib/Plugin.h"
#include "lib/Stereo.h"

template <typename signal_t>
struct ntFilters : public NtFx::NtPlugin<signal_t> {
  float fs;
  signal_t fHpf = 20;
  signal_t fLpf = 20000;
  bool bypass   = false;
  NtFx::FirstOrder::FilterStereo<signal_t, NtFx::FirstOrder::Shape::hpf>
      firstOrderHpf;
  NtFx::FirstOrder::FilterStereo<signal_t, NtFx::FirstOrder::Shape::lpf>
      firstOrderLpf;
  ntFilters() {
    this->uiSpec.defaultWindowWidth = 400;
    this->primaryKnobs              = {
      { &this->fHpf, "HPF", " Hz", 20, 20e3, 2e3 },
      { &this->fLpf, "LPF", " Hz", 20, 20e3, 2e3 },
    };
    this->toggles       = { { &this->bypass, "Bypass" } };
    this->uiSpec.meters = { { "IN" }, { "OUT", .hasScale = true } };
    this->updateDefaults();
  }

  NtFx::Stereo<signal_t> process(NtFx::Stereo<signal_t> x) noexcept override {
    auto yHpf = this->firstOrderHpf.process(x);
    auto yLpf = this->firstOrderLpf.process(yHpf);
    auto y    = yLpf;
    this->template updatePeakLevel<0>(x);
    if (this->bypass) {
      this->template updatePeakLevel<1>(x);
      return x;
    }
    this->template updatePeakLevel<1>(y);
    return y;
  }

  void update() noexcept override {
    this->firstOrderHpf.setFc(fHpf);
    this->firstOrderLpf.setFc(fLpf);
    this->firstOrderHpf.update();
    this->firstOrderLpf.update();
  }

  void reset(int fs) noexcept override {
    this->firstOrderHpf.reset(fs);
    this->firstOrderLpf.reset(fs);
  }
};
