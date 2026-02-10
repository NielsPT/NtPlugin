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
/**
 * @file PeakSensor.h
 * @brief Peak sensor component for audio processing.
 *
 * This file contains the implementation of a peak sensor component, which is
 * used to detect and track the peak amplitude of an audio signal. The peak
 * sensor can be used in various audio effects, such as compressors, limiters,
 * and more.
 */

#include "gcem.hpp"
#include "lib/Component.h"
#include "lib/Stereo.h"
#include "lib/utils.h"

namespace NtFx {

/**
 * @brief Peak sensor component for audio processing.
 *
 * The PeakSensor component detects and tracks the peak amplitude of an audio
 * signal. It uses an exponential decay to smooth the detected peaks, allowing
 * for a configurable release time. This component is useful in audio effects
 * that require peak detection, such as compressors, limiters, and more.
 *
 * @tparam signal_t The type of the audio signal (e.g., float, double).
 */
template <typename signal_t>
struct PeakSensor : public Component<signal_t> {
  signal_t tPeak_ms; ///< Time constant for peak detection in milliseconds.
  signal_t alpha;    ///< Smoothing factor for the peak detection.
  signal_t state;    ///< Internal state of the peak sensor.

  /**
   * @brief Processes the input signal to detect and track the peak amplitude.
   *
   * This method applies the peak sensor algorithm to the input signal, updating
   * the internal state and returning the detected peak amplitude.
   *
   * @param x The input signal sample.
   * @return The detected peak amplitude.
   */
  virtual signal_t process(signal_t x) noexcept override {
    return this->_peakSensor(this->alpha, this->state, x);
  }

  /**
   * @brief Updates the internal parameters of the peak sensor.
   *
   * This method recalculates the smoothing factor (alpha) based on the current
   * time constant (tPeak_ms) and sample rate (fs).
   */
  virtual void update() noexcept override {
    this->alpha = gcem::exp(-2200.0 / (this->tPeak_ms * this->fs));
  }

  /**
   * @brief Static method to apply the peak sensor algorithm.
   *
   * This method implements the core peak sensor algorithm, which detects the
   * peak amplitude of the input signal and applies an exponential decay to
   * smooth the detected peaks.
   *
   * @param alpha The smoothing factor for the peak detection.
   * @param p_state Reference to the internal state of the peak sensor.
   * @param x The input signal sample.
   * @return The detected peak amplitude.
   */
  static inline signal_t _peakSensor(
      signal_t alpha, signal_t& p_state, signal_t x) {
    auto xAbs            = gcem::abs(x);
    signal_t sensRelease = alpha * p_state + (1 - alpha) * xAbs;
    signal_t ySens       = gcem::max(xAbs, sensRelease);
    ensureFinite(ySens);
    p_state = ySens;
    return ySens;
  }
};

/**
 * @brief Stereo peak sensor component for audio processing.
 *
 * The PeakSensorStereo component is a stereo version of the PeakSensor,
 * allowing for independent peak detection on the left and right channels of an
 * audio signal. This is useful in stereo audio effects where peak detection
 * needs to be applied separately to each channel.
 *
 * @tparam signal_t The type of the audio signal (e.g., float, double).
 */
template <typename signal_t>
struct PeakSensorStereo
    : public StereoComponent<signal_t, PeakSensor<signal_t>> {
  /**
   * @brief Sets the time constant for peak detection in milliseconds.
   *
   * This method updates the time constant for both the left and right channels.
   *
   * @param t_ms The time constant in milliseconds.
   */
  void setT_ms(signal_t t_ms) {
    this->l.tPeak_ms = t_ms;
    this->r.tPeak_ms = t_ms;
  }
};
}

// #include "gcem.hpp"
// #include "lib/Component.h"
// #include "lib/Stereo.h"
// #include "lib/utils.h"

// namespace NtFx {

// template <typename signal_t>
// struct PeakSensor : public Component<signal_t> {
//   signal_t tPeak_ms;
//   signal_t alpha;
//   signal_t state;

//   virtual signal_t process(signal_t x) noexcept override {
//     return this->_peakSensor(this->alpha, this->state, x);
//   }

//   virtual void update() noexcept override {
//     this->alpha = gcem::exp(-2200.0 / (this->tPeak_ms * this->fs));
//   }

//   static inline signal_t _peakSensor(
//       signal_t alpha, signal_t& p_state, signal_t x) {
//     auto xAbs            = gcem::abs(x);
//     signal_t sensRelease = alpha * p_state + (1 - alpha) * xAbs;
//     signal_t ySens       = gcem::max(xAbs, sensRelease);
//     ensureFinite(ySens);
//     p_state = ySens;
//     return ySens;
//   }
// };

// template <typename signal_t>
// struct PeakSensorStereo
//     : public StereoComponent<signal_t, PeakSensor<signal_t>> {
//   void setT_ms(signal_t t_ms) {
//     this->l.tPeak_ms = t_ms;
//     this->r.tPeak_ms = t_ms;
//   }
// };
// }