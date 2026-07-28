#pragma once

#include "lib/Audio.h"
#include "lib/Biquad.h"
#include "lib/Comp.h"
#include "lib/Plugin.h"
#include "lib/utils.h"
#include <array>
#include <cstddef>

enum Bands { lf, loMid, mid, hiMid, hf, n };
enum AttRelMode { relative, user };

struct ntDynamicEq : public NtFx::NtPlugin {
  const std::array<std::string, Bands::n> bandNames {
    "Low", "Low Mid", "Mid", "High Mid", "High"
  };
  std::array<NtFx::Biquad::EqBand, Bands::n> bands;
  std::array<NtFx::Comp::ScSettings, Bands::n> scSettings;
  std::array<NtFx::Comp::PeakSideChainLinear, Bands::n> scs;
  std::array<signal_t, Bands::n> gain_lin;
  std::array<bool, Bands::n> solos = { false, false, false, false };
  std::array<bool, Bands::n> mutes = { false, false, false, false };
  // std::array<bool, Bands::n> compDisables = { false, false, false, false };
  signal_t attScale { 0.25 };
  signal_t relScale { 4 };
  AttRelMode attRelMode { relative };
  bool bypassEnable { false };
  bool soloAny { false };

  ntDynamicEq()
      : scs({
            scSettings[0],
            scSettings[1],
            scSettings[2],
            scSettings[3],
            scSettings[4],
        }) {
    for (size_t i = 0; i < Bands::n; i++) {
      this->knobGroups.push_back({ .name = bandNames[i] });
    }
    for (size_t i = 0; i < Bands::n; i++) {
      this->knobGroups[i].primaryKnobs.push_back({
          .p_val  = &this->bands[i].settings.gain_db,
          .name   = "Gain",
          .suffix = " dB",
          .minVal = -20,
          .maxVal = 20,
      });
      this->knobGroups[i].primaryKnobs.push_back({
          .p_val    = &this->bands[i].settings.fc_hz,
          .name     = "Freq",
          .suffix   = " Hz",
          .minVal   = 20,
          .maxVal   = 20e3,
          .midPoint = 2e3,
      });
      this->knobGroups[i].primaryKnobs.push_back({
          .p_val    = &this->bands[i].settings.q,
          .name     = "Q",
          .minVal   = 0.5,
          .maxVal   = 10,
          .midPoint = 1,
      });
      this->knobGroups[i].primaryKnobs.push_back({
          .p_val  = &this->scSettings[i].thresh_db,
          .name   = "Thresh",
          .suffix = " dB",
          .minVal = -60,
          .maxVal = 0,
      });
      this->knobGroups[i].primaryKnobs.push_back({
          .p_val    = &this->scSettings[i].ratio,
          .name     = "Ratio",
          .minVal   = 1,
          .maxVal   = 20,
          .midPoint = 2,
      });
      this->knobGroups[i].primaryKnobs.push_back({
          .p_val    = &this->scSettings[i].tAtt_ms,
          .name     = "Attack",
          .suffix   = " ms",
          .minVal   = 0.01,
          .maxVal   = 50.0,
          .midPoint = 5,
          .isActive = false,
      });
      this->knobGroups[i].primaryKnobs.push_back({
          .p_val    = &this->scSettings[i].tRel_ms,
          .name     = "Release",
          .suffix   = " ms",
          .minVal   = 10.0,
          .maxVal   = 1000.0,
          .midPoint = 100.0,
          .isActive = false,
      });
    }
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
    this->toggleSets = {
      { "Solo", { } },
      { "Bypass", { } },
      // { "Comp Off", { } },
    };
    this->radioButtons = {
      { (int*)&this->attRelMode, "Att/Rel", { "Relative", "Variable" } },
    };
    for (size_t i = 0; i < Bands::n; i++) {
      this->toggleSets[0].toggles.push_back({
          .p_val = &this->solos[i],
          .name  = bandNames[i],
      });
      this->toggleSets[1].toggles.push_back({
          .p_val = &this->mutes[i],
          .name  = bandNames[i],
      });
      // this->toggleSets[2].toggles.push_back({
      //     .p_val = &this->compDisables[i],
      //     .name  = bandNames[i],
      // });
    }
    this->toggles = {
      { .p_val = &this->bypassEnable, .name = "Bypass" },
    };
    for (size_t i = 0; i < Bands::n; i++) {
      this->meters.push_back({
          .name   = this->bandNames[i],
          .invert = true,
      });
    }
    this->uiSpec.maxColumns                  = Bands::n;
    this->uiSpec.maxRows                     = 8;
    this->uiSpec.knobHeight                  = 140;
    this->bands[Bands::lf].settings.fc_hz    = 100;
    this->bands[Bands::loMid].settings.fc_hz = 500;
    this->bands[Bands::mid].settings.fc_hz   = 1000;
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
    Audio acc { 0, 0 };
    if (!this->soloAny) { acc = x; }
    std::array<Audio, Bands::n> gr { 1, 1, 1, 1 };
    for (size_t i = 0; i < Bands::n; i++) {
      auto yFlt = this->bands[i].process(x);
      if (this->mutes[i]) { continue; }
      gr[i] = this->scs[i].process(yFlt);
      acc += yFlt * !(this->soloAny ^ this->solos[i])
          * (gr[i] * this->gain_lin[i] - 1) / this->bands[i].settings.q;
    }
    this->template updatePeakLevel<2, true>(gr[0]);
    this->template updatePeakLevel<3, true>(gr[1]);
    this->template updatePeakLevel<4, true>(gr[2]);
    this->template updatePeakLevel<5, true>(gr[3]);
    this->template updatePeakLevel<6, true>(gr[4]);
    this->template updatePeakLevel<1>(acc);
    return acc;
  }

  void update() noexcept override {
    if (this->attRelMode == relative) {
      this->secondaryKnobs[0].isActive = true;
      this->secondaryKnobs[1].isActive = true;
    } else {
      this->secondaryKnobs[0].isActive = false;
      this->secondaryKnobs[1].isActive = false;
    }
    for (size_t i = 0; i < Bands::n; i++) {
      this->gain_lin[i] = NtFx::invDb(this->bands[i].settings.gain_db);
      if (this->attRelMode == relative) {
        auto tau = signal_t(1) / this->bands[i].settings.fc_hz;
        this->scSettings[i].tAtt_ms                  = tau * this->attScale;
        this->scSettings[i].tRel_ms                  = tau * this->relScale;
        this->knobGroups[i].primaryKnobs[5].isActive = false;
        this->knobGroups[i].primaryKnobs[6].isActive = false;
      } else {
        this->knobGroups[i].primaryKnobs[5].isActive = true;
        this->knobGroups[i].primaryKnobs[6].isActive = true;
      }
      this->scs[i].update();
      this->bands[i].update();
    }
    this->_updateMutes();
    this->uiNeedsUpdate = true;
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

  void _updateMutes() {
    this->soloAny = false;
    for (size_t i = 0; i < Bands::n; i++) {
      if (this->solos[i]) { this->soloAny = true; }
    }
  }
};
