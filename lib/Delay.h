
/**
 * @file DelayLine.h
 * @author Niels Thøgersen (niels.thoegersen@gmail.com)
 * @brief Audio delay lines.
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
 *
 * You are free to download, build and use this code for commercial
 * purposes. Just don't resell it or a build of it, modified or otherwise.
 */

#include "lib/Audio.h"
#include "lib/Component.h"
#include "lib/Glider.h"
#include "lib/gcem.h"
#include <array>
#include <vector>

namespace NtFx {
namespace Delay {
  /**
   * @brief Fixed-size short delay line with compile-time length.
   *
   * A high-performance delay line for short delays where the maximum delay
   * time is known at compile time. Uses a fixed-size array for predictable
   * memory usage.
   *
   * @tparam dlLen_ms Maximum delay length in milliseconds (compile-time
   * constant).
   *                   Must be specified as a constexpr double value.
   * @tparam T Sample type, typically Audio (stereo) or float/double (mono).
   *           Defaults to Audio for stereo processing.
   *
   * The delay time is controlled via the `t_ms` parameter and updated via the
   * `update()` method. Processing is performed in the `process()` method.
   */
  template <double dlLen_ms, typename T = Audio>
  struct Short final : public ComponentBase<T> {
    constexpr const static int nDl = dlLen_ms * 192
        * 8; /**< Total samples in delay line (48 kHz * 8x oversampling) */
    std::array<T, nDl> dl; /**< Fixed circular buffer for delay samples */
    signal_t t_ms { 0 };   /**< Desired delay time in milliseconds */
    int _n { 0 };          /**< Current delay in samples */
    int _i { 0 };          /**< Write pointer in circular buffer */

    /**
     * @brief Process a single sample through the delay line.
     *
     * Writes the input sample to the buffer and reads from the delayed
     * position. Uses modulo arithmetic to handle circular buffer wrap-around.
     *
     * @param x Input sample (Audio stereo or scalar type)
     * @return Delayed sample from the delay buffer
     */
    T process(T x) noexcept override {
      this->dl[this->_i++] = x;
      if (this->_i >= nDl) { this->_i = 0; }
      if (this->_n == 0) { return x; }
      auto i = this->_i - this->_n;
      if (i < 0) { i += nDl; }
      return this->dl[i];
    }

    /**
     * @brief Update the internal delay time based on the t_ms parameter.
     *
     * Converts the delay time in milliseconds to the number of samples
     * based on the current sample rate. Clamps to maximum buffer size.
     */
    void update() noexcept override {
      this->_n = int(gcem::floor(this->t_ms * 0.001 * this->_fs));
      if (this->_n >= nDl) { this->_n = nDl; }
    }

    /**
     * @brief Initialize the delay line with a sample rate.
     *
     * Clears the buffer and updates internal parameters for the given sample
     * rate.
     *
     * @param fs Sample rate in Hz (e.g., 48000)
     */
    void reset(signal_t fs) noexcept override {
      this->_fs = fs;
      std::fill(this->dl.begin(), this->dl.end(), 0);
      this->update();
    }
  };

  /**
   * @brief Dynamic delay line for longer delays with runtime-allocated buffer.
   *
   * Similar to Short but allocates memory dynamically via std::vector.
   * Useful for delays longer than practical compile-time constants or when
   * memory allocation is deferred to initialization.
   *
   * @tparam dlLen_ms Maximum delay length in milliseconds (compile-time
   * constant).
   * @tparam T Sample type (Audio for stereo, or scalar for mono).
   */
  template <double dlLen_ms, typename T = Audio>
  struct Long final : public ComponentBase<T> {
    int nDl {
      1
    }; /**< Number of samples in delay buffer (runtime-determined) */
    std::vector<T> dl;   /**< Dynamic circular buffer for delay samples */
    signal_t t_ms { 0 }; /**< Desired delay time in milliseconds */
    int _n { 0 };        /**< Current delay in samples */
    int _i { 0 };        /**< Write pointer in circular buffer */

    /**
     * @brief Process a single sample through the dynamic delay line.
     */
    T process(T x) noexcept override {
      this->dl[this->_i++] = x;
      if (this->_i >= this->nDl) { this->_i = 0; }
      if (this->_n == 0) { return x; }
      auto i = this->_i - this->_n;
      if (i < 0) { i += this->nDl; }
      return this->dl[i];
    }

    /**
     * @brief Update the internal delay time in samples.
     */
    void update() noexcept override {
      this->_n = int(gcem::floor(this->t_ms * 0.001 * this->_fs));
      if (this->_n >= this->nDl) { this->_n = this->nDl; }
    }

