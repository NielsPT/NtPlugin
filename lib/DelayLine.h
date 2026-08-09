
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

#include "gcem.hpp"
#include "lib/Audio.h"
#include "lib/Component.h"
#include "lib/Glider.h"
#include <array>
#include <vector>

namespace NtFx {
namespace Delay {
  template <double dlLen_ms, typename T = Audio>
  struct Base : public ComponentBase<T> {
    constexpr const static int nDl = dlLen_ms * 192 * 8;
    T* dl { nullptr };
    signal_t t_ms { 0 };
    int _n { 0 };
    int _i { 0 };

    virtual void store(T x) noexcept {
      this->dl[this->_i++] = x;
      if (this->_i >= nDl) { this->_i = 0; }
    }

    virtual T read() noexcept {
      auto _i = this->_i - this->_n;
      if (_i < 0) { _i += nDl; }
      return this->dl[_i];
    }

    virtual T process(T x) noexcept override {
      store(x);
      if (this->_n == 0) { return x; }
      return read();
    }

    virtual void update() noexcept override {
      this->_n = int(gcem::floor(this->t_ms * 0.001 * this->fs));
      if (this->_n >= nDl) { this->_n = nDl; }
    }

  protected:
    Base() { }
  };

  template <double dlLen_ms, typename T = Audio>
  struct Short : public Base<dlLen_ms, T> {
    constexpr const static int nDl = Base<dlLen_ms, T>::nDl;
    std::array<T, nDl> dl;

    virtual T process(T x) noexcept override {
      // this->dl[this->_i++] = x;
      // if (this->_i >= nDl) { this->_i = 0; }
      this->store(x);
      if (this->_n == 0) { return x; }
      // auto _i = this->_i - this->_n;
      // if (_i < 0) { _i += nDl; }
      // return this->dl[_i];
      return this->read();
    }

    virtual void update() noexcept override {
      this->_n = int(gcem::floor(this->t_ms * 0.001 * this->fs));
      if (this->_n >= nDl) { this->_n = nDl; }
    }

    virtual void reset(float fs) noexcept override {
      this->fs = fs;
      std::fill(this->dl.begin(), this->dl.end(), 0);
      this->update();
    }
  };

  template <double dlLen_ms, typename T = Audio>
  struct Long : public ComponentBase<T> {
    constexpr const static int nDl = dlLen_ms * 192 * 8;
    std::vector<T> dl;
    signal_t t_ms { 0 };
    int _n { 0 };
    int _i { 0 };

    virtual T process(T x) noexcept override {
      this->dl[this->_i++] = x;
      if (this->_i >= nDl) { this->_i = 0; }
      if (this->_n == 0) { return x; }
      auto _i = this->_i - this->_n;
      if (_i < 0) { _i += nDl; }
      return this->dl[_i];
    }

    virtual void update() noexcept override {
      this->_n = int(gcem::floor(this->t_ms * 0.001 * this->fs));
      if (this->_n >= nDl) { this->_n = nDl; }
    }

    virtual void reset(float fs) noexcept override {
      this->fs = fs;
      this->update();
      this->dl.resize(gcem::ceil(dlLen_ms * 0.001 * this->fs));
      this->dl.clear();
    }
  };

  template <double dlLen_ms, typename T = Audio>
  struct ShortGlided : public Base<dlLen_ms, T> {
    constexpr const static int nDl = dlLen_ms * 192 * 8;
    std::array<T, nDl> dl;
    ExpGlider t_ms { 0 };
    // int _i { 0 };

    virtual T process(T x) noexcept override {
      // this->t_ms.process();
      // this->dl[this->_i++] = x;
      // if (this->_i >= nDl) { this->_i = 0; }
      // int n { 0 };
      // n = int(this->t_ms.pr * 0.001 * this->fs);
      // if (n == 0) { return x; }
      // if (n >= nDl) { n = nDl; }
      // auto _i = this->_i - n;
      // if (_i < 0) { _i += nDl; }
      // return this->dl[_i];

      this->store(x);
      this->_n = int(this->t_ms.pr * 0.001 * this->fs);
      if (this->_n == 0) { return x; }
      if (this->_n >= nDl) { this->_n = nDl; }
      if (this->_n == 0) { return x; }
      return this->read();
    }

    virtual void update() noexcept override { this->t_ms.update(this->fs); }

    virtual void reset(float fs) noexcept override {
      this->fs = fs;
      std::fill(this->dl.begin(), this->dl.end(), 0);
      this->update();
    }
  };

  template <int nDl, typename T, int nCoeffs = 4>
  struct ShortFract : public ComponentBase<T> {
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

  struct Mod : public NtFx::ComponentBase<Audio> {
    constexpr static const signal_t minRate_hz = 0.1;
    constexpr static const int tModDlMax_ms    = 10 * int(1.0 / minRate_hz);
    constexpr static const int nDl             = 192 * 8 * tModDlMax_ms;

    NtFx::Delay::ShortFract<nDl, signal_t> l;
    NtFx::Delay::ShortFract<nDl, signal_t> r;
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
      auto tmp     = gcem::sin(omegaT_rad);
      this->l.t_ms = (tmp + 1) * this->_tDelayMod_s.pr * 1000;
      this->l.update();
      tmp          = gcem::sin(omegaT_rad + this->_phaseMod_rad.pr);
      this->r.t_ms = (tmp + 1) * this->_tDelayMod_s.pr * 1000;
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
