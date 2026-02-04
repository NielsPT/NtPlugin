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

#include "lib/Biquad.h"
#include "lib/FirstOrder.h"
#include "lib/Plugin.h"
#include "lib/Stereo.h"

enum Order : int {
  first,
  second,
  third,
  fourth,
};

template <typename signal_t>
struct ntFilters : public NtFx::NtPlugin<signal_t> {
  signal_t fHpf = 20;
  signal_t fLpf = 20000;
  signal_t qHpf = 0.707;
  signal_t qLpf = 0.707;
  Order orderHpf;
  Order orderLpf;
  bool enableHpf = true;
  bool enableLpf = true;
  NtFx::FirstOrder::StereoFilter<signal_t, NtFx::FirstOrder::Shape::hpf>
      firstOrderHpf;
  NtFx::FirstOrder::StereoFilter<signal_t, NtFx::FirstOrder::Shape::lpfZero>
      firstOrderLpf;
  NtFx::Biquad::EqBand<signal_t> bqLpf0;
  NtFx::Biquad::EqBand<signal_t> bqLpf1;
  NtFx::Biquad::EqBand<signal_t> bqHpf0;
  NtFx::Biquad::EqBand<signal_t> bqHpf1;
  ntFilters() {
    this->uiSpec.defaultWindowWidth = 800;

    this->primaryKnobs = {
      { &this->fHpf, "HPF", " Hz", 20, 20e3, 2e3 },
      { &this->qHpf, "Q_HPF", "", 0.1, 2 },
      { &this->fLpf, "LPF", " Hz", 20, 20e3, 2e3 },
      { &this->qLpf, "Q_LPF", "", 0.1, 2 },
    };
    this->dropdowns = {
      {
          (int*)&this->orderHpf,
          "HPF_Order",
          {
              "first",
              "second",
              "third",
              "fourth",
          },
          0,
      },
      {
          (int*)&this->orderLpf,
          "LPF_Order",
          {
              "first",
              "second",
              "third",
              "fourth",
          },
          0,
      },
    };
    this->toggles = {
      { &this->enableHpf, "HPF_on" },
      { &this->enableLpf, "LPF_on" },
    };
    this->uiSpec.meters = { { "IN" }, { "OUT", .hasScale = true } };
    this->updateDefaults();
  }

  NtFx::Stereo<signal_t> process(NtFx::Stereo<signal_t> x) noexcept override {
    auto xBqHpf0 = x;
    if ((this->orderHpf + 1) % 2) { xBqHpf0 = this->firstOrderHpf.process(x); }
    auto yBqHpf0 = this->bqHpf0.process(xBqHpf0);
    auto yBqHpf1 = this->bqHpf1.process(yBqHpf0);

    auto xLpf = yBqHpf1;
    if (!this->enableHpf) { xLpf = x; }
    auto xBqLpf0 = xLpf;
    if ((this->orderLpf + 1) % 2) {
      xBqLpf0 = this->firstOrderLpf.process(xLpf);
    }
    auto yBqLpf0 = this->bqLpf0.process(xBqLpf0);
    auto yBqLpf1 = this->bqLpf1.process(yBqLpf0);

    auto y = yBqLpf1;
    if (!this->enableLpf) { y = xLpf; }
    this->template updatePeakLevel<0>(x);
    this->template updatePeakLevel<1>(y);
    return y;
  }

  void update() noexcept override {
    this->primaryKnobs[1].isActive = true;
    this->primaryKnobs[3].isActive = true;
    this->firstOrderHpf.setFc(fHpf);
    this->firstOrderHpf.update();
    if (this->orderHpf == Order::fourth) {
      this->bqHpf0.settings.q     = gcem::sqrt(this->qHpf);
      this->bqHpf1.settings.q     = gcem::sqrt(this->qHpf);
      this->bqHpf0.settings.shape = NtFx::Biquad::Shape::hpf;
      this->bqHpf1.settings.shape = NtFx::Biquad::Shape::hpf;
    } else if (this->orderHpf == Order::first) {
      this->bqHpf0.settings.shape    = NtFx::Biquad::Shape::none;
      this->bqHpf1.settings.shape    = NtFx::Biquad::Shape::none;
      this->primaryKnobs[1].isActive = false;
    } else {
      this->bqHpf0.settings.q     = this->qHpf;
      this->bqHpf0.settings.shape = NtFx::Biquad::Shape::hpf;
      this->bqHpf1.settings.shape = NtFx::Biquad::Shape::none;
    }
    this->bqHpf0.settings.fc_hz = fHpf;
    this->bqHpf1.settings.fc_hz = fHpf;
    this->bqHpf0.update();
    this->bqHpf1.update();

    this->firstOrderLpf.setFc(fLpf);
    this->firstOrderLpf.update();
    if (this->orderLpf == Order::fourth) {
      this->bqLpf0.settings.q     = gcem::sqrt(this->qLpf);
      this->bqLpf1.settings.q     = gcem::sqrt(this->qLpf);
      this->bqLpf0.settings.shape = NtFx::Biquad::Shape::lpf;
      this->bqLpf1.settings.shape = NtFx::Biquad::Shape::lpf;
    } else if (this->orderLpf == Order::first) {
      this->bqLpf0.settings.shape    = NtFx::Biquad::Shape::none;
      this->bqLpf1.settings.shape    = NtFx::Biquad::Shape::none;
      this->primaryKnobs[3].isActive = false;
    } else {
      this->bqLpf0.settings.q     = this->qLpf;
      this->bqLpf0.settings.shape = NtFx::Biquad::Shape::lpf;
      this->bqLpf1.settings.shape = NtFx::Biquad::Shape::none;
    }
    this->bqLpf0.settings.fc_hz = fLpf;
    this->bqLpf1.settings.fc_hz = fLpf;
    this->bqLpf0.update();
    this->bqLpf1.update();
    this->uiNeedsUpdate = true;
  }

  void reset(float fs) noexcept override {
    this->fs = fs;
    this->firstOrderHpf.reset(fs);
    this->firstOrderLpf.reset(fs);
    this->bqHpf0.reset(fs);
    this->bqHpf1.reset(fs);
    this->bqLpf0.reset(fs);
    this->bqLpf1.reset(fs);
    this->update();
  }
};
