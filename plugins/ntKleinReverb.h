#pragma once

#include "lib/Audio.h"
#include "lib/DryMix.h"
#include "lib/FirstOrder.h"
#include "lib/Glider.h"
#include "lib/Plugin.h"
#include "lib/Reverb.h"

struct ntKleinReverb final : public NtFx::Plugin {
  NtFx::Reverb::GoldenRatioAllPass<signal_t> rev;
  NtFx::ExpGlider sizeGlide_ms;
  NtFx::FirstOrder::Filter<NtFx::FirstOrder::Shape::lpf, signal_t> lpf;
  NtFx::FirstOrder::ToneControl<signal_t> tone;
  NtFx::DryMix dryMix;
  signal_t t60_s { 1 };
  signal_t size_m { 13 };
  bool bypassEnable { false };

  ntKleinReverb() {
    this->primaryKnobs = {
      { &this->t60_s, "Time", " s", 0.1, 2 },
      { &this->size_m, "Size", " m", 5, 34 },
      { &this->tone.gain_db, "Tone", " dB", -20, 20 },
    };
    this->secondaryKnobs = {
      { &this->lpf.fc_hz, "Lpf", " Hz", 200, 20e3, 2e3 },
      { &this->tone.fc_hz, "Tone Pivot", " Hz", 20, 20e3, 2e3 },
      { &this->dryMix.mix_p, "Mix", " %", 0, 100 },
    };
    this->toggles = {
      { .p_val = &this->bypassEnable, .name = "Bypass" },
    };
    this->lpf.fc_hz  = 10e3;
    this->tone.fc_hz = 4545.0;
    this->updateDefaults();
  }

  Audio process(Audio x) noexcept override {
    this->updatePeakLevel(0, x);
    if (this->bypassEnable) {
      this->updatePeakLevel(1, x);
      return x;
    }
    this->rev.size_ms = this->sizeGlide_ms.process();
    this->rev.update();
    auto yLpf  = this->lpf.process(x.avg() / 2);
    auto yTone = this->tone.process(yLpf);
    auto yRev  = this->rev.process(yTone);
    auto y     = this->dryMix.process(yRev, x);
    this->updatePeakLevel(1, y);
    return y;
  }

  void update() noexcept override {
    this->rev.t60_ms      = this->t60_s * 1000;
    this->sizeGlide_ms.ui = this->size_m * signal_t(1000.0 / 340.0);
    this->sizeGlide_ms.update(this->_fs);
    this->rev.update();
    this->lpf.update();
    this->tone.update();
    this->dryMix.update();
  }

  void reset(signal_t fs) noexcept override {
    this->_fs = fs;
    this->rev.reset(fs);
    this->lpf.reset(fs);
    this->tone.reset(fs);
    this->update();
  }
};
