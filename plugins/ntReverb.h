#pragma once

#include "lib/Audio.h"
#include "lib/Delay.h"
#include "lib/FirstOrder.h"
#include "lib/Plugin.h"
#include "lib/Reverb.h"
#include "lib/SoftClip.h"
#include "lib/utils.h"
#include <cstddef>

struct ntReverb final : public NtFx::Plugin {
  static constexpr size_t nLate        = 32;
  static constexpr size_t nEarly       = 16;
  static constexpr double tEarlyMax_ms = 500;
  static constexpr double tLateMax_ms  = 250;
  NtFx::Reverb::DelayNetwork<nLate, tLateMax_ms> tail;
  NtFx::Reverb::EarlyReflections<nEarly, tEarlyMax_ms> early;
  NtFx::Delay::Long<100.0> preDelay;
  NtFx::FirstOrder::Filter<NtFx::FirstOrder::Shape::lpfZero> lpf;
  NtFx::FirstOrder::Filter<NtFx::FirstOrder::Shape::hpf> hpf;
  NtFx::Clip::Soft5 tailClip;
  signal_t gEarly_db { 0 };
  signal_t gTail_db { -14 };
  signal_t gEarly_lin { 1 };
  signal_t gTail_lin { 1 };
  signal_t gDry_db { -100 };
  signal_t gDry_lin { 0 };
  signal_t tPre_ms { 25 };
  bool bypassEnable { false };

  ntReverb() {
    this->knobGroups = {
      {
          "Early",
          {
              { &this->early.tBegin_ms, "First", " ms", 3, tEarlyMax_ms / 2 },
              { &this->early.tEnd_ms, "Last", " ms", 27, tEarlyMax_ms },
              { &this->early.decayDepth, "Decay", "", 0, 1 },
          },
      },
      {
          "Modulation",
          {
              { &this->tail.mod.depth_p, "Depth", " %", 0, 100 },
              { &this->tail.mod.fMod_hz.ui, "Frequency", " Hz", 0.1, 20 },
          },
      },
      {
          "Tail",
          {
              { &this->tail.tBegin_ms, "First", " ms", 3, tLateMax_ms / 2 },
              { &this->tail.tEnd_ms, "Last", " ms", 27, tLateMax_ms },
              { &this->tailClip.gain_db, "Drive", " dB", -24, 24 },
          },
      },
      {
          "Damping",
          {
              { &this->tail.gDamp_db, "Damp", " dB", 0, 3 },
              { &this->tail.fDamp, "Damp Freq", " Hz", 1000, 20e3 },
              { &this->tail.loMult, "Low Mult", " x", 0.5, 2, 1 },
              { &this->tail.fLfMult, "Mult Freq", " Hz", 20, 2000 },
          },
      },
      {
          "Filters",
          {
              { &this->lpf.fc_hz, "LPF", " Hz", 1e3, 20e3, 2e3 },
              { &this->hpf.fc_hz, "HPF", " Hz", 20, 1e3, 200 },
          },
      },
      { "Mixer",
          {
              { &this->gEarly_db, "Early", " dB", -100, 0, -10 },
              { &this->gTail_db, "Tail", " dB", -100, 0, -10 },
              { &this->gDry_db, "Dry", " dB", -100, 0, -10 },
          } }
    };
    this->primaryKnobs = {
      { &this->tail.t60_s, "Time", " s", 0.2, 20, 2 },
      { &this->preDelay.t_ms, "Predelay", " ms", 0, 100 },
    };
    this->tail.gDamp_db    = 0.8;
    this->early.tEnd_ms    = 160;
    this->tail.tBegin_ms   = 11;
    this->tail.tEnd_ms     = 130;
    this->lpf.fc_hz        = 20e3;
    this->hpf.fc_hz        = 20;
    this->early.decayDepth = 0.7;
    this->lpf.fc_hz        = 6e3;
    this->tail.fDamp       = 2e3;

    this->uiSpec.nGroupRows = 4;
    // this->uiSpec.knobWidth  = 200;
    this->uiSpec.maxColumns = 1;
    this->updateDefaults();
  }

  Audio process(Audio x) noexcept override {
    this->updatePeakLevel(0, x);
    if (this->bypassEnable) {
      this->updatePeakLevel(1, x);
      return x;
    }
    auto yLpf      = this->lpf.process(x);
    auto yHpf      = this->hpf.process(yLpf);
    auto yPreDelay = this->preDelay.process(yHpf);
    auto xEarly    = yPreDelay;
    auto yEarly    = this->early.process(xEarly);
    auto xTail     = yPreDelay;
    auto yTail     = this->tail.process(this->tailClip.process(xTail));
    auto y         = yTail * gTail_lin + yEarly * gEarly_lin + x * gDry_lin;
    this->updatePeakLevel(1, y);
    return y;
  }

  template <class T>
  void conformBeginEnd(T& obj) {
    if (size_t(obj.tEnd_ms) <= NtFx::nextPrime(size_t(obj.tBegin_ms))) {
      obj.tEnd_ms =
          signal_t(NtFx::nextPrime(NtFx::nextPrime(size_t(obj.tBegin_ms))));
      this->uiNeedsUpdate = true;
    }
  }

  void update() noexcept override {
    conformBeginEnd(tail);
    conformBeginEnd(early);
    this->gEarly_lin = NtFx::invDb(this->gEarly_db);
    this->gTail_lin  = NtFx::invDb(this->gTail_db);
    this->gDry_lin   = NtFx::invDb(this->gDry_db);
    this->tail.update();
    this->early.update();
    this->preDelay.update();
    this->lpf.update();
    this->hpf.update();
    this->tailClip.update();
  }

  void reset(signal_t fs) noexcept override {
    this->_fs = fs;
    this->tail.reset(fs);
    this->early.reset(fs);
    this->preDelay.reset(fs);
    this->lpf.reset(fs);
    this->hpf.reset(fs);
    this->tailClip.reset(fs);
    this->update();
  }
};
