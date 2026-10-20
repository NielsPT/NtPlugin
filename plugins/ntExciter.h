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

#include "lib/Audio.h"
#include "lib/Biquad.h"
#include "lib/Plugin.h"
#include "lib/SoftClip.h"
#include "lib/utils.h"

struct ntExciter final : public NtFx::Plugin {
  NtFx::Biquad::EqBand flt1;
  NtFx::Biquad::EqBand flt2;
  NtFx::Clip::Soft5 clip;
  signal_t gain_db { 0 };
  signal_t gain_lin { 1 };
  bool flt2Enable { true };
  bool bypassEnable { false };

  ntExciter() {
    this->primaryKnobs = {
      { &this->flt1.settings.fc_hz, "Frequency", " Hz", 20, 20e3, 2e3 },
      { &this->flt1.settings.q, "Q", "", 0.5, 10, 2 },
      { &this->clip.gain_db, "Drive", " dB", 0, 48 },
      { &this->gain_db, "Gain", " dB", 0, 18 },
    };
    this->toggles = {
      { &this->flt2Enable, "2nd Filter" },
      { .p_val = &this->bypassEnable, .name = "Bypass" },
    };
    this->flt1.settings.shape = NtFx::Biquad::Shape::bpf;
    this->flt2.settings.shape = NtFx::Biquad::Shape::bpf;
    this->flt1.settings.q     = 2;
    this->updateDefaults();
  }

  Audio process(Audio x) noexcept override {
    this->updatePeakLevel(0, x);
    if (this->bypassEnable) {
      this->updatePeakLevel(1, x);
      return x;
    }
    auto yFlt1 = this->flt1.process(x);
    auto yClip = this->clip.process(yFlt1);
    auto yFlt2 = yClip;
    if (this->flt2Enable) { yFlt2 = this->flt2.process(yClip); }
    Audio y = yFlt2 * this->gain_lin + x;
    this->updatePeakLevel(1, y);
    return y;
  }

  void update() noexcept override {
    this->flt2.settings.fc_hz = this->flt1.settings.fc_hz * 3;
    this->flt2.settings.q     = this->flt1.settings.q;
    this->gain_lin            = NtFx::invDb(this->gain_db) - 1;
    this->flt1.update();
    this->flt2.update();
    this->clip.update();
  }

  void reset(signal_t fs) noexcept override {
    this->_fs = fs;
    this->flt1.reset(fs);
    this->flt2.reset(fs);
    this->clip.reset(fs);
    this->update();
  }
};
