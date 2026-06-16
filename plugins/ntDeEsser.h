#pragma once

#include "lib/Audio.h"
#include "lib/Biquad.h"
#include "lib/Comp.h"
#include "lib/DelayLine.h"
#include "lib/DynamicFilter.h"
#include "lib/Plugin.h"
#include <algorithm>

// 10 ms lookahead max
constexpr int dlLookaheadLen = 192 * 8 * 10;
enum Mode { wide, shelf, bell };

struct ntDeEsser : public NtFx::NtPlugin {
  NtFx::Comp::PeakSideChainLinear sc;
  NtFx::Biquad::EqBand scHpf;
  NtFx::Biquad::EqBand scBpf;
  NtFx::DynamicFilter::Shelf shelf;
  NtFx::Biquad::EqBand bpf;
  NtFx::Comp::ScSettings scSettings;
  NtFx::Delay::ShortDelayLine<dlLookaheadLen> dl;
  signal_t q { 0.6 };
  signal_t fc_hz { 4e3 };
  Mode mode { Mode::bell };
  bool lookaheadEnable { true };
  bool bypassEnable { false };
  bool scListenEnable { false };
  ntDeEsser() : sc(scSettings) {
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
          .p_val  = &this->scSettings.thresh_db,
          .name   = "Threshold",
          .suffix = " dB",
          .minVal = -60,
          .maxVal = 0,
      },
      // {
      //     .p_val    = &this->scSettings.ratio,
      //     .name     = "Ratio",
      //     .suffix   = "",
      //     .minVal   = 1.0,
      //     .maxVal   = 20.0,
      //     .midPoint = 2.0,
      // },
    };
    this->secondaryKnobs = {
      {
          .p_val  = &this->q,
          .name   = "Q",
          .minVal = 0.4,
          .maxVal = 2,
      },
      {
          .p_val  = &this->dl.t_ms,
          .name   = "Lookahead",
          .suffix = " ms",
          .minVal = 0,
          .maxVal = 10,
      },

      {
          .p_val  = &this->scSettings.tRel_ms,
          .name   = "Release",
          .suffix = " ms",
          .minVal = 1.0,
          .maxVal = 100.0,
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
      // { .p_val = &this->lookaheadEnable, .name = "Lookahead_enable" },
      { .p_val = &this->scListenEnable, .name = "SC_Listen" },
      { .p_val = &this->scSettings.linkEnable, .name = "Link" },
      { .p_val = &this->bypassEnable, .name = "Bypass" },
    };
    this->meters.push_back({ .name = "GR", .invert = true });
    this->scSettings.ratio   = 20;
    this->scSettings.knee_db = 3;
    this->dl.t_ms            = 5;
    this->scSettings.tRel_ms = 30;
    // TODO: only one bpf.
    this->bpf.settings.shape   = NtFx::Biquad::Shape::bpf;
    this->scBpf.settings.shape = NtFx::Biquad::Shape::bpf;
    this->scHpf.settings.shape = NtFx::Biquad::Shape::hpf;
    // this->shelf._flt.settings.shape = NtFx::Biquad::Shape::hiShelf;
    this->updateDefaults();
  }

  Audio process(Audio x) noexcept override {
    auto yDl = this->dl.process(x);
    this->template updatePeakLevel<0>(x);
    if (this->bypassEnable) {
      this->template updatePeakLevel<1>(x);
      return x;
    }
    auto xFlt = x;
    if (this->lookaheadEnable) { xFlt = yDl; }
    Audio yScFlt, ySc, y;
    if (this->mode == Mode::wide) {
      yScFlt = this->scBpf.process(x / this->q);
      ySc    = sc.process(yScFlt);
      y      = xFlt * ySc;
    } else if (this->mode == Mode::shelf) {
      yScFlt               = scHpf.process(x);
      ySc                  = sc.process(yScFlt);
      this->shelf.gain_lin = ySc.absMin();
      y                    = shelf.process(xFlt);
    } else if (this->mode == Mode::bell) {
      yScFlt = this->scBpf.process(x / this->q);
      ySc    = sc.process(yScFlt);
      y      = xFlt - this->bpf.process(xFlt) * (Audio(1) - ySc) / this->q;
    }
    this->template updatePeakLevel<1>(y);
    this->template updatePeakLevel<2, true>(ySc);
    if (this->scListenEnable) { return yScFlt; }
    return y;
  }

  void update() noexcept override {
    this->scSettings.tPeakHold_ms = this->dl.t_ms;
    this->scSettings.tAtt_ms      = gcem::max(this->dl.t_ms, 0.1);
    this->sc.update();

    this->scHpf.settings.shape = NtFx::Biquad::Shape::hpf;
    this->scHpf.settings.q     = this->q;
    this->scHpf.settings.fc_hz = this->fc_hz;
    this->scHpf.update();

    this->shelf.q1    = this->q;
    this->shelf.q2    = this->q;
    this->shelf.fc_hz = this->fc_hz;
    this->shelf.update();

    this->bpf.settings.fc_hz = this->fc_hz;
    this->bpf.settings.q     = this->q;
    this->bpf.update();

    this->scBpf.settings.fc_hz = this->fc_hz;
    this->scBpf.settings.q     = this->q;
    this->scBpf.update();

    this->dl.update();
    // this->scSettings.tAtt_ms   = signal_t(1e3) / this->fc_hz;
    // this->scSettings.tRel_ms   = this->scSettings.tAtt_ms * 10;
    if (this->lookaheadEnable) {
      this->latency = this->dl.n;
    } else {
      this->latency = 0;
    }
  }

  void reset(float fs) noexcept override {
    this->sc.reset(fs);
    this->scHpf.reset(fs);
    this->shelf.reset(fs);
    this->scBpf.reset(fs);
    this->bpf.reset(fs);
    this->fs = fs;
    this->update();
  }
};
