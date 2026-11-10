#pragma once

#include "lib/Audio.h"
#include "lib/Plugin.h"
#include "lib/SoftClip.h"
#include "lib/Tilt.h"
#include "lib/utils.h"

struct ntSoftClip final : public NtFx::Plugin {
  enum class Mode {

    // second,
    third,
    fifth,
    hard,
    alt1,
    thirdAntialias,
    fifthAntialias,
  };
  NtFx::Clip::SoftAntialias3 clipA3;
  NtFx::Clip::SoftAntialias5 clipA5;
  // NtFx::Clip::Second second;
  NtFx::Clip::Soft3 clip3;
  NtFx::Clip::Soft5 clip5;
  NtFx::Clip::Hard hard;
  NtFx::Clip::Alt1 alt1;
  NtFx::FirstOrder::Filter<NtFx::FirstOrder::Shape::lpfZero> lpf;
  NtFx::Tilt<> preTilt;
  NtFx::Tilt<> postTilt;
  Mode mode { Mode::third };
  signal_t gain_db { 0 };
  signal_t trim_db { 0 };
  signal_t trim_lin { 1 };
  bool lpfEnable { false };
  bool bypassEnable { false };

  ntSoftClip() {
    this->primaryKnobs = {
      { &this->preTilt.tilt_db, "Tilt", " dB/decade", -20, 20 },
      { &this->gain_db, "Drive", " dB", -24, 24 },
      { &this->lpf.fc_hz, "LPF", " Hz", 20, 22e3, 2e3 },
    };
    this->secondaryKnobs = { { &this->trim_db, "Trim", " dB", -24, 24 } };
    this->toggles        = {
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

              // "Second",
              "Third",
              "Fifth",
              "Hard",
              "Alt1",
              "Third x-over",
              "Fifth x-over",
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
    auto yTilt = this->preTilt.process(x);
    Audio yClip { 0 };
    switch (this->mode) {
    case Mode::thirdAntialias:
      yClip = this->clipA3.process(yTilt);
      break;
    case Mode::fifthAntialias:
      yClip = this->clipA5.process(yTilt);
      break;
    // case Mode::second:
    //   yClip = this->second.process(x);
    //   break;
    case Mode::third:
      yClip = this->clip3.process(yTilt);
      break;
    case Mode::fifth:
      yClip = this->clip5.process(yTilt);
      break;
    case Mode::hard:
      yClip = this->hard.process(yTilt);
      break;
    case Mode::alt1:
      yClip = this->alt1.process(yTilt);
      break;
    }
    Audio y = this->postTilt.process(yClip) * this->trim_lin;
    if (this->lpfEnable) { y = this->lpf.process(y); }
    this->updatePeakLevel(1, y);
    return y;
  }

  void update() noexcept override {
    this->trim_lin       = NtFx::invDb(this->trim_db);
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
    this->postTilt.tilt_db = -this->preTilt.tilt_db;
    this->preTilt.update();
    this->postTilt.update();
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
    this->preTilt.reset(fs);
    this->postTilt.reset(fs);
    this->update();
  }
};
