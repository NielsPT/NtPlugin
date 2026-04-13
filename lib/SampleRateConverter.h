/**
 * @file SampleRateConverter.h
 * @brief Sample rate converter using FIR filtering for oversampling
 * @author Niels Thøgersen
 * @copyright Copyright (C) 2026 Niels Thøgersen, NTlyd
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

#pragma once

#include NTFX_PLUGIN_FILE
#include "Stereo.h"
#include "lib/windowFunctions.h"
#include <algorithm>
#include <array>
#include <cstddef>

namespace NtFx {
namespace Src {
  /**
   * @brief Delay line size for interpolation and antialiasing
   */
  constexpr int nDelayLine = 192;
  /**
   * @brief Oversampling FIR multiplier for high quality mode
   */
  constexpr int oversamplingFirMultHq = 24;
  /**
   * @brief Oversampling FIR multiplier for low quality mode
   */
  constexpr int oversamplingFirMultLq = 12;
  /**
   * @brief Cutoff frequency for anti-aliasing filter
   */
  constexpr int fc = 22e3;
  /**
   * @brief Oversampling mode enumeration
   */
  enum oversamplingMode : int {
    disable = 1,
    // iir_2x,
    // iir_4x,
    // iir_8x,
    fir_2x_lq,
    fir_4x_lq,
    fir_8x_lq,
    fir_2x_hq,
    fir_4x_hq,
    fir_8x_hq,
    end = fir_8x_hq
  };
  /**
   * @brief State structure for sample rate converter
   * @tparam signal_t Type of signal samples
   */
  template <typename signal_t>
  struct State {
    /**
     * @brief Index for storing input samples
     */
    size_t iStoreIn;
    /**
     * @brief Index for storing output samples
     */
    size_t iStoreOut;
    /**
     * @brief Delay line for interpolation filter (at base sample rate).
     */
    std::array<Stereo<signal_t>, nDelayLine * 2> dlInterpolation;
    /**
     * @brief Delay line for antialiasing (stores processed samples at high
     * sample rate).
     */
    std::array<Stereo<signal_t>, nDelayLine * 2> dlAntialiasing;
  };
  /**
   * @brief Coefficients structure for sample rate converter
   * @tparam signal_t Type of signal samples
   */
  template <typename signal_t>
  struct Coeffs {
    /**
     * @brief Flag to disable sample rate conversion
     */
    bool disable = false;
    /**
     * @brief Oversampling factor
     */
    size_t osFactor = 1;
    /**
     * @brief FIR filter length multiplier
     */
    size_t osFirLenMult = 12;
    /**
     * @brief FIR filter length
     */
    size_t n = 12;
    /**
     * @brief High sampling rate
     */
    signal_t fsHi = 48000;
    /**
     * @brief FIR filter coefficients
     */
    std::array<signal_t, nDelayLine> b;
  };
  /**
   * @brief Sample rate converter class
   * @tparam signal_t Type of signal samples
   */
  template <typename signal_t>
  struct SampleRateConverter {
    /**
     * @brief Reference to the plugin
     */
    NTFX_PLUGIN<signal_t>& plug;
    /**
     * @brief State of the sample rate converter
     */
    State<signal_t> state;
    /**
     * @brief Coefficients of the sample rate converter
     */
    Coeffs<signal_t> coeffs;
    /**
     * @brief Oversampling mode
     */
    oversamplingMode mode;
    /**
     * @brief Sampling rate
     */
    float fs;

    /**
     * @brief Constructor
     * @param plug Reference to the plugin
     */
    SampleRateConverter(NTFX_PLUGIN<signal_t>& plug) : plug(plug) { }

    /**
     * @brief Process audio samples through the sample rate converter
     * @param x Input audio samples
     * @return Processed audio samples
     */
    Stereo<signal_t> process(Stereo<signal_t> x) {
      if (this->coeffs.disable) { return this->plug.process(x); }
      this->state.dlInterpolation[this->state.iStoreIn]              = x;
      this->state.dlInterpolation[this->state.iStoreIn + nDelayLine] = x;
      if (++this->state.iStoreIn >= nDelayLine) { this->state.iStoreIn = 0; }
      auto iReadIn = this->state.iStoreIn + nDelayLine;
      for (size_t i = 0; i < this->coeffs.osFactor; i++) {
        Stereo<signal_t> accum;
        for (size_t j = 0; j < this->coeffs.osFirLenMult; j++) {
          accum += this->coeffs.b[j * this->coeffs.osFactor + i]
              * this->state.dlInterpolation[iReadIn - j];
        }
        auto xProc = accum * this->coeffs.osFactor;
        auto yProc = this->plug.process(xProc);
        this->state.dlAntialiasing[this->state.iStoreOut]              = yProc;
        this->state.dlAntialiasing[this->state.iStoreOut + nDelayLine] = yProc;
        if (++this->state.iStoreOut >= nDelayLine) {
          this->state.iStoreOut = 0;
        }
      }
      auto iReadOut = this->state.iStoreOut + nDelayLine
          - this->coeffs.osFactor * this->coeffs.osFirLenMult;
      Stereo<signal_t> accum;
      for (size_t i = 0; i < this->coeffs.osFactor * this->coeffs.osFirLenMult;
          i++) {
        accum += this->coeffs.b[i] * this->state.dlAntialiasing[iReadOut + i];
      }
      return accum;
    }

    /**
     * @brief Update coefficients based on current mode
     */
    inline void update() {
      switch (this->mode) {

        // TODO: IIR oversampling
        // case iir_2x:
        //   this->coeffs.osFactor = 2;
        //   break;
        // case iir_4x:
        //   this->coeffs.osFactor = 4;
        //   break;
        // case iir_8x:
        //   this->coeffs.osFactor = 8;
        //   break;
      case fir_2x_lq:
        this->coeffs.osFactor     = 2;
        this->coeffs.osFirLenMult = oversamplingFirMultLq;
        this->coeffs.disable      = false;
        break;
      case fir_4x_lq:
        this->coeffs.osFactor     = 4;
        this->coeffs.osFirLenMult = oversamplingFirMultLq;
        this->coeffs.disable      = false;
        break;
      case fir_8x_lq:
        this->coeffs.osFactor     = 8;
        this->coeffs.osFirLenMult = oversamplingFirMultLq;
        this->coeffs.disable      = false;
        break;
      case fir_2x_hq:
        this->coeffs.osFactor     = 2;
        this->coeffs.osFirLenMult = oversamplingFirMultHq;
        this->coeffs.disable      = false;
        break;
      case fir_4x_hq:
        this->coeffs.osFactor     = 4;
        this->coeffs.osFirLenMult = oversamplingFirMultHq;
        this->coeffs.disable      = false;
        break;
      case fir_8x_hq:
        this->coeffs.osFactor     = 8;
        this->coeffs.osFirLenMult = oversamplingFirMultHq;
        this->coeffs.disable      = false;
        break;
      default:
      case disable:
        this->coeffs.osFactor     = 1;
        this->coeffs.osFirLenMult = 1;
        this->coeffs.disable      = true;
      }
      this->coeffs.fsHi = this->fs * this->coeffs.osFactor;
      this->coeffs.n    = this->coeffs.osFactor * this->coeffs.osFirLenMult;
      if (!this->coeffs.disable) {
        auto b = windowMethod<signal_t>(fc, this->coeffs.n, this->coeffs.fsHi);
        std::fill(this->coeffs.b.begin(), this->coeffs.b.end(), 0.0);
        for (size_t i = 0; i < this->coeffs.n; i++) {
          this->coeffs.b[i] = b[i];
        }
      }
    }

    /**
     * @brief Reset the sample rate converter and change the sample rate.
     * @param fs Sampling rate
     */
    inline void reset(float fs) {
      this->fs              = fs;
      this->state.iStoreIn  = 0;
      this->state.iStoreOut = 0;
      std::fill(this->state.dlAntialiasing.begin(),
          this->state.dlAntialiasing.end(),
          0.0);
      std::fill(this->state.dlInterpolation.begin(),
          this->state.dlInterpolation.end(),
          0.0);
      this->update();
    }
  };
}
}