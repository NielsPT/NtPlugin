#pragma once

#include "lib/Audio.h"
#include "lib/Generator.h"
#include "lib/Plugin.h"
#include "lib/utils.h"
#include <cassert>
#include <vector>

enum Mode : int {
  e_sin,
  e_saw,
  e_white,
  e_pink,
  e_filteredWhite,
  e_filteredPink,
  e_n
};

struct ntGenerator final : public NtFx::Plugin {
  NtFx::Generator::Sin sin;
  NtFx::Generator::Saw saw;
  NtFx::Generator::WhiteNoise white;
  NtFx::Generator::PinkNoise pink;
  NtFx::Generator::FilteredWhiteNoise filteredWhite;
  NtFx::Generator::FilteredPinkNoise filteredPink;
  signal_t f_hz { 1e3 };
  signal_t wet_db { 0 };
  signal_t dry_db { 0 };
  signal_t wet_lin { 1 };
  signal_t dry_lin { 1 };
  signal_t fHpf { 20 };
  signal_t fLpf { 20e3 };
  Mode mode { Mode::e_sin };

  ntGenerator() {
    this->primaryKnobs = {
      { &f_hz, "Frequency", " Hz", 20, 20e3, 1e3 },
      { &wet_db, "Wet", " dB", -100, 0 },
      { &dry_db, "Dry", " dB", -100, 0 },
    };
    this->secondaryKnobs = {
      { &this->fHpf, "HPF", " Hz", 20, 20e3, 1e3 },
      { &this->fLpf, "LPF", " Hz", 20, 20e3, 1e3 },
    };
    this->radioButtons = {
      {
          (int*)&this->mode,
          "Mode",
          {
              "Sine Wave",
              "Saw Wave",
              "White Noise",
              "Pink Noise",
              "Filtered White",
              "Filtered Pink",
          },
      },
    };
    this->updateDefaults();
  }

  Audio process(Audio x) noexcept override {
    this->updatePeakLevel(0, x);
    Audio yGen = { 0, 0 };
    switch (this->mode) {
    case Mode::e_sin:
      yGen = this->sin.process(x);
      break;
    case Mode::e_saw:
      yGen = this->saw.process(x);
      break;
    case Mode::e_white:
      yGen = this->white.process(x);
      break;
    case Mode::e_pink:
      yGen = this->pink.process(x);
      break;
    case Mode::e_filteredWhite:
      yGen = this->filteredWhite.process(x);
      break;
    case Mode::e_filteredPink:
      yGen = this->filteredPink.process(x);
      break;
    case Mode::e_n:
    default:
      assert(false);
    }
    auto y = yGen * wet_lin + x * dry_lin;
    this->updatePeakLevel(1, y);
    return y;
  }

  void update() noexcept override {
    wet_lin                       = NtFx::invDb(wet_db);
    dry_lin                       = NtFx::invDb(dry_db);
    this->filteredPink.lpf.fc_hz  = this->fLpf;
    this->filteredWhite.lpf.fc_hz = this->fLpf;
    this->filteredPink.hpf.fc_hz  = this->fHpf;
    this->filteredWhite.hpf.fc_hz = this->fHpf;
    this->sin.f_hz                = this->f_hz;
    this->saw.f_hz                = this->f_hz;
    sin.update();
    saw.update();
    white.update();
    pink.update();
    filteredWhite.update();
    filteredPink.update();
    if (this->mode <= Mode::e_saw) {
      this->activateParameter("Frequency");
    } else {
      this->deactivateParameter("Frequency");
    }
    if (this->mode >= Mode::e_filteredWhite) {
      this->activateParameter("LPF");
      this->activateParameter("HPF");
    } else {
      this->deactivateParameter("LPF");
      this->deactivateParameter("HPF");
    }
  }

  void reset(signal_t fs) noexcept override {
    sin.reset(fs);
    saw.reset(fs);
    white.reset(fs);
    pink.reset(fs);
    filteredWhite.reset(fs);
    filteredPink.reset(fs);
    this->_fs = fs;
    this->update();
  }
};
