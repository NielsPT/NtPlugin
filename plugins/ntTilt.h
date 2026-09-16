#pragma once

#include "lib/Audio.h"
#include "lib/Plugin.h"
#include "lib/Tilt.h"
#include <cstddef>

struct ntTilt final : public NtFx::Plugin {
  NtFx::Tilt<> filter;
  bool bypassEnable { false };

  ntTilt() {
    this->primaryKnobs = { { &this->filter.tilt_db, "Tilt", " dB", -12, 12 } };
    this->toggles      = {
      { .p_val = &this->bypassEnable, .name = "Bypass" },
    };
    this->updateDefaults();
  }

  Audio process(Audio x) noexcept override {
    this->updatePeakLevel(0, x);
    if (this->bypassEnable) {
      this->updatePeakLevel(1, x);
      return x;
    }
    Audio y = filter.process(x);
    this->updatePeakLevel(1, y);
    return y;
  }

  void update() noexcept override { this->filter.update(); }

  void reset(signal_t fs) noexcept override {
    this->_fs = fs;
    this->filter.reset(fs);
    this->update();
  }
};
