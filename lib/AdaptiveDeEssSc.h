#pragma once

/**
 * @file DynamicFilter.h
 * @author Niels Thøgersen (niels.thoegersen@gmail.com)
 * @brief Dynamic biquad filter.
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
#include "lib/Biquad.h"
#include "lib/Comp.h"
#include "lib/Component.h"
#include "lib/PeakSensor.h"
#include "lib/RmsSensor.h"
#include "lib/gcem.h"
#include "lib/utils.h"
#include <cassert>

namespace NtFx {
struct AdaptiveDeEssSc : public ComponentBase<Audio> {
  ShortRmsSensor<> rmsLo;
  ShortRmsSensor<> rmsHi;
  Biquad::LinkwitzLpfFourth xOverLpf;
  Biquad::LinkwitzHpfFourth xOverHpf;
  PeakHoldSensor<> peakLo;
  Comp::PeakSideChainLin sc;
  signal_t fc_hz { 4000 };
  signal_t offset_db { 0 };
  signal_t offset_lin { 1 };
  bool scListen { false };
  bool rmsEnable { false };

  AdaptiveDeEssSc() {
    this->sc.settings.tPeakHold_ms = 5;
    this->sc.settings.ratio        = 20;
    this->sc.settings.knee_db      = 3;
    this->sc.settings.linkEnable   = true;
    this->sc.settings.tRel_ms      = 50;
    this->peakLo.tHold_ms          = 5;
    this->peakLo.tRel_ms           = 20;
    this->rmsLo.t_ms               = 10;
    this->rmsHi.t_ms               = 10;
  }

  Audio process(Audio x) noexcept override {
    auto yLpf = this->xOverLpf.process(x);
    auto yHpf = this->xOverHpf.process(x);
    Audio ySenLo { 0 };
    Audio ySensHi { 0 };
    if (this->rmsEnable) {
      ySenLo  = this->rmsLo.process(yLpf);
      ySensHi = this->rmsHi.process(yHpf);
    } else {
      ySenLo  = this->peakLo.process(yLpf);
      ySensHi = this->sc.sensor.process(yHpf);
    }
    Audio ySc;
    auto xGc = ySensHi / (ySenLo + signal_t(1e-8)) * this->offset_lin;
    ySc.l    = this->sc._gainComputer_lin(xGc.l, this->sc.stateFilter.l);
    ySc.r    = this->sc._gainComputer_lin(xGc.r, this->sc.stateFilter.r);
    if (this->sc.settings.linkEnable) { ySc = ySc.absMin(); }
    if (this->scListen) { return yHpf; }
    return ySc;
  }

  void update() noexcept override {
    this->offset_lin     = gcem::pow(NtFx::invDb(-this->offset_db), 2);
    this->xOverHpf.fc_hz = this->fc_hz;
    this->xOverLpf.fc_hz = this->fc_hz;
    this->xOverLpf.update();
    this->xOverHpf.update();
    this->peakLo.update();
    this->sc.update();
    this->rmsLo.update();
    this->rmsHi.update();
  }

  void reset(signal_t fs) noexcept override {
    this->_fs = fs;
    this->xOverLpf.reset(fs);
    this->xOverHpf.reset(fs);
    this->peakLo.reset(fs);
    this->sc.reset(fs);
    this->rmsLo.reset(fs);
    this->rmsHi.reset(fs);
    this->update();
  }
};
}