    /**
     * @brief Initialize the delay line, allocating the buffer based on sample
     * rate.
     *
     * @param fs Sample rate in Hz
     */
    void reset(signal_t fs) noexcept override {
      this->_fs = fs;
      this->nDl = int(gcem::ceil(dlLen_ms * 0.001 * this->_fs));
      this->dl.resize(this->nDl);
      std::fill(this->dl.begin(), this->dl.end(), 0);
      this->update();
    }
  };

  /**
   * @brief Fixed-size delay line with exponentially-smoothed delay time.
   *
   * Combines the performance of Short with smooth delay time transitions.
   * The delay parameter uses ExpGlider for exponential ramping, preventing
   * discontinuities in the delay time. Suitable for modulation effects.
   *
   * @tparam dlLen_ms Maximum delay length in milliseconds
   * @tparam T Sample type (Audio or scalar)
   */
  template <double dlLen_ms, typename T = Audio>
  struct ShortGlided final : public ComponentBase<T> {
    constexpr const static int _nDl =
        dlLen_ms * 192 * 8; /**< Total samples in delay line */
    std::array<T, _nDl> dl; /**< Fixed circular buffer */
    ExpGlider t_ms { 0 }; /**< Smoothed delay time parameter in milliseconds */
    int _i { 0 };         /**< Write pointer in circular buffer */

    /**
     * @brief Process a sample with smoothed delay time.
     *
     * Advances the glider, then reads from the smoothed delay position.
     */
    T process(T x) noexcept override {
      this->t_ms.process();
      this->dl[this->_i++] = x;
      if (this->_i >= this->_nDl) { this->_i = 0; }
      int n { 0 };
      n = int(this->t_ms.pr * 0.001 * this->_fs);
      if (n == 0) { return x; }
      if (n >= this->_nDl) { n = this->_nDl; }
      auto i = this->_i - n;
      if (i < 0) { i += this->_nDl; }
      return this->dl[i];
    }

    /// @brief Update the glider's sample rate dependent parameters.
    void update() noexcept override { this->t_ms.update(this->_fs); }

    /**
     * @brief Initialize with sample rate and clear buffer.
     */
    void reset(signal_t fs) noexcept override {
      this->_fs = fs;
      std::fill(this->dl.begin(), this->dl.end(), 0);
      this->update();
    }
  };

  /**
   * @brief Dynamic delay line with exponentially-smoothed delay time.
   *
   * Combines Long's dynamic memory allocation with ExpGlider smoothing.
   * Buffer is allocated at runtime based on sample rate and dlLen_ms.
   *
   * @tparam dlLen_ms Maximum delay length in milliseconds
   * @tparam T Sample type (Audio or scalar)
   */
  template <double dlLen_ms, typename T = Audio>
  struct LongGlided final : public ComponentBase<T> {
    std::vector<T> dl;    /**< Dynamic circular buffer */
    ExpGlider t_ms { 0 }; /**< Smoothed delay time parameter in milliseconds */
    int _nDl { 1 };       /**< Number of samples in delay buffer */
    int _n { 0 }; /**< Current delay in samples (reserved for compatibility) */
    int _i { 0 }; /**< Write pointer in circular buffer */

    /**
     * @brief Process a sample with smoothed, dynamic delay time.
     */
    T process(T x) noexcept override {
      this->t_ms.process();
      this->dl[this->_i++] = x;
      if (this->_i >= this->_nDl) { this->_i = 0; }
      int n { 0 };
      n = int(this->t_ms.pr * 0.001 * this->_fs);
      if (n == 0) { return x; }
      if (n >= this->_nDl) { n = this->_nDl; }
      auto _i = this->_i - n;
      if (_i < 0) { _i += this->_nDl; }
      return this->dl[_i];
    }

    /**
     * @brief Update the glider's sample rate dependent parameters.
     */
    void update() noexcept override { this->t_ms.update(this->_fs); }

    /**
     * @brief Initialize with sample rate, allocating the dynamic buffer.
     */
    void reset(signal_t fs) noexcept override {
      this->_fs = fs;
      this->update();
      this->_nDl = int(gcem::ceil(dlLen_ms * 0.001 * this->_fs));
      this->dl.resize(this->_nDl);
      std::fill(this->dl.begin(), this->dl.end(), 0);
    }
  };

  /**
   * @brief Fixed delay line with fractional sample interpolation.
   *
   * Provides high-quality delay time control using polynomial Lagrange
   * interpolation. Allows sub-sample accurate delay times without aliasing.
   * The duplicated circular buffer (nDl * 2) simplifies index arithmetic.
   *
   * @tparam nDl Maximum number of samples in delay buffer (buffer size is nDl
   * * 2)
   * @tparam T Sample type (typically signal_t for high precision)
   * @tparam nCoeffs Number of interpolation coefficients (default 4 for cubic)
   */
  template <int nDl, typename T = Audio, int nCoeffs = 4>
  struct ShortFract final : public ComponentBase<T> {
    std::array<T, nCoeffs> coeffs; /**< Lagrange interpolation coefficients */
    std::array<T, nDl * 2>
        dl; /**< Duplicated circular buffer for wrap-around-free indexing */
    signal_t t_ms { 0 }; /**< Desired delay time in milliseconds */
    int _i { 0 };        /**< Write pointer in circular buffer */
    int _n { 1 };        /**< Integer part of delay in samples */

    /**
     * @brief Process a single sample with fractional delay interpolation.
     *
     * Writes the sample to both buffer locations (for wrap-around safety),
     * then reads from the interpolated delay position using precomputed
     * Lagrange coefficients.
     *
     * @param x Input sample
     * @return Interpolated delayed sample
     */
    T process(T x) noexcept override {
      this->dl[size_t(this->_i)]       = x;
      this->dl[size_t(this->_i + nDl)] = x;
      this->_i++;
      this->_i      = (this->_i < nDl ? this->_i : 0);
      int readIndex = this->_i - this->_n;
      readIndex += nDl * (readIndex < nDl);
      T acc0 { 0 };
      for (int i = 0; i < nCoeffs; i++) {
        acc0 += this->coeffs[i] * this->dl[readIndex - i];
      }
      return acc0;
    }

    /**
     * @brief Update delay time and compute Lagrange interpolation
     * coefficients.
     *
     * Converts delay time from milliseconds to samples, separating the integer
     * and fractional parts. Computes Lagrange basis polynomials for the
     * fractional portion. Specialized for nCoeffs=4 (cubic interpolation) with
     * general fallback.
     *
     * The fractional delay is centered at D which adjusts for filter group
     * delay.
     */
    void update() noexcept override {
      auto delay_samples = this->t_ms * this->_fs * 0.001;
      delay_samples      = delay_samples < 1.0f ? 1.0f : delay_samples;
      auto n             = int(delay_samples);
      float d            = delay_samples - n;
      this->_n           = n;
      if (d < 0.0 || d >= 1.0) { return; }
      const signal_t D = d + gcem::floor(nCoeffs / 2.0) - 1.0;
      if constexpr (nCoeffs == 4) {
        // Cubic Lagrange interpolation (4 coefficients)
        signal_t acc = 1.0;
        acc *= (D - signal_t(1)) / (signal_t(0) - signal_t(1));
        acc *= (D - signal_t(2)) / (signal_t(0) - signal_t(2));
        acc *= (D - signal_t(3)) / (signal_t(0) - signal_t(3));
        this->coeffs[0] = acc;
        acc             = 1.0;
        acc *= (D - signal_t(0)) / (signal_t(1) - signal_t(0));
        acc *= (D - signal_t(2)) / (signal_t(1) - signal_t(2));
        acc *= (D - signal_t(3)) / (signal_t(1) - signal_t(3));
        this->coeffs[1] = acc;
        acc             = 1.0;
        acc *= (D - signal_t(0)) / (signal_t(2) - signal_t(0));
        acc *= (D - signal_t(1)) / (signal_t(2) - signal_t(1));
        acc *= (D - signal_t(3)) / (signal_t(2) - signal_t(3));
        this->coeffs[2] = acc;
        acc             = 1.0;
        acc *= (D - signal_t(0)) / (signal_t(3) - signal_t(0));
        acc *= (D - signal_t(1)) / (signal_t(3) - signal_t(1));
        acc *= (D - signal_t(2)) / (signal_t(3) - signal_t(2));
        this->coeffs[3] = acc;
        return;
      }
      // Generic Lagrange interpolation for any nCoeffs
      for (size_t i = 0; i < nCoeffs; i++) {
        signal_t acc = 1.0;
        for (size_t j = 0; j < nCoeffs; j++) {
          if (j != i) {
            acc *= (D - signal_t(j)) / (signal_t(i) - signal_t(j));
          }
        }
        this->coeffs[i] = acc;
      }
    }

    void reset(signal_t fs) noexcept override {
      this->_fs = fs;
      std::fill(this->dl.begin(), this->dl.end(), 0);
      this->update();
    }
  };

