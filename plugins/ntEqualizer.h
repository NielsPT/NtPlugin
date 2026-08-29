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
#include "lib/FirstOrder.h"
#include "lib/Plugin.h"
#include <array>
#include <string>
#include <vector>

constexpr int nHpf = 2;
constexpr int nLpf = 2;

enum Order : int { none, first, second, third, fourth };
enum Bands : int { lo, loMid, hiMid, hi, n };
enum Filters : int { fLo, fLoMid, fHiMid, fHi, fHpf1, fHpf2, fLpf1, fLpf2, fN };
const std::array<std::string, Bands::n> names = {
  "Low", "Low Mid", "High Mid", "High"
};
const std::vector<std::string> hpfLpfOptions {
  "Off", "1st Order", "2nd Order", "3rd Order", "4th Order"
};
const std::vector<std::string> bandOptions {
  "Bell",
  "High Shelf",
  "Low Shelf",
  "Notch",
  "Highpass",
  "Lowpass",
  "Allpass",
  "Bandpass",
  "Off",
};

struct ntEqualizer final : public NtFx::Plugin {
  NtFx::Biquad::Cascade<fN> cascade;
  NtFx::FirstOrder::StereoFilter<NtFx::FirstOrder::Shape::hpf> firstOrderHpf;
  NtFx::FirstOrder::StereoFilter<NtFx::FirstOrder::Shape::lpf> firstOrderLpf;
  signal_t fHpf_hz { 20 };
  signal_t fLpf_hz { 20e3 };
  signal_t qHpf { 0.707 };
  signal_t qLpf { 0.707 };
  Order orderHpf { Order::none };
  Order orderLpf { Order::none };
  bool bypassEnable { false };

  ntEqualizer() {
    this->knobGroups.push_back({
        "HPF",
        {
            { &this->fHpf_hz, "Freq", " Hz", 20, 20e3, 2e3 },
            { &this->qHpf, "Q", "", 0.5, 10 },
        },
    });
    for (size_t i = 0; i < Bands::n; i++) {
      this->knobGroups.push_back({
          names[i],
          {
              { &this->cascade.settings[i].gain_db, "Gain", " dB", -20, 20 },
              { &this->cascade.settings[i].fc_hz,
                  "Freq",
                  " Hz",
                  20,
                  20e3,
                  2e3 },
              { &this->cascade.settings[i].q, "Q", "", 0.5, 10.0, 1.0 },
          },
      });
    }
    this->knobGroups.push_back({
        "LPF",
        {
            { &this->fLpf_hz, "Freq", " Hz", 20, 20e3, 2e3 },
            { &this->qLpf, "Q", "", 0.5, 10 },
        },
    });
    this->dropdowns.push_back(
        { (int*)&this->orderHpf, "HPF Order", hpfLpfOptions, true });
    for (size_t i = 0; i < Bands::n; i++) {
      this->dropdowns.push_back({
          (int*)&this->cascade.settings[i].shape,
          names[i],
          bandOptions,
          true,
      });
    }
    this->dropdowns.push_back(
        { (int*)&this->orderLpf, "LPF Order", hpfLpfOptions, true });
    this->_resetDefaultValues();
    this->updateDefaults();
  }

  Audio process(Audio x) noexcept override {
    this->updatePeakLevel(0, x);
    if (this->bypassEnable) {
      this->updatePeakLevel(1, x);
      return x;
    }
    auto yCascade       = cascade.process(x);
    auto yFirstOrderHpf = yCascade;
    if (this->orderHpf % 2) {
      yFirstOrderHpf = this->firstOrderHpf.process(yCascade);
    }
    auto y = yFirstOrderHpf;
    if (this->orderLpf % 2) { y = this->firstOrderLpf.process(yFirstOrderHpf); }
    this->updatePeakLevel(1, y);
    return y;
  }

  void update() noexcept override {
    this->_updateHpfLpf();
    this->firstOrderHpf.update();
    this->firstOrderLpf.update();
    this->cascade.update();
  }

  void reset(signal_t fs) noexcept override {
    this->_fs = fs;
    this->firstOrderHpf.reset(fs);
    this->firstOrderLpf.reset(fs);
    this->cascade.reset(fs);
    this->update();
  }

  void _resetDefaultValues() {
    this->cascade.settings[fLo].shape    = NtFx::Biquad::Shape::loShelf;
    this->cascade.settings[fLo].fc_hz    = 100;
    this->cascade.settings[fLoMid].fc_hz = 200;
    this->cascade.settings[fHiMid].fc_hz = 2e3;
    this->cascade.settings[fHi].shape    = NtFx::Biquad::Shape::hiShelf;
    this->cascade.settings[fHi].fc_hz    = 12e3;
    this->uiSpec.groupWidth              = 120;
  }

  void _updateHpfLpf() {
    if (this->orderHpf == Order::fourth) {
      this->cascade.settings[fHpf1].shape = NtFx::Biquad::Shape::hpf;
      this->cascade.settings[fHpf2].shape = NtFx::Biquad::Shape::hpf;
    } else if (this->orderHpf == Order::first
        || this->orderHpf == Order::none) {
      this->cascade.settings[fHpf1].shape = NtFx::Biquad::Shape::none;
      this->cascade.settings[fHpf2].shape = NtFx::Biquad::Shape::none;
    } else {
      this->cascade.settings[fHpf1].shape = NtFx::Biquad::Shape::hpf;
      this->cascade.settings[fHpf2].shape = NtFx::Biquad::Shape::none;
    }
    if (this->orderLpf == Order::fourth) {
      this->cascade.settings[fLpf1].shape = NtFx::Biquad::Shape::lpf;
      this->cascade.settings[fLpf2].shape = NtFx::Biquad::Shape::lpf;
    } else if (this->orderLpf == Order::first
        || this->orderLpf == Order::none) {
      this->cascade.settings[fLpf1].shape = NtFx::Biquad::Shape::none;
      this->cascade.settings[fLpf2].shape = NtFx::Biquad::Shape::none;
    } else {
      this->cascade.settings[fLpf1].shape = NtFx::Biquad::Shape::lpf;
      this->cascade.settings[fLpf2].shape = NtFx::Biquad::Shape::none;
    }
    this->cascade.settings[fHpf1].fc_hz = this->fHpf_hz;
    this->cascade.settings[fHpf2].fc_hz = this->fHpf_hz;
    this->firstOrderHpf.fc_hz           = this->fHpf_hz;
    this->cascade.settings[fLpf1].fc_hz = this->fLpf_hz;
    this->cascade.settings[fLpf2].fc_hz = this->fLpf_hz;
    this->firstOrderLpf.fc_hz           = this->fLpf_hz;
    this->cascade.settings[fHpf1].q     = this->qHpf;
    this->cascade.settings[fLpf1].q     = this->qLpf;
  }
};
