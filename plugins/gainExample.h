#pragma once

#include "lib/Plugin.h"
#include "lib/Stereo.h"

// Make a class the inherits from Plugin. This must be a template so we can keep
// the whole thing in headers and swap the signal data type. I perfer using
// structs so I don't need to consider private/public.
template <typename signal_t>
struct gainExample : NtFx::NtPlugin<signal_t> {
  // Make some variables.
  signal_t gain_db { 0 };
  signal_t gain_lin { 1 };

  // Write the constructor.
  gainExample() {

// Add a knob.
#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
  #warning BE WARE! WINDOWS!!
    this->primaryKnobs.push_back(
        { &this->gain_db, "Gain", " dB", -24, 24, 0.0, 0.0 });
#else
    this->primaryKnobs.push_back({
        .p_val  = &this->gain_db,
        .name   = "Gain",
        .suffix = " dB",
        .minVal = -24,
        .maxVal = 24,
    });
#endif

    // Change the default background colours.
    this->guiSpec.backgroundColour = 0xFFFFFFFF;
    this->guiSpec.foregroundColour = 0xFF000000;

    // We don't need that big a window for just one knob.
    this->guiSpec.defaultWindowWidth = 500;

    // Let's make the meter smaller.
    this->guiSpec.meterHeight_dots = 8;

    // Add two meters.
    this->guiSpec.meters.push_back({ .name = "IN" });
    this->guiSpec.meters.push_back({ .name = "OUT", .hasScale = true });

    // Always remember to update defaults. This will load the member values into
    // the UI and session storage.
    this->updateDefaults();
  }

  // Override the processSample method.
  NtFx::Stereo<signal_t> processSample(
      NtFx::Stereo<signal_t> x) noexcept override {

    // Update the input meter.
    this->template updatePeakLevel<0>(x);

    // Calculate gain
    NtFx::Stereo<signal_t> y = x * this->gain_lin;

    // Update output meter.
    this->template updatePeakLevel<1>(y);
    return y;
  }

  // Override the processSample method.
  void updateCoeffs() noexcept override {

    // Calculate the gain in the linear domain.
    this->gain_lin = NtFx::invDb(this->gain_db);
  }

  // Override the reset method.
  void reset(int fs) noexcept override { this->updateCoeffs(); }
};