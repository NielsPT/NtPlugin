#pragma once

#include "Component.h"
#include "Stereo.h"
#include <algorithm>
#include <array>

namespace NtFx {
constexpr int maxMsDLineLen = 192 * 8;
template <typename signal_t, int dLineLen = 10, int nDLines = 5>
struct RmsSensor : public Component<Stereo<signal_t>> {
  int msDLineLen = maxMsDLineLen;
  std::array<Stereo<signal_t>, maxMsDLineLen> msDLine;
  std::array<std::array<Stereo<signal_t>, dLineLen>, nDLines> dLines;
  std::array<Stereo<signal_t>, nDLines> accums;
  std::array<int, nDLines> idx;
  virtual Stereo<signal_t> process(Stereo<signal_t> x) noexcept override {
    auto x2 = x * x;
    if (x2 != x2) { x2 = signal_t(0.0); }
    this->accums[0] += x2 - this->msDLine[this->idx[0]];
    this->msDLine[this->idx[0]] = x2;
    this->idx[0]++;
    if (this->idx[0] >= this->msDLineLen) {
      this->idx[0] = 0;
      updateDLine(this->accums[0]);
    }
    return this->accums[0];
  }

  void updateDLine(Stereo<signal_t> x2, int dLineIdx = 1) {
    if (dLineIdx >= nDLines) return;
    this->accums[dLineIdx] += x2 - this->dLines[dLineIdx][this->idx[dLineIdx]];
    this->dLines[dLineIdx][this->idx[dLineIdx]] = x2;
    this->idx[dLineIdx]++;
    if (this->idx[dLineIdx] >= dLineLen) {
      this->idx[dLineIdx] = 0;
      updateDLine(this->accums[dLineIdx], dLineIdx + 1);
    }
  }

  virtual void update() noexcept override { }

  virtual void reset(float fs) noexcept override {
    this->msDLineLen = fs / 1000;
    std::fill(this->msDLine.begin(), this->msDLine.end(), 0);
    for (auto& dl : this->dLines) { std::fill(dl.begin(), dl.end(), 0); }
    for (auto& i : this->idx) { i = 0; }
    this->update();
  }

  Stereo<signal_t> getRms(int expo) const {
    auto n = (this->msDLineLen * gcem::pow(dLineLen, expo));
    return {
      gcem::sqrt(signal_t(2.0) * this->accums[expo].l / n),
      gcem::sqrt(signal_t(2.0) * this->accums[expo].r / n),
    };
  }
};
}