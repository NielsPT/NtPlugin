
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
#include <array>
namespace NtFx {
namespace Delay {
  template <int dlLen, typename T = Audio>
  struct ShortDelayLine : public ComponentBase<T> {
    std::array<T, dlLen> dl;
    // TODO: Glide delay time. Separate class? Or just the way it is?
    signal_t t_ms { 0 };
    int n { 0 };
    int i { 0 };

    virtual T process(T x) noexcept override {
      this->dl[this->i++] = x;
      if (this->i >= dlLen) { this->i = 0; }
      if (this->n == 0) { return x; }
      auto _i = this->i - this->n;
      if (_i < 0) { _i += dlLen; }
      return this->dl[_i];
    }

    virtual void update() noexcept override {
      this->n = int(this->t_ms * 0.001 * this->fs);
      if (this->n >= dlLen) { this->n = dlLen; }
    }

    virtual void reset(float fs) noexcept override {
      this->fs = fs;
      std::fill(this->dl.begin(), this->dl.end(), 0);
      this->update();
    }
  };

  template <int dlLen, typename T = Audio>
  struct ShortGlideDelayLine : public ComponentBase<T> {
    std::array<T, dlLen> dl;
    ExpGlider t_ms { 0 };
    int i { 0 };

    virtual T process(T x) noexcept override {
      this->t_ms.process();
      this->dl[this->i++] = x;
      if (this->i >= dlLen) { this->i = 0; }
      int n { 0 };
      n = int(this->t_ms.pr * 0.001 * this->fs);
      if (n == 0) { return x; }
      if (n >= dlLen) { n = dlLen; }
      auto _i = this->i - n;
      if (_i < 0) { _i += dlLen; }
      return this->dl[_i];
    }

    virtual void update() noexcept override {
      this->t_ms.update(this->fs);
      this->t_ms.process();
    }

    virtual void reset(float fs) noexcept override {
      this->fs = fs;
      std::fill(this->dl.begin(), this->dl.end(), 0);
      this->update();
    }
  };

  template <int nDl, typename T, int nCoeffs = 4>
  struct ShortFractDelayLine : public ComponentBase<T> {
    std::array<T, nCoeffs> coeffs;
    T dl[nDl * 2] { 0 };
    signal_t t_ms { 0 };
    int32_t idx { 0 };
    int32_t nDelay;

    void _store(T x) {
      this->dl[idx]       = x;
      this->dl[idx + nDl] = x;
      ++this->idx;
      this->idx = (this->idx < nDl ? this->idx : 0);
    }

    virtual T process(T x) noexcept override {
      _store(x);
      int32_t readIndex = idx - this->nDelay;
      readIndex += nDl * (readIndex < nDl);
      T acc0 { 0 };
      for (int32_t i = 0; i < nCoeffs; i++) {
        acc0 += this->coeffs[i] * this->dl[readIndex - i];
      }
      return acc0;
    }

    virtual void update() noexcept override {
      auto delay_samples = this->t_ms * this->fs * 0.001;
      delay_samples      = delay_samples < 1.0f ? 1.0f : delay_samples;
      auto n             = int(delay_samples);
      float d            = delay_samples - n;
      this->nDelay       = n;
      if (d < 0.0 || d >= 1.0) { return; }
      const signal_t D = d + gcem::floor(nCoeffs / 2.0) - 1.0;
      if constexpr (nCoeffs == 4) {
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
  };

  struct ModDelayLine : public NtFx::ComponentBase<Audio> {
    constexpr static const signal_t minRate_hz = 0.1;
    constexpr static const int tModDlMax_ms    = 10 * int(1.0 / minRate_hz);
    constexpr static const int nDl             = 192 * 8 * tModDlMax_ms;

    NtFx::Delay::ShortFractDelayLine<nDl, signal_t> l;
    NtFx::Delay::ShortFractDelayLine<nDl, signal_t> r;
    NtFx::ExpGlider _tDelayMod_s;
    NtFx::ExpGlider _phaseMod_rad;
    NtFx::ExpGlider fMod_hz { 0.25 };
    signal_t depth_p { 25 };
    signal_t phaseMod_deg { 90 };
    signal_t _tSample { 1 / 48e3 };
    signal_t _t { 0 };

    Audio process(Audio x) noexcept override {
      this->_tDelayMod_s.process();
      this->_phaseMod_rad.process();
      this->fMod_hz.process();
      signal_t omegaT_rad = 2 * GCEM_PI * this->fMod_hz.pr * this->_t;
      this->_t += this->_tSample;
      if (this->_t >= 1 / this->fMod_hz.pr) { this->_t = 0; }
      this->l.t_ms = (gcem::sin(omegaT_rad) * this->_tDelayMod_s.pr
                         + this->_tDelayMod_s.pr)
          * 1000;
      this->r.t_ms = (gcem::sin(omegaT_rad + this->_phaseMod_rad.pr)
                             * this->_tDelayMod_s.pr
                         + this->_tDelayMod_s.pr)
          * 1000;
      this->l.update();
      this->r.update();
      return { this->l.process(x.l), this->r.process(x.r) };
    }

    void update() noexcept override {
      this->_tDelayMod_s.ui = this->depth_p * tModDlMax_ms / 2000000;
      this->_tDelayMod_s.ui /= this->fMod_hz.ui;
      this->_phaseMod_rad.ui = this->phaseMod_deg * GCEM_PI / 180;
      this->_tSample         = 1 / this->fs;
      if (this->fMod_hz.ui < 0.1) { this->fMod_hz.ui = 0.1; }
      this->fMod_hz.update(this->fs);
      this->_phaseMod_rad.update(this->fs);
      this->_tDelayMod_s.update(this->fs);
    }

    void reset(float fs) noexcept override {
      this->fs = fs;
      this->l.reset(fs);
      this->r.reset(fs);
    }
  };
}
}
