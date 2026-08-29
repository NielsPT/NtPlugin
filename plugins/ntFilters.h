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

#include "lib/Audio.h"
#include "lib/Biquad.h"
#include "lib/FirstOrder.h"
#include "lib/Plugin.h"

enum Order : int {
  first,
  second,
  third,
  fourth,
};

constexpr signal_t butterworthFourthOrderQ0 = 0.5411961f;
constexpr signal_t butterworthFourthOrderQ1 = 1.3065630f;

enum CascadeIdx { bqHpf0, bqHpf1, bqLpf0, bqLpf1 };

struct ntFilters final : public NtFx::Plugin {
  signal_t fHpf = 20;
  signal_t fLpf = 20000;
  signal_t qHpf = 0.707;
  signal_t qLpf = 0.707;
  Order orderHpf;
  Order orderLpf;
  bool enableHpf = true;
  bool enableLpf = true;
  NtFx::FirstOrder::StereoFilter<NtFx::FirstOrder::Shape::hpf> firstOrderHpf;
  NtFx::FirstOrder::StereoFilter<NtFx::FirstOrder::Shape::lpfZero>
      firstOrderLpf;
  NtFx::Biquad::Cascade<4> cascade;
  ntFilters() {

    this->primaryKnobs = {
      {
          .p_val    = &this->fHpf,
          .name     = "HPF",
          .suffix   = " Hz",
          .minVal   = 20,
          .maxVal   = 20e3,
          .midPoint = 2e3,
      },
      {
          .p_val  = &this->qHpf,
          .name   = "Q HPF",
          .suffix = "",
          .minVal = 0.1,
          .maxVal = 2,
      },
      {
          .p_val    = &this->fLpf,
          .name     = "LPF",
          .suffix   = " Hz",
          .minVal   = 20,
          .maxVal   = 20e3,
          .midPoint = 2e3,
      },
      {
          .p_val  = &this->qLpf,
          .name   = "Q LPF",
          .suffix = "",
          .minVal = 0.1,
          .maxVal = 2,
      },
    };
    this->radioButtons = {
      {
          (int*)&this->orderHpf,
          "HPF Order",
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
          "LPF Order",
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
      { &this->enableHpf, "HPF on" },
      { &this->enableLpf, "LPF on" },
    };
    this->meters = {
      { .name = "IN", .addRms = true },
      { .name = "OUT", .hasScale = true, .addRms = true },
    };
    this->updateDefaults();
  }

  Audio process(Audio x) noexcept override {
    auto y1stHpf = x;
    if (this->enableHpf && (this->orderHpf + 1) % 2) {
      y1stHpf = this->firstOrderHpf.process(x);
    }
    auto y1stLpf = y1stHpf;
    if (this->enableLpf && (this->orderLpf + 1) % 2) {
      y1stLpf = this->firstOrderLpf.process(y1stHpf);
    }
    auto y = this->cascade.process(y1stLpf);
    this->updatePeakLevel(0, x);
    this->updatePeakLevel(1, y);
    return y;
  }

  void update() noexcept override {
    this->activateParameter("Q HPF");
    this->activateParameter("Q LPF");
    this->firstOrderHpf.fc_hz = fHpf;
    if (this->orderHpf == Order::fourth) {
      auto qScale                          = this->qHpf / signal_t(0.707);
      this->cascade.settings[bqHpf0].q     = butterworthFourthOrderQ0 * qScale;
      this->cascade.settings[bqHpf1].q     = butterworthFourthOrderQ1 * qScale;
      this->cascade.settings[bqHpf0].shape = NtFx::Biquad::Shape::hpf;
      this->cascade.settings[bqHpf1].shape = NtFx::Biquad::Shape::hpf;
    } else if (this->orderHpf == Order::first || !this->enableHpf) {
      this->cascade.settings[bqHpf0].shape = NtFx::Biquad::Shape::none;
      this->cascade.settings[bqHpf1].shape = NtFx::Biquad::Shape::none;
      this->deactivateParameter("Q HPF");
    } else {
      this->cascade.settings[bqHpf0].q     = this->qHpf;
      this->cascade.settings[bqHpf0].shape = NtFx::Biquad::Shape::hpf;
      this->cascade.settings[bqHpf1].shape = NtFx::Biquad::Shape::none;
    }
    this->cascade.settings[bqHpf0].fc_hz = fHpf;
    this->cascade.settings[bqHpf1].fc_hz = fHpf;
    this->firstOrderLpf.fc_hz            = fLpf;
    if (this->orderLpf == Order::fourth) {
      auto qScale                          = this->qLpf / signal_t(0.707);
      this->cascade.settings[bqLpf0].q     = butterworthFourthOrderQ0 * qScale;
      this->cascade.settings[bqLpf1].q     = butterworthFourthOrderQ1 * qScale;
      this->cascade.settings[bqLpf0].shape = NtFx::Biquad::Shape::lpf;
      this->cascade.settings[bqLpf1].shape = NtFx::Biquad::Shape::lpf;
    } else if (this->orderLpf == Order::first || !this->enableLpf) {
      this->cascade.settings[bqLpf0].shape = NtFx::Biquad::Shape::none;
      this->cascade.settings[bqLpf1].shape = NtFx::Biquad::Shape::none;
      this->deactivateParameter("Q LPF");
    } else {
      this->cascade.settings[bqLpf0].q     = this->qLpf;
      this->cascade.settings[bqLpf0].shape = NtFx::Biquad::Shape::lpf;
      this->cascade.settings[bqLpf1].shape = NtFx::Biquad::Shape::none;
    }
    if (this->orderLpf == Order::third) {
      this->cascade.settings[bqLpf0].q = this->qLpf / signal_t(0.707);
    }
    if (this->orderHpf == Order::third) {
      this->cascade.settings[bqHpf0].q = this->qHpf / signal_t(0.707);
    }
    this->cascade.settings[bqLpf0].fc_hz = fLpf;
    this->cascade.settings[bqLpf1].fc_hz = fLpf;
    this->firstOrderHpf.update();
    this->firstOrderLpf.update();
    this->cascade.update();
    this->uiNeedsUpdate = true;
  }

  void reset(signal_t fs) noexcept override {
    this->_fs = fs;
    this->firstOrderHpf.reset(fs);
    this->firstOrderLpf.reset(fs);
    this->cascade.reset(fs);
    this->update();
  }
};
