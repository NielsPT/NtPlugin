#pragma once

#include "lib/Plugin.h"
#include "lib/RmsSensor.h"
#include "lib/Stereo.h"

template <typename signal_t>
struct ntRmsMeter : NtFx::NtPlugin<signal_t> {
  enum Meter { Peak, RMS_10_ms, RMS_100_ms, RMS_1_s };
  NtFx::RmsSensor<signal_t> sensor;
  signal_t decay_s = 0.1;
  signal_t hold_s  = 2;
  ntRmsMeter() {
    this->uiSpec.meters = {
      { "Peak", .hasScale = true, .minVal_db = -50 },
      { "RMS_10_ms", .minVal_db = -50 },
      { "RMS_100_ms", .minVal_db = -50 },
      { "RMS_1_s", .hasScale = true, .minVal_db = -50 },
    };
    this->secondaryKnobs = {
      { &this->decay_s, "Decay", " s", 0, 1 },
      { &this->hold_s, "Hold", " s", 0, 10 },
    };
    this->uiSpec.meterHeight_dots = 25;
    this->updateDefaults();
  }
  virtual NtFx::Stereo<signal_t> process(
      NtFx::Stereo<signal_t> x) noexcept override {
    sensor.process(x);
    this->template updatePeakLevel<Peak>(x);
    this->template updatePeakLevel<RMS_10_ms>(sensor.getRms(1));
    this->template updatePeakLevel<RMS_100_ms>(sensor.getRms(2));
    this->template updatePeakLevel<RMS_1_s>(sensor.getRms(3));
    return x;
  }
  virtual void update() noexcept override {
    for (auto& m : this->uiSpec.meters) { m.decay_s = this->decay_s; }
    for (auto& m : this->uiSpec.meters) { m.hold_s = this->hold_s; }
    this->uiNeedsUpdate = true;
  }
  virtual void reset(float fs) noexcept override {
    this->fs = fs;
    this->sensor.reset(fs);
    this->update();
  }
};