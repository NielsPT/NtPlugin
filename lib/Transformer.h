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

#include "lib/Audio.h"
#include "lib/Component.h"
#include "lib/FirstOrder.h"
#include "lib/SoftClip.h"

namespace NtFx {

struct Transformer : public ComponentBase<Audio> {
  signal_t fc_hz       = 250;
  signal_t lfCutoff_hz = 20;
  signal_t gain_lin { 0 };
  NtFx::FirstOrder::StereoFilter<NtFx::FirstOrder::Shape::lpf> lpf;
  NtFx::FirstOrder::StereoFilter<NtFx::FirstOrder::Shape::hpf> hpf;
  virtual Audio process(Audio x) noexcept override {
    auto yShelf = x + this->lpf.process(x) * this->gain_lin;
    auto yClip  = NtFx::softClip5thStereo(yShelf);
    auto yHpf   = this->hpf.process(yClip);
    return yHpf;
  }
  virtual void update() noexcept override {
    if (this->fc_hz < 20) { this->fc_hz = 20; }
    this->gain_lin  = this->fc_hz / this->lfCutoff_hz - 1;
    this->hpf.fc_hz = fc_hz;
    this->lpf.update();
    this->hpf.update();
  }
  virtual void reset(float fs) noexcept override {
    this->fs        = fs;
    this->lpf.fc_hz = this->lfCutoff_hz;
    this->lpf.reset(fs);
    this->hpf.reset(fs);
    this->update();
  }
};
}