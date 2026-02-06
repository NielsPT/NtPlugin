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
#include "lib/MsRmsSensor.h"
#include <algorithm>
#include <array>

namespace NtFx {
template <typename signal_t, int dLineLen = 10, int nDLines = 5>
struct MultitapRmsSensor : public Component<Stereo<signal_t>> {
  int sampleDLineLen = maxSampleDLineLen;
  std::array<Stereo<signal_t>, maxSampleDLineLen> sampleDLine;
  std::array<std::array<Stereo<signal_t>, dLineLen>, nDLines> msDLines;
  std::array<Stereo<signal_t>, nDLines> accums;
  std::array<int, nDLines> idx;
  virtual Stereo<signal_t> process(Stereo<signal_t> x) noexcept override {
    auto x2 = x * x;
    if (x2 != x2) { x2 = signal_t(0.0); }
    this->accums[0] += x2 - this->sampleDLine[this->idx[0]];
    this->sampleDLine[this->idx[0]] = x2;
    this->idx[0]++;
    if (this->idx[0] >= this->sampleDLineLen) {
      this->idx[0] = 0;
      updateDLine(this->accums[0]);
    }
    return this->accums[0];
  }

  void updateDLine(Stereo<signal_t> x2, int dLineIdx = 1) {
    if (dLineIdx >= nDLines) return;
    this->accums[dLineIdx] +=
        x2 - this->msDLines[dLineIdx][this->idx[dLineIdx]];
    this->msDLines[dLineIdx][this->idx[dLineIdx]] = x2;
    this->idx[dLineIdx]++;
    if (this->idx[dLineIdx] >= dLineLen) {
      this->idx[dLineIdx] = 0;
      updateDLine(this->accums[dLineIdx], dLineIdx + 1);
    }
  }

  virtual void update() noexcept override { }

  virtual void reset(float fs) noexcept override {
    this->sampleDLineLen = fs / 1000;
    std::fill(this->sampleDLine.begin(), this->sampleDLine.end(), 0);
    for (auto& dl : this->msDLines) { std::fill(dl.begin(), dl.end(), 0); }
    for (auto& i : this->idx) { i = 0; }
    this->update();
  }

  Stereo<signal_t> getRms(int expo) const {
    auto n = this->sampleDLineLen * gcem::pow(dLineLen, expo);
    return {
      gcem::sqrt(signal_t(2.0) * this->accums[expo].l / n),
      gcem::sqrt(signal_t(2.0) * this->accums[expo].r / n),
    };
  }
};
}