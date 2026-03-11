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
 **/

#pragma once

#include "Component.h"
#include "Stereo.h"
#include <algorithm>
#include <array>

namespace NtFx {

/**
 * @brief RMS (Root Mean Square) sensor component for audio signal processing
 *
 * This component calculates the RMS value of an audio signal over a specified
 * time period. It maintains two delay lines: one for sample-level accumulation
 * and one for millisecond-level accumulation, allowing for efficient RMS
 * calculation.
 *
 * @tparam signal_t The type of the audio signal (e.g., float, double)
 * @tparam maxT_ms Maximum time window in milliseconds for RMS calculation
 * (default: 1000)
 * @tparam maxSampleDLineLen Maximum length of the sample delay line (default:
 * 192 * 8)
 */
template <typename signal_t,
    int maxT_ms           = 1000,
    int maxSampleDLineLen = 192 * 8>
struct RmsSensor : public Component<signal_t> {
  /** @brief Flag to reset accumulators */
  bool resetAccums = false;
  /** @brief Current time window in milliseconds */
  int msDLineLen = maxT_ms;
  /** @brief Current length of the sample delay line */
  int sampleDLineLen = maxSampleDLineLen;
  /** @brief Sample delay line for storing recent signal values */
  std::array<signal_t, maxSampleDLineLen> samleDLine;
  /** @brief Millisecond delay line for storing accumulated sample values */
  std::array<signal_t, maxT_ms> msDLine;
  /** @brief Accumulator for current sample values */
  signal_t sampleAccum;
  /** @brief Accumulator for millisecond-level values */
  signal_t msAccum;
  /** @brief Current index in the sample delay line */
  int sampleIdx;
  /** @brief Current index in the millisecond delay line */
  int msIdx;

  /**
   * @brief Process the input signal and update RMS calculation
   *
   * This method processes the input signal, calculates its square value,
   * and updates the RMS calculation using a delay line approach.
   *
   * @param x The input signal value
   * @return The current RMS value
   */
  virtual signal_t process(signal_t x) noexcept override {
    auto x2 = x * x;
    if (x2 != x2) { x2 = signal_t(0.0); }
    this->sampleAccum += x2 - this->samleDLine[this->sampleIdx];
    this->samleDLine[this->sampleIdx] = x2;
    if (++this->sampleIdx >= this->sampleDLineLen) {
      this->sampleIdx = 0;
      this->msAccum += sampleAccum - this->msDLine[this->msIdx];
      this->msDLine[this->msIdx] = sampleAccum;
      if (++this->msIdx >= this->msDLineLen) { this->msIdx = 0; }
    }
    return this->getRms();
  }

  /**
   * @brief Update the component state
   *
   * This method resets the accumulators and delay lines if the resetAccums flag
   * is set.
   */
  virtual void update() noexcept override {
    if (this->resetAccums) {
      this->sampleIdx   = 0;
      this->msIdx       = 0;
      this->sampleAccum = 0;
      this->msAccum     = 0;
      std::fill(this->samleDLine.begin(), this->samleDLine.end(), 0);
      std::fill(this->msDLine.begin(), this->msDLine.end(), 0);
      this->resetAccums = false;
    }
  }

  /**
   * @brief Reset the component with a new sample rate
   *
   * This method updates the sample rate and recalculates the sample delay line
   * length based on the new rate.
   *
   * @param fs The new sample rate in Hz
   */
  virtual void reset(float fs) noexcept override {
    if (this->fs == fs) { return; }
    this->fs             = fs;
    this->sampleDLineLen = fs / 1000;
    this->resetAccums    = true;
    this->update();
  }

  /**
   * @brief Get the current RMS value
   *
   * This method calculates and returns the current RMS value based on the
   * accumulated signal values.
   *
   * @return The current RMS value
   */
  signal_t getRms() const noexcept {
    signal_t y = gcem::sqrt(signal_t(2.0) * this->msAccum
        / signal_t(this->sampleDLineLen * this->msDLineLen));

    if (y != y) { y = signal_t(0.0); }
    return y;
  }

  /**
   * @brief Set the time window for RMS calculation
   *
   * This method updates the time window in milliseconds and resets the
   * accumulators.
   *
   * @param t_ms The new time window in milliseconds
   */
  void setT_ms(int t_ms) {
    if (t_ms == this->msDLineLen) { return; }
    this->msDLineLen  = t_ms;
    this->resetAccums = true;
    this->update();
  }
};

/**
 * @brief Stereo RMS sensor component
 *
 * This component provides RMS calculation for stereo audio signals by
 * maintaining two RmsSensor instances (one for each channel).
 *
 * @tparam signal_t The type of the audio signal (e.g., float, double)
 * @tparam maxT_ms Maximum time window in milliseconds for RMS calculation
 * (default: 1000)
 * @tparam maxSampleDLineLen Maximum length of the sample delay line (default:
 * 192 * 8)
 */
template <typename signal_t,
    int maxT_ms           = 1000,
    int maxSampleDLineLen = 192 * 8>
struct RmsSensorStereo : public StereoComponent<signal_t,
                             RmsSensor<signal_t, maxT_ms, maxSampleDLineLen>> {
  /**
   * @brief Set the time window for RMS calculation
   *
   * This method updates the time window for both left and right channels.
   *
   * @param t_ms The new time window in milliseconds
   */
  void setT_ms(int t_ms) {
    this->l.setT_ms(t_ms);
    this->r.setT_ms(t_ms);
  }
  /**
   * @brief Get the current RMS values for both channels
   *
   * This method returns the RMS values for both left and right channels
   * as a Stereo structure.
   *
   * @return The current RMS values for both channels
   */
  Stereo<signal_t> getRms() const noexcept {
    return { this->l.getRms(), this->r.getRms() };
  }
};
}