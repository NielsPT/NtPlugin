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
#include "lib/Comp.h"
#include "lib/Delay.h"
#include "lib/DynamicFilter.h"
#include "lib/Plugin.h"
#include <cstddef>

enum Mode { wide, shelf, bell };

struct ntDeEsser final : public NtFx::Plugin {
  NtFx::Delay::Short<10.0> dl;
  NtFx::Comp::PeakSideChainLin sc;
  NtFx::Biquad::EqBand scBpf;
  NtFx::DynamicFilter::ShelfFixedZeros shelf;
  NtFx::Biquad::EqBand bpf;
  signal_t q { 1.0 };
  signal_t red_p { 100 };
  signal_t red_lin { 100 };
  signal_t fc_hz { 4e3 };
  Mode mode { Mode::bell };
  bool bypassEnable { false };
  bool scListenEnable { false };
  ntDeEsser() {
    this->primaryKnobs = {
      {
          .p_val    = &this->fc_hz,
          .name     = "Frequency",
          .suffix   = " Hz",
          .minVal   = 20,
          .maxVal   = 20e3,
          .logScale = true,
      },
      {
          .p_val  = &this->sc.settings.thresh_db,
          .name   = "Threshold",
          .suffix = " dB",
          .minVal = -60,
          .maxVal = 0,
      },
      {
          .p_val  = &this->red_p,
          .name   = "Reduction",
          .suffix = " %",
          .minVal = 0,
          .maxVal = 100,
      },
    };
    this->secondaryKnobs = {
      {
          .p_val  = &this->q,
          .name   = "Q",
          .minVal = 0.4,
          .maxVal = 2,
      },
      {
          .p_val    = &this->dl.t_ms,
          .name     = "Lookahead",
          .suffix   = " ms",
          .minVal   = 0,
          .maxVal   = 10,
          .midPoint = 1,
      },
      {
          .p_val    = &this->sc.settings.tRel_ms,
          .name     = "Release",
          .suffix   = " ms",
          .minVal   = 1.0,
          .maxVal   = 100.0,
          .midPoint = 10.0,
      },
    };
    this->radioButtons = {
      {
          .p_val   = (int*)&this->mode,
          .name    = "Mode",
          .options = { "Wideband", "Shelf", "Bell" },
      },
    };
    this->toggles = {
      { .p_val = &this->scListenEnable, .name = "SC Listen" },
      { .p_val = &this->sc.settings.linkEnable, .name = "Link" },
      { .p_val = &this->bypassEnable, .name = "Bypass" },
    };
    this->meters.push_back({ .name = "GR", .invert = true });
    this->sc.settings.knee_db    = 3;
    this->sc.settings.linkEnable = true;
    this->sc.settings.tRel_ms    = 30;
    this->dl.t_ms                = 1;
    this->bpf.settings.shape     = NtFx::Biquad::Shape::bpf;
    this->scBpf.settings.shape   = NtFx::Biquad::Shape::bpf;
    this->updateDefaults();
  }

  Audio process(Audio x) noexcept override {
    auto yDl = this->dl.process(x);
    this->template updatePeakLevel<0>(x);
    if (this->bypassEnable) {
      this->template updatePeakLevel<1>(x);
      return x;
    }

    Audio yScFlt, gr, y;
    yScFlt = this->scBpf.process(x);
    gr     = sc.process(yScFlt);
    switch (this->mode) {
    case Mode::bell:
      y = yDl - this->bpf.process(yDl) * (Audio(1) - gr);
      break;
    case Mode::shelf:
      this->shelf.gain_lin = gr.absMin();
      y                    = this->shelf.process(yDl);
      break;
    case Mode::wide:
      y = yDl * gr;
      break;
    }

    this->template updatePeakLevel<1>(y);
    this->template updatePeakLevel<2, true>(gr);
    if (this->scListenEnable) { return yScFlt; }
    return y;
  }

  void update() noexcept override {
    this->red_lin                  = this->red_p / 100;
    this->sc.settings.ratio        = 20 * this->red_lin;
    this->sc.settings.tPeakHold_ms = this->dl.t_ms;
    this->sc.settings.tAtt_ms      = gcem::max(this->dl.t_ms, signal_t(0.1));
    this->sc.update();

    this->shelf.fc_hz = this->fc_hz;
    this->shelf.update();

    this->bpf.settings.fc_hz = this->fc_hz;
    this->bpf.settings.q     = this->q;
    this->bpf.update();

    this->scBpf.settings.fc_hz = this->fc_hz;
    this->scBpf.settings.q     = this->q;
    this->scBpf.update();

    this->dl.update();
    this->latency = size_t(this->dl._n);
  }

  void reset(signal_t fs) noexcept override {
    this->sc.reset(fs);
    this->shelf.reset(fs);
    this->scBpf.reset(fs);
    this->bpf.reset(fs);
    this->dl.reset(fs);
    this->_fs = fs;
    this->update();
  }
};
