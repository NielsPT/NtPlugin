#pragma once

#include "lib/Audio.h"
#include "lib/Plugin.h"
#include "lib/SoftClip.h"

struct ntSoftClip final : public NtFx::Plugin {
  NtFx::SoftClipAntiAlias3 clip3;
  bool bypassEnable { false };

  ntSoftClip() {
    this->primaryKnobs = {
      { &this->clip3.gain_db, "Drive", " dB", -24, 24 },
      // { &this->clip3.fXOver_hz, "xOver", " Hz", 20, 20e3, 2e3 },
    };
    this->toggles = {
      { &this->clip3.bypass1Enable, "HF bypass" },
      { &this->clip3.bypass2Enable, "HHF bypass" },
      { &this->bypassEnable, "Bypass" },
    };
    this->updateDefaults();
  }

  Audio process(Audio x) noexcept override {
    this->updatePeakLevel(0, x);
    if (this->bypassEnable) {
      this->updatePeakLevel(1, x);
      return x;
    }
    Audio y = clip3.process(x);
    this->updatePeakLevel(1, y);
    return y;
  }

  void update() noexcept override { this->clip3.update(); }

  void reset(signal_t fs) noexcept override {
    this->_fs = fs;
    this->clip3.reset(fs);
    this->update();
  }
};
