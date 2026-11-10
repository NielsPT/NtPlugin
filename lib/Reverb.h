#pragma once

/**
 * @file Reverb.h
 * @author Niels Thøgersen (niels.thoegersen@gmail.com)
 * @brief Various bits and bobs for reverb algorithms.
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
#include "lib/Delay.h"
#include "lib/FirstOrder.h"
#include "lib/gcem.h"
#include "lib/primes.h"
#include "lib/utils.h"
#include <array>
#include <bit> // C++20 for std::popcount
#include <csignal>
#include <cstddef>

namespace NtFx {
namespace Reverb {

  template <size_t N>
  struct FeedbackMatrix {
    static_assert(N > 0, "N must be positive");
    static_assert(
        (N & (N - 1)) == 0, "N must be a power of 2 (2, 4, 8, 16, 32...)");

    using Matrix_t = std::array<std::array<int8_t, N>, N>;
    static consteval Matrix_t _hadamard() {
      Matrix_t mat { };
      for (size_t i = 0; i < N; ++i) {
        for (size_t j = 0; j < N; ++j) {
          // The bitwise AND determines the "intersection" of the basis vectors
          // If the number of set bits in (i & j) is even, sign is +1
          // If odd, sign is -1
          int pop   = std::popcount(i & j);
          mat[i][j] = (pop % 2 == 0) ? 1 : -1;
        }
      }
      return mat;
    }

    static consteval Matrix_t _diagonal() {
      Matrix_t mat { };
      for (size_t i = 0; i < N; i++) {
        for (size_t j = 0; j < N; j++) {
          mat[i][j] = 0;
          if (i == j) { mat[i][j] = 1; }
        }
      }
      return mat;
    }

    static consteval Matrix_t _hadMinusDiag() {
      auto mat = _hadamard();
      for (size_t i = 0; i < N; i++) {
        for (size_t j = 0; j < N; j++) {
          if (i == j) { mat[i][j] = 0; }
        }
      }
      return mat;
    }

    static constexpr Matrix_t hadamard { _hadamard() };
    static constexpr Matrix_t diagonal { _diagonal() };
    static constexpr Matrix_t hadMinusDiag { _hadMinusDiag() };
  };

  template <double s_tMax_ms = 13.0>
  struct SchroederAllPass final : public ComponentBase<Audio> {
    Delay::Short<s_tMax_ms> xDl;
    Audio yState { 0 };
    signal_t g_lin { 0.707 };
    signal_t t_ms { 6 };
    Audio process(Audio x) noexcept override {
      auto tmp =
          -this->g_lin * x + this->xDl.process(x + this->g_lin * this->yState);
      this->yState = tmp;
      return tmp;
    }
    void update() noexcept override {
      if (this->t_ms > s_tMax_ms) { this->t_ms = s_tMax_ms; }
      this->xDl.t_ms = this->t_ms;
      this->xDl.update();
    }
    void reset(signal_t fs) noexcept override {
      this->_fs = fs;
      this->xDl.reset(fs);
      this->update();
    }
  };

  template <size_t n, double tMax_ms = 100.0>
  struct EarlyReflections final : public ComponentBase<Audio> {
    NtFx::Delay::LongMultiTap<1000.0, n, signal_t> dlL;
    NtFx::Delay::LongMultiTap<1000.0, n, signal_t> dlR;
    signal_t decayDepth { 1 };
    signal_t tBegin_ms { 7 };
    signal_t tEnd_ms { tMax_ms };
    Audio process(Audio x) noexcept override {
      return Audio({ this->dlL.process(x.l), this->dlR.process(x.r) })
          / gcem::sqrt(signal_t(n));
    }
    void update() noexcept override {
      auto t =
          NtFx::primespace<signal_t>(this->tBegin_ms, this->tEnd_ms * 0.9, n);
      for (size_t i = 0; i < n; i++) {
        this->dlL.t_ms[i] = t[i];
        this->dlR.t_ms[i] = signal_t(NtFx::nextPrime(size_t(t[i])));
      }
      this->dlL.setGainToDecay(this->decayDepth);
      this->dlR.setGainToDecay(this->decayDepth);
      this->dlL.update();
      this->dlR.update();
    }
    void reset(signal_t fs) noexcept override {
      this->dlL.reset(fs);
      this->dlR.reset(fs);
    }
  };

  template <size_t n = 16, double tMax_ms = 100.0>
  struct DelayNetwork final : public ComponentBase<Audio> {
    // enum class FbMode { hadamard, diagonal, hadMinusDiag };
    std::array<Delay::Short<tMax_ms, signal_t>, n> _dls;
    NtFx::Delay::Mod mod;
    NtFx::FirstOrder::Filter<FirstOrder::Shape::hpf, Audio> dcBlock;
    std::array<NtFx::FirstOrder::Filter<FirstOrder::Shape::hpf, signal_t>, n>
        dampHpf;
    std::array<signal_t, n> _state;
    std::array<signal_t, n> _a;
    FeedbackMatrix<n>::Matrix_t fbMat;
    // FbMode fbMode;
    signal_t t60_s { 2 };
    signal_t tEnd_ms { tMax_ms };
    signal_t tBegin_ms { 3 };
    signal_t gDamp_db { 0 };
    signal_t gDamp_lin { 0 };
    Audio process(Audio x) noexcept override {
      auto yDcBlock = this->dcBlock.process(x);
      auto yMod     = this->mod.process(yDcBlock);
      std::array<signal_t, n> yDls;
      // if (this->fbMode == FbMode::diagonal) {
      // for (size_t i = 0; i < n; i += 2) { this->_state[i] += yMod.r / 10; }
      // for (size_t i = 1; i < n; i += 2) { this->_state[i] += yMod.l / 10; }
      // } else {
      for (size_t i = 0; i < n; i += 4) { this->_state[i] += yMod.r; }
      for (size_t i = 1; i < n; i += 4) { this->_state[i] += yMod.l; }
      // }
      for (size_t i = 0; i < n; i++) {
        yDls[i] = this->_dls[i].process(this->_state[i]);
      }
      for (size_t i = 1; i < n; i++) {
        signal_t acc { 0 };
        for (size_t j = 1; j < n; j++) {
          acc += yDls[j] * FeedbackMatrix<n>::hadamard[i][j];
        }
        auto yDamp      = this->dampHpf[i].process(acc) * this->gDamp_lin + acc;
        this->_state[i] = yDamp * this->_a[i];
      }
      Audio y;
      // if (this->fbMode == FbMode::diagonal) {
      //   for (size_t i = 0; i < n; i += 2) { y.l += yDls[i]; }
      //   for (size_t i = 1; i < n; i += 2) { y.r += yDls[i]; }
      // } else {
      for (size_t i = 2; i < n; i += 4) { y.l += yDls[i]; }
      for (size_t i = 3; i < n; i += 4) { y.r += yDls[i]; }
      // }
      return y;
    }
    void update() noexcept override {
      // switch (fbMode) {
      // case FbMode::hadamard:
      //   this->fbMat = FeedbackMatrix<n>::hadamard;
      //   break;
      // case FbMode::diagonal:
      //   this->fbMat = FeedbackMatrix<n>::diagonal;
      //   break;
      // case FbMode::hadMinusDiag:
      //   this->fbMat = FeedbackMatrix<n>::hadMinusDiag;
      //   break;
      // }
      if (this->t60_s < 0.1) { this->t60_s = 0.1; }
      auto t_ms =
          NtFx::primespace<signal_t>(this->tBegin_ms, this->tEnd_ms * 0.9, n);
      for (size_t i = 0; i < n; i++) {
        this->_dls[i].t_ms = t_ms[i];
        this->_dls[i].update();
        while (!isPrime(this->_dls[i]._n)) { this->_dls[i]._n++; }
        this->_a[i] = gcem::pow(
            signal_t(10), signal_t(-3) * t_ms[i] * 0.001 / (this->t60_s));
        // if (this->fbMode != FbMode::diagonal) {
        this->_a[i] /= gcem::sqrt(signal_t(n));
        // }
      }
      this->gDamp_lin = invDb(-this->gDamp_db) - 1;
      this->mod.update();
      for (size_t i = 0; i < n; i++) { this->dampHpf[i].update(); }
      this->dcBlock.update();
    }
    void reset(signal_t fs) noexcept override {
      this->_fs = fs;
      std::fill(this->_state.begin(), this->_state.end(), 0);
      for (size_t i = 0; i < n; i++) { this->_dls[i].reset(fs); }
      this->dcBlock.fc_hz = 10;
      this->mod.reset(fs);
      for (size_t i = 0; i < n; i++) { this->dampHpf[i].reset(fs); }
      this->dcBlock.reset(fs);
      this->update();
    }
  };
}
}