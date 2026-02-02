/*
 * Copyright (C) 2026 Niels Thøgersen, NTlyd
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Affero General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option) any
 * later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 * You are free to download, build and use this code for commercial
 * purposes. Just don't resell it or a build of it, modified or otherwise.
 **/

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
    this->primaryKnobs.push_back({
        .p_val  = &this->gain_db,
        .name   = "Gain",
        .suffix = " dB",
        .minVal = -24,
        .maxVal = 24,
    });

    // Change the default background colours.
    this->uiSpec.backgroundColour = 0xFFFFFFFF;
    this->uiSpec.foregroundColour = 0xFF000000;

    // We don't need that big a window for just one knob.
    this->uiSpec.defaultWindowWidth = 500;

    // Let's make the meter smaller.
    this->uiSpec.meterHeight_dots = 8;

    // Add two meters.
    this->uiSpec.meters.push_back({ .name = "IN" });
    this->uiSpec.meters.push_back({ .name = "OUT", .hasScale = true });

    // Always remember to update defaults. This will load the member values into
    // the UI and session storage.
    this->updateDefaults();
  }

  // Override the process method.
  NtFx::Stereo<signal_t> process(NtFx::Stereo<signal_t> x) noexcept override {

    // Update the input meter.
    this->template updatePeakLevel<0>(x);

    // Calculate gain
    NtFx::Stereo<signal_t> y = x * this->gain_lin;

    // Update output meter.
    this->template updatePeakLevel<1>(y);
    return y;
  }

  // Override the process method.
  void update() noexcept override {

    // Calculate the gain in the linear domain.
    this->gain_lin = NtFx::invDb(this->gain_db);
  }

  // Override the reset method.
  void reset(int fs) noexcept override { this->update(); }
};