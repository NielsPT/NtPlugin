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
constexpr int maxSampleDLineLen = 192 * 8;
constexpr int maxMsDLineLen     = 1000;

template <typename signal_t>
struct RmsSensor : public Component<Stereo<signal_t>> {
  bool resetAccums   = false;
  int msDLineLen     = maxMsDLineLen;
  int sampleDLineLen = maxSampleDLineLen;
  std::array<Stereo<signal_t>, maxSampleDLineLen> samleDLine;
  std::array<Stereo<signal_t>, maxMsDLineLen> msDLine;
  Stereo<signal_t> sampleAccum;
  Stereo<signal_t> msAccum;
  int sampleIdx;
  int msIdx;

  virtual Stereo<signal_t> process(Stereo<signal_t> x) noexcept override {
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

  virtual void reset(float fs) noexcept override {
    this->fs             = fs;
    this->sampleDLineLen = fs / 1000;
    this->resetAccums    = true;
    this->update();
  }

  Stereo<signal_t> getRms() const noexcept {
    Stereo<signal_t> y = {
      gcem::sqrt(signal_t(2.0) * this->msAccum.l
          / signal_t(this->sampleDLineLen * this->msDLineLen)),
      gcem::sqrt(signal_t(2.0) * this->msAccum.r
          / signal_t(this->sampleDLineLen * this->msDLineLen)),
    };
    if (y != y) { y = signal_t(0.0); }
    return y;
  }

  void setRmsAvgTime(int t_ms) {
    if (t_ms == this->msDLineLen) { return; }
    this->msDLineLen  = t_ms;
    this->resetAccums = true;
  }
};
}