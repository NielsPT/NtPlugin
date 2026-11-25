#pragma once

#include "lib/Audio.h"
#include "lib/DryMix.h"
#include "lib/FirstOrder.h"
#include "lib/Plugin.h"
#include "lib/Reverb.h"
#include "lib/primes.h"

struct ntMoritzKleinReverb final : public NtFx::Plugin {
  NtFx::Reverb::GoldenRatioAllPass<signal_t> revL;
  NtFx::Reverb::GoldenRatioAllPass<signal_t> revR;
  NtFx::FirstOrder::Filter<NtFx::FirstOrder::Shape::lpf> lpf;
  NtFx::DryMix dryMix;
  signal_t t60_s { 1 };
  signal_t size_m { 13 };
  bool bypassEnable { false };

  ntMoritzKleinReverb() {
    this->primaryKnobs = {
      { &this->t60_s, "Time", " s", 0.1, 2 },
      { &this->size_m, "Size", " m", 5, 26 },
      { &this->lpf.fc_hz, "Damping", " Hz", 200, 20e3, 2e3 },
    };
    this->secondaryKnobs = {
      { &this->dryMix.mix_p, "Mix", " %", 0, 100 },
    };
    this->toggles = {
      { .p_val = &this->bypassEnable, .name = "Bypass" },
    };
    this->lpf.fc_hz = 3e3;
    this->updateDefaults();
  }

  Audio process(Audio x) noexcept override {
    this->updatePeakLevel(0, x);
    if (this->bypassEnable) {
      this->updatePeakLevel(1, x);
      return x;
    }
    auto yLpf  = this->lpf.process(x / 2);
    auto yRevL = this->revL.process(yLpf.l);
    auto yRevR = this->revR.process(yLpf.r);
    auto y     = this->dryMix.process(yRevL + yRevR, x);
    this->updatePeakLevel(1, y);
    return y;
  }

  void update() noexcept override {
    this->revL.t60_ms  = this->t60_s * 1000;
    this->revR.t60_ms  = this->t60_s * 1000;
    this->revL.size_ms = signal_t(NtFx::nextPrime(size_t(this->size_m * 3.4)));
    this->revR.size_ms = signal_t(NtFx::nextPrime(size_t(this->revL.size_ms)));
    this->revL.update();
    this->revR.update();
    this->lpf.update();
    this->dryMix.update();
  }

  void reset(signal_t fs) noexcept override {
    this->_fs = fs;
    this->revL.reset(fs);
    this->revR.reset(fs);
    this->lpf.reset(fs);
    this->update();
  }
};
