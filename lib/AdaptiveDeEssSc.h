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
#include "lib/utils.h"

namespace NtFx {
struct AdaptiveDeEssSc : public ComponentBase<Audio> {
  Biquad::EqBand xOverLpf;
  Biquad::EqBand xOverHpf;
  Biquad::EqBand scHpf;
  PeakHoldSensorStereo<> peakLo;
  Comp::PeakSideChainLin sc;
  signal_t fc_hz { 2000 };
  signal_t offset_db { 0 };
  signal_t offset_lin { 1 };
  bool scListen { false };

  AdaptiveDeEssSc() {
    this->xOverLpf.settings.shape  = NtFx::Biquad::Shape::lpf;
    this->xOverHpf.settings.shape  = NtFx::Biquad::Shape::hpf;
    this->sc.settings.tPeakHold_ms = 2.5;
    this->sc.settings.ratio        = 20;
    this->sc.settings.knee_db      = 3;
    this->sc.settings.linkEnable   = true;
    this->sc.settings.tRel_ms      = 30;
    this->xOverLpf.settings.q      = 0.508;
    this->xOverHpf.settings.q      = 0.508;
    this->scHpf.settings.fc_hz     = 200;
    this->scHpf.settings.shape     = NtFx::Biquad::Shape::hpf;
    this->peakLo.tHold_ms          = 2.5;
    this->peakLo.tRel_ms           = 20;
  }

  Audio process(Audio x) noexcept override {
    auto yHpfMain = this->scHpf.process(x);
    auto yLpf     = this->xOverLpf.process(yHpfMain);
    auto yHpf     = this->xOverHpf.process(yHpfMain);
    auto yPeakLo  = this->peakLo.process(yLpf);
    auto yPeakHi  = this->sc.peakSensor.process(yHpf);
    Audio ySc;
    auto xGc = yPeakHi / (yPeakLo + signal_t(1e-8)) * this->offset_lin;
    ySc.l    = this->sc._gainComputer_lin(xGc.l, this->sc.stateFilter.l);
    ySc.r    = this->sc._gainComputer_lin(xGc.r, this->sc.stateFilter.r);
    if (this->sc.settings.linkEnable) { ySc = ySc.absMin(); }
    if (this->scListen) { return yHpf; }
    return ySc;
  }

  void update() noexcept override {
    this->offset_lin              = NtFx::invDb(this->offset_db);
    this->xOverHpf.settings.fc_hz = this->fc_hz;
    this->xOverLpf.settings.fc_hz = this->fc_hz;
    this->xOverLpf.update();
    this->xOverHpf.update();
    this->scHpf.update();
    this->peakLo.update();
    this->sc.update();
  }

  void reset(signal_t fs) noexcept override {
    this->_fs = fs;
    this->xOverLpf.reset(fs);
    this->xOverHpf.reset(fs);
    this->scHpf.reset(fs);
    this->peakLo.reset(fs);
    this->sc.reset(fs);
    this->update();
  }
};
}
