#pragma once

/**
 * @file RmsSensor.h
 * @author Niels Thøgersen (niels.thoegersen@gmail.com)
 * @brief RMS sensor for audio. Accumulates in multiple stages, with a one
 * milliseond accumulator followed by a user controllable accumulator. This
 * means that the output has a real world sample rate of 1 kHz.
 *
 * @copyright Copyright (c) 2026
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
 */

#include "lib/Audio.h"
#include "lib/Component.h"
#include <algorithm>
#include <array>

namespace NtFx {

template <double tRmsMax_ms>
struct ShortRmsSensorMono : public ComponentBase<signal_t> {
  constexpr static const int nDlMax = int(tRmsMax_ms * 192.0 * 8.0);
  std::array<signal_t, nDlMax> _dl; ///< Sample delay line.
  signal_t tRms_ms;
  signal_t _acc { 0 };
  int _i { 0 };
  int _n { 1 };
  virtual signal_t process(signal_t x) noexcept override {
    this->processDelayLine(x);
    return this->getRms();
  }
  void processDelayLine(signal_t x) noexcept {
    auto x2 = x * x;
    if (x2 != x2) { x2 = signal_t(0.0); }
    this->_acc += x2 - this->_dl[this->_i];
    this->_dl[this->_i++] = x2;
    if (this->_i >= this->_n) { this->_i = 0; }
  }
  signal_t getRms() const noexcept {
    signal_t y = gcem::sqrt(signal_t(2.0) * this->_acc / signal_t(this->_n));
    if (y != y) { y = signal_t(0.0); }
    return y;
  }
  virtual void update() noexcept override {
    auto n = int(gcem::floor(this->tRms_ms * 0.001 * this->fs));
    if (n == this->_n) { return; }
    this->_n   = n;
    this->_i   = 0;
    this->_acc = 0;
    std::fill(this->_dl.begin(), this->_dl.end(), 0);
  }
};

template <double tRmsMax_ms>
struct ShortRmsSensorStereo
    : public AudioComponent<signal_t, ShortRmsSensorMono<tRmsMax_ms>> {
  void setT_ms(signal_t t) {
    this->l.tRms_ms = t;
    this->r.tRms_ms = t;
    this->update();
  }
  Audio getRms() const noexcept {
    return { this->l.getRms(), this->r.getRms() };
  }
};

template <double tRmsMax_ms = 100.0>
using ShortRmsSensor = ShortRmsSensorStereo<tRmsMax_ms>;

/**
 * @brief RMS (Root Mean Square) sensor component for audio signal processing
 *
 * This component calculates the RMS value of an audio signal over a specified
 * time period. It maintains two delay lines: one for sample-level
 * accumulation and one for millisecond-level accumulation, allowing for
 * efficient RMS calculation.
 *
 * @tparam signal_t The type of the audio signal (e.g., float, double)
 * @tparam maxT_ms Maximum time window in milliseconds for RMS calculation
 * (default: 1000)
 * @tparam maxSampleDLineLen Maximum length of the sample delay line (default:
 * 192 * 8)
 */
template <int maxT_ms = 1000, int maxSampleDLineLen = 192 * 8>
struct LongRmsSensorMono : public ComponentBase<Mono<signal_t>> {
  std::array<signal_t, maxSampleDLineLen> sampleDLine; ///< Sample delay line.
  std::array<signal_t, maxT_ms> msDLine; ///< Millisecond delay line.
  signal_t sampleAccum { 0 }; ///< Accumulator for current sample values.
  signal_t msAccum { 0 };     ///< Accumulator for millisecond-level values.
  int sampleIdx { 0 };        ///< Current index in the sample delay line.
  int msIdx { 0 };            ///< Current index in the millisecond delay line.
  int msDLineLen { maxT_ms }; ///< Current time window in milliseconds.
  int sampleDLineLen { 48 };  ///< Length of sample delay line.
  bool resetAccums { false }; ///< Flag to reset accumulators.

  /**
   * @brief Process the input signal and update RMS calculation
   *
   * This method processes the input signal, calculates its square value,
   * and updates the RMS calculation using a delay line approach.
   *
   * @param x The input signal value
   * @return The current RMS value
   */
  virtual Mono<signal_t> process(Mono<signal_t> x) noexcept override {
    this->processDelayLine(x.l);
    return this->getRms();
  }

  /**
   * @brief Processes accumulators and update delay lines without calculating
   * result. To be called each sample if 'process' is not used. This way, you
   * can update accumulators in the processing thread while calculating rms
   * values somewhere else.
   *
   * @param x Inout signal to calculate RMS level for.
   */
  void processDelayLine(signal_t x) noexcept {
    auto x2 = x * x;
    if (x2 != x2) { x2 = signal_t(0.0); }
    this->sampleAccum += x2 - this->sampleDLine[this->sampleIdx];
    this->sampleDLine[this->sampleIdx] = x2;
    if (++this->sampleIdx < this->sampleDLineLen) { return; }
    this->sampleIdx = 0;
    this->msAccum += sampleAccum - this->msDLine[this->msIdx];
    this->msDLine[this->msIdx] = sampleAccum;
    if (++this->msIdx >= this->msDLineLen) { this->msIdx = 0; }
  }

  /**
   * @brief Update the component state
   *
   * This method resets the accumulators and delay lines if the resetAccums
   * flag is set.
   */
  virtual void update() noexcept override {
    if (this->resetAccums) {
      this->sampleIdx   = 0;
      this->msIdx       = 0;
      this->sampleAccum = 0;
      this->msAccum     = 0;
      std::fill(this->sampleDLine.begin(), this->sampleDLine.end(), 0);
      std::fill(this->msDLine.begin(), this->msDLine.end(), 0);
      this->resetAccums = false;
    }
  }

  /**
   * @brief Reset the component with a new sample rate
   *
   * This method updates the sample rate and recalculates the sample delay
   * line length based on the new rate.
   *
   * @param fs The new sample rate in Hz
   */
  virtual void reset(float fs) noexcept override {
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

  // TODO: Use the same pattern as short rms sensor. No resetAccums.
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
template <int maxT_ms = 1000, int maxSampleDLineLen = 192 * 8>
struct LongRmsSensorStereo
    : public AudioComponent<signal_t,
          LongRmsSensorMono<maxT_ms, maxSampleDLineLen>> {
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
  Audio getRms() const noexcept {
    return { this->l.getRms(), this->r.getRms() };
  }
};
template <int maxT_ms = 1000, int maxSampleDLineLen = 192 * 8>
using LongRmsSensor = LongRmsSensorStereo<maxT_ms, maxSampleDLineLen>;
}