#pragma once

#include "lib/Audio.h"
#include "lib/Plugin.h"
#include "lib/SoftClip.h"

struct ntSoftClip final : public NtFx::Plugin {
  enum class Mode {
    thirdAntialias,
    fifthAntialias,
    // second,
    third,
    fifth,
    hard,
    alt1,
  };
  NtFx::Clip::SoftAntialias3 clipA3;
  NtFx::Clip::SoftAntialias5 clipA5;
  // NtFx::Clip::Second second;
  NtFx::Clip::Soft3 clip3;
  NtFx::Clip::Soft5 clip5;
  NtFx::Clip::Hard hard;
  NtFx::Clip::Alt1 alt1;
  NtFx::FirstOrder::StereoFilter<NtFx::FirstOrder::Shape::lpfZero> lpf;
  Mode mode { Mode::thirdAntialias };
  signal_t gain_db { 0 };
  bool lpfEnable { true };
  bool bypassEnable { false };

  ntSoftClip() {
    this->primaryKnobs = {
      { &this->gain_db, "Drive", " dB", -24, 24 },
      { &this->lpf.fc_hz, "LPF", " Hz", 20, 22e3, 2e3 },
    };
    this->toggles = {
      { &this->lpfEnable, "LPF enable" },
      // { &this->clip3.bypass1Enable, "HF bypass" },
      // { &this->clip3.bypass2Enable, "HHF bypass" },
      { &this->bypassEnable, "Bypass" },
    };
    this->radioButtons = {
      {
          (int*)&this->mode,
          "Mode",
          {
              "Third antialias",
              "Fifth antialias",
              // "Second",
              "Third",
              "Fifth",
              "Hard",
              "Alt1",
          },
      },
    };
    this->lpf.fc_hz = 22e3;
    this->updateDefaults();
  }

  Audio process(Audio x) noexcept override {
    this->updatePeakLevel(0, x);
    if (this->bypassEnable) {
      this->updatePeakLevel(1, x);
      return x;
    }
    Audio yClip { 0 };
    switch (this->mode) {
    case Mode::thirdAntialias:
      yClip = this->clipA3.process(x);
      break;
    case Mode::fifthAntialias:
      yClip = this->clipA5.process(x);
      break;
    // case Mode::second:
    //   yClip = this->second.process(x);
    //   break;
    case Mode::third:
      yClip = this->clip3.process(x);
      break;
    case Mode::fifth:
      yClip = this->clip5.process(x);
      break;
    case Mode::hard:
      yClip = this->hard.process(x);
      break;
    case Mode::alt1:
      yClip = this->alt1.process(x);
      break;
    }
    Audio y = yClip;
    if (this->lpfEnable) { y = this->lpf.process(yClip); }
    this->updatePeakLevel(1, y);
    return y;
  }

  void update() noexcept override {
    this->clipA3.gain_db = this->gain_db;
    this->clipA5.gain_db = this->gain_db;
    // this->second.gain_db = this->gain_db;
    this->clip3.gain_db = this->gain_db;
    this->clip5.gain_db = this->gain_db;
    this->hard.gain_db  = this->gain_db;
    this->alt1.gain_db  = this->gain_db;
    this->clipA3.update();
    this->clipA5.update();
    // this->second.update();
    this->clip3.update();
    this->clip5.update();
    this->hard.update();
    this->alt1.update();
    this->lpf.update();
  }

  void reset(signal_t fs) noexcept override {
    this->_fs = fs;
    this->clipA3.reset(fs);
    this->clipA5.reset(fs);
    // this->second.reset(fs);
    this->clip3.reset(fs);
    this->clip5.reset(fs);
    this->hard.reset(fs);
    this->alt1.reset(fs);
    this->lpf.reset(fs);
    this->update();
  }
};
