#pragma once

#include "lib/Audio.h"
#include "lib/Biquad.h"
#include "lib/Comp.h"
#include "lib/DynamicFilter.h"
#include "lib/Plugin.h"

struct ntDeEsser : public NtFx::NtPlugin {
  NtFx::Comp::PeakSideChainLinear sc;
  NtFx::Biquad::EqBand scFlt;
  NtFx::DynamicFilter::Shelf flt;
  NtFx::Comp::ScSettings scSettings;
  signal_t q { 0.6 };
  signal_t fc_hz { 4e3 };
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
    };
    this->secondaryKnobs = {
      {
          .p_val  = &this->q,
          .name   = "Q",
          .minVal = 0.5,
          .maxVal = 1,
      },
      // {
      //     .p_val    = &this->scSettings.ratio_db,
      //     .name     = "Ratio",
      //     .suffix   = "",
      //     .minVal   = 1.0,
      //     .maxVal   = 20.0,
      //     .midPoint = 2.0,
      // },
      // {
      //     .p_val  = &this->scSettings.tAtt_ms,
      //     .name   = "Attack",
      //     .suffix = " ms",
      //     .minVal = 0.01,
      //     .maxVal = 50.0,
      // },
      {
          .p_val  = &this->scSettings.tRel_ms,
          .name   = "Release",
          .suffix = " ms",
          .minVal = 1.0,
          .maxVal = 100.0,
      },
    };
    this->toggles = {
      { .p_val = &this->bypassEnable, .name = "Bypass" },
      { .p_val = &this->scListenEnable, .name = "SC_Listen" },
    };
    this->meters.push_back({ .name = "GR", .invert = true });
    this->scSettings.ratio_db = 10;
    this->scSettings.knee_db  = 3;
    this->scSettings.tAtt_ms  = 0.01;
    this->updateDefaults();
  }

  Audio process(Audio x) noexcept override {
    this->template updatePeakLevel<0>(x);
    if (this->bypassEnable) {
      this->template updatePeakLevel<1>(x);
      return x;
    }
    auto yScFlt        = scFlt.process(x);
    auto ySc           = sc.process(yScFlt);
    this->flt.gain_lin = ySc.absMin();
    auto y             = flt.process(x);
    this->template updatePeakLevel<1>(y);
    this->template updatePeakLevel<2, true>(ySc);
    if (this->scListenEnable) { return yScFlt; }
    return y;
  }

  void update() noexcept override {
    this->scFlt.settings.shape = NtFx::Biquad::Shape::hpf;
    this->scFlt.settings.q     = this->q;
    this->flt.q1               = this->q;
    this->flt.q2               = this->q;
    this->scFlt.settings.fc_hz = this->fc_hz;
    this->flt.fc_hz            = this->fc_hz;
    // this->scSettings.tAtt_ms   = signal_t(1e3) / this->fc_hz;
    // this->scSettings.tRel_ms   = this->scSettings.tAtt_ms * 10;
    this->sc.update();
    this->scFlt.update();
    // Don't update flt. It's done in process.
  }

  void reset(float fs) noexcept override {
    this->sc.reset(fs);
    this->scFlt.reset(fs);
    this->flt.reset(fs);
    this->fs = fs;
    this->update();
  }
};
