#pragma once

#include "lib/Audio.h"
#include "lib/Biquad.h"
#include "lib/Comp.h"
#include "lib/Plugin.h"
#include "lib/utils.h"
#include <array>
#include <cstddef>

enum Bands { lf, loMid, hiMid, hf, n };

struct ntDynamicEq : public NtFx::NtPlugin {
  const std::array<std::string, Bands::n> bandNames {
    "Low", "Low_Mid", "High_Mid", "High"
  };
  std::array<NtFx::Biquad::EqBand, Bands::n> bands;
  std::array<NtFx::Comp::ScSettings, Bands::n> scSettings;
  std::array<NtFx::Comp::PeakSideChainLinear, Bands::n> scs;
  std::array<signal_t, Bands::n> gain_lin;
  signal_t attScale { 0.25 };
  signal_t relScale { 4 };
  bool bypassEnable { false };

  ntDynamicEq()
      : scs({
            scSettings[0],
            scSettings[1],
            scSettings[2],
            scSettings[3],
        }) {
    for (size_t i = 0; i < Bands::n; i++) {
      this->primaryKnobs.push_back({
          .p_val  = &this->bands[i].settings.gain_db,
          .name   = bandNames[i] + "_Gain",
          .suffix = " dB",
          .minVal = -20,
          .maxVal = 20,
      });
    }
    for (size_t i = 0; i < Bands::n; i++) {
      this->primaryKnobs.push_back({
          .p_val    = &this->bands[i].settings.fc_hz,
          .name     = bandNames[i] + "_Freq",
          .suffix   = " Hz",
          .minVal   = 20,
          .maxVal   = 20e3,
          .midPoint = 2e3,
      });
    }
    for (size_t i = 0; i < Bands::n; i++) {
      this->primaryKnobs.push_back({
          .p_val    = &this->bands[i].settings.q,
          .name     = bandNames[i] + "_Q",
          .minVal   = 0.5,
          .maxVal   = 10,
          .midPoint = 1,
      });
    }
    for (size_t i = 0; i < Bands::n; i++) {
      this->primaryKnobs.push_back({
          .p_val  = &this->scSettings[i].thresh_db,
          .name   = bandNames[i] + "_Thresh",
          .suffix = " dB",
          .minVal = -60,
          .maxVal = 0,
      });
    }
    for (size_t i = 0; i < Bands::n; i++) {
      this->primaryKnobs.push_back({
          .p_val    = &this->scSettings[i].ratio,
          .name     = bandNames[i] + "_Ratio",
          .minVal   = 1,
          .maxVal   = 20,
          .midPoint = 2,
      });
    }
    // for (size_t i = 0; i < Bands::n; i++) {
    //   this->primaryKnobs.push_back({
    //       .p_val    = &this->scSettings[i].tAtt_ms,
    //       .name     = bandNames[i] + "_Attack",
    //       .suffix   = " ms",
    //       .minVal   = 0.01,
    //       .maxVal   = 50.0,
    //       .midPoint = 5,
    //   });
    // }
    // for (size_t i = 0; i < Bands::n; i++) {
    //   this->primaryKnobs.push_back({
    //       .p_val    = &this->scSettings[i].tRel_ms,
    //       .name     = this->bandNames[i] + "_Release",
    //       .suffix   = " ms",
    //       .minVal   = 10.0,
    //       .maxVal   = 1000.0,
    //       .midPoint = 100.0,
    //   });
    // }
    this->secondaryKnobs = {
      {
          .p_val    = &this->attScale,
          .name     = "Attack",
          .minVal   = 1.0 / 16.0,
          .maxVal   = 4.0,
          .midPoint = 1.0 / 4.0,
      },
      {
          .p_val    = &this->relScale,
          .name     = "Release",
          .minVal   = 1.0,
          .maxVal   = 16.0,
          .midPoint = 4.0,
      },
    };
    this->toggles = {
      { .p_val = &this->bypassEnable, .name = "Bypass" },
    };
    for (size_t i = 0; i < Bands::n; i++) {
      this->meters.push_back({
          .name   = this->bandNames[i],
          .invert = true,
      });
    }
    this->uiSpec.maxColumns = Bands::n;
    this->uiSpec.maxRows    = 8;
    this->uiSpec.knobHeight = 140;
    // this->uiSpec.maxColumns = 7;
    // this->uiSpec.maxRows    = Bands::n;
    this->bands[Bands::lf].settings.fc_hz    = 100;
    this->bands[Bands::loMid].settings.fc_hz = 500;
    this->bands[Bands::hiMid].settings.fc_hz = 2e3;
    this->bands[Bands::hf].settings.fc_hz    = 10e3;
    this->updateDefaults();
  }

  Audio process(Audio x) noexcept override {
    this->template updatePeakLevel<0>(x);
    if (this->bypassEnable) {
      this->template updatePeakLevel<1>(x);
      return x;
    }
    Audio y = x;
    std::array<Audio, Bands::n> gr;
    for (size_t i = 0; i < Bands::n; i++) {
      auto yFlt = this->bands[i].process(x);
      gr[i]     = this->scs[i].process(yFlt);
      y += yFlt * (gr[i] * this->gain_lin[i] - 1) / this->bands[i].settings.q;
    }
    this->template updatePeakLevel<2, true>(gr[0]);
    this->template updatePeakLevel<3, true>(gr[1]);
    this->template updatePeakLevel<4, true>(gr[2]);
    this->template updatePeakLevel<5, true>(gr[3]);
    this->template updatePeakLevel<1>(y);
    return y;
  }

  void update() noexcept override {
    for (size_t i = 0; i < Bands::n; i++) {
      this->gain_lin[i] = NtFx::invDb(this->bands[i].settings.gain_db);
      auto tau          = signal_t(1) / this->bands[i].settings.fc_hz;
      this->scSettings[i].tAtt_ms = tau * this->attScale;
      this->scSettings[i].tRel_ms = tau * this->relScale;
      this->scs[i].update();
      this->bands[i].update();
    }
  }

  void reset(float fs) noexcept override {
    this->fs = fs;
    for (size_t i = 0; i < Bands::n; i++) {
      this->bands[i].settings.shape = NtFx::Biquad::Shape::bpf;
      this->scs[i].reset(fs);
      this->bands[i].reset(fs);
    }
    this->update();
  }
};
