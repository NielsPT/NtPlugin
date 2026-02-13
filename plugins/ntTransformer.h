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

#pragma once

#include "lib/Biquad.h"
#include "lib/Plugin.h"
#include "lib/Stereo.h"
#include "lib/Transformer.h"
#include "lib/utils.h"

template <typename signal_t>
struct ntTransformer : public NtFx::NtPlugin<signal_t> {
  NtFx::Biquad::EqBand<signal_t> bqHpf0;
  NtFx::Transformer<signal_t> transformer;
  int rbVal;
  signal_t drive_db  = -10;
  signal_t drive_lin = 0.3;
  bool bypass        = false;
  ntTransformer() {
    this->primaryKnobs = {
      { &this->drive_db, "Drive", " dB", -24, 24 },
    };
    this->toggles                   = { { &this->bypass, "Bypass" } };
    this->uiSpec.defaultWindowWidth = 350;
  }
  virtual NtFx::Stereo<signal_t> process(
      NtFx::Stereo<signal_t> x) noexcept override {
    this->template updatePeakLevel<0>(x);
    if (this->bypass) {
      this->template updatePeakLevel<1>(x);
      return x;
    }
    auto xTrans = this->bqHpf0.process(x);
    auto y =
        this->transformer.process(xTrans * this->drive_lin) / this->drive_lin;
    this->template updatePeakLevel<1>(y);
    return y;
  }
  virtual void update() noexcept override {
    this->drive_lin = NtFx::invDb(this->drive_db);
    this->transformer.update();
    this->bqHpf0.update();
  }
  virtual void reset(float fs) noexcept override {
    this->fs = fs;
    this->transformer.reset(fs);
    this->bqHpf0.settings.fc_hz = 40;
    this->bqHpf0.settings.q     = 1.1;
    this->bqHpf0.settings.shape = NtFx::Biquad::Shape::hpf;
    this->bqHpf0.reset(fs);
    this->update();
  }
};