  /**
   * @brief Stereo modulated delay line with LFO modulation.
   *
   * Provides a classic chorus/flanger effect by modulating the delay time
   * with sinusoidal LFO waveforms. The left and right channels are modulated
   * 90 degrees out of phase by default, creating a stereo width effect.
   *
   * Key parameters:
   * - `fMod_hz`: Modulation frequency (LFO rate)
   * - `depth_p`: Modulation depth as a percentage of maximum delay
   * - `phaseMod_deg`: Phase offset between left and right channels (degrees)
   */
  struct Mod final : public ComponentBase<Audio> {
    constexpr static const signal_t minRate_hz =
        0.1; /**< Minimum LFO frequency (Hz) */
    constexpr static const int tModDlMax_ms =
        10 * int(1.0 / minRate_hz); /**< Max delay in ms (100ms at 0.1Hz) */
    constexpr static const int nDl =
        192 * 8 * tModDlMax_ms; /**< Total samples for modulated delay */

    NtFx::Delay::ShortFract<nDl, signal_t>
        l; /**< Left channel delay with fractional interpolation */
    NtFx::Delay::ShortFract<nDl, signal_t>
        r; /**< Right channel delay with fractional interpolation */
    NtFx::ExpGlider _tDelayMod_s;  /**< Smoothed modulation depth in seconds */
    NtFx::ExpGlider _phaseMod_rad; /**< Smoothed phase offset in radians */
    NtFx::ExpGlider fMod_hz {
      0.25
    }; /**< Smoothed LFO frequency in Hz (default 0.25 Hz) */
    signal_t depth_p { 25 };      /**< Modulation depth as percentage (0-100) */
    signal_t phaseMod_deg { 90 }; /**< Phase offset between L/R in degrees */
    signal_t _tSample { 1 };      /**< Sample period for LFO calculation */
    signal_t _t { 0 }; /**< Current LFO phase accumulator (0 to 1/fMod_hz) */

    /**
     * @brief Process stereo audio with modulated delay.
     *
     * Advances all gliders and LFO phase accumulator. Generates sinusoidal
     * modulation waveforms with phase offset for L/R channels. Updates both
     * delay lines and processes the stereo pair independently.
     *
     * @param x Stereo input (Audio with .l and .r members)
     * @return Modulated delayed stereo output
     */
    Audio process(Audio x) noexcept override {
      this->_tDelayMod_s.process();
      this->_phaseMod_rad.process();
      this->fMod_hz.process();
      signal_t omegaT_rad = 2 * GCEM_PI * this->fMod_hz.pr * this->_t;
      this->_t += this->_tSample;
      if (this->_t >= 1 / this->fMod_hz.pr) { this->_t = 0; }
      // Left channel: modulate delay with unshifted sine
      auto tmp     = gcem::sin(omegaT_rad);
      this->l.t_ms = (tmp + 1) * this->_tDelayMod_s.pr * 1000;
      this->l.update();
      // Right channel: modulate delay with phase-shifted sine
      tmp          = gcem::sin(omegaT_rad + this->_phaseMod_rad.pr);
      this->r.t_ms = (tmp + 1) * this->_tDelayMod_s.pr * 1000;
      this->r.update();
      return { this->l.process(x.l), this->r.process(x.r) };
    }

    /**
     * @brief Update glider targets and recalculate sample period.
     *
     * Computes the smoothed modulation depth from the depth percentage and
     * modulation frequency. Converts phase offset from degrees to radians.
     * Enforces minimum LFO frequency of 0.1 Hz. Updates all gliders.
     */
    void update() noexcept override {
      // Modulation depth = depth_p% * max_delay_ms, adjusted for frequency
      this->_tDelayMod_s.ui = this->depth_p * tModDlMax_ms / 2000000;
      this->_tDelayMod_s.ui /= this->fMod_hz.ui;
      this->_phaseMod_rad.ui = this->phaseMod_deg * GCEM_PI / 180;
      this->_tSample         = 1 / this->_fs;
      if (this->fMod_hz.ui < 0.1) { this->fMod_hz.ui = 0.1; }
      this->fMod_hz.update(this->_fs);
      this->_phaseMod_rad.update(this->_fs);
      this->_tDelayMod_s.update(this->_fs);
    }

    /**
     * @brief Initialize with sample rate, resetting both channels.
     *
     * @param fs Sample rate in Hz
     */
    void reset(signal_t fs) noexcept override {
      this->_fs = fs;
      this->l.reset(fs);
      this->r.reset(fs);
    }
  };
}
}
