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

#include "gcem.hpp"
#include "juce_core/system/juce_PlatformDefs.h"
#include "lib/PeakSensor.h"
#include "lib/Stereo.h"
#include "lib/UiSpec.h"
#include "lib/utils.h"

#include <cstdint>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_audio_plugin_client/juce_audio_plugin_client.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_processors_headless/juce_audio_processors_headless.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_core/juce_core.h>
#include <juce_data_structures/juce_data_structures.h>
#include <juce_events/juce_events.h>
#include <juce_graphics/juce_graphics.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_gui_extra/juce_gui_extra.h>

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

namespace NtFx {
struct MonoMeter : public juce::Component {
  MeterSpec& meterSpec;
  UiSpec& uiSpec;
  int pad             = 10;
  int dotDiameter     = 0;
  int dotDist         = 0;
  int nDots           = 14;
  int nActiveDotsPeak = 0;
  float fractPeak     = 0;
  int nActiveDotsRms  = 0;
  float peakVal_lin   = 0;
  float rmsVal_lin    = 0;
  float dbPrDot       = 0;
  float opacity       = 0.7f;

  std::string label  = "";
  bool isInitialized = false;
  int fontSize       = 20;

  // Peak hold dot
  int nHold_frames       = 0;
  int holdCounter_frames = 0;
  float holdVal_db       = 0;
  int iHoldDot           = 0;

  PeakSensor<float> peakSensor;

  MonoMeter(MeterSpec& meterSpec, UiSpec& uiSpec)
      : meterSpec(meterSpec), uiSpec(uiSpec), nDots(uiSpec.meterHeight_dots) {
    this->updateRelease(48000, 250);
    this->refresh();
    this->isInitialized = true;
  };
  ~MonoMeter() = default;

  void paint(juce::Graphics& g) override {
    if (!this->isInitialized) { return; }
    if (this->getWidth() <= 0) { return; }
    int y = this->pad;
    g.setColour(juce::Colour(this->uiSpec.foregroundColour));
    g.setFont(this->fontSize);
    g.drawText(this->label,
        0,
        y,
        this->getWidth(),
        this->fontSize,
        juce::Justification::centred);
    for (size_t i = 0; i < this->nDots; i++) {
      int y;
      y = this->pad + (i + 1) * this->dotDist;
      g.setColour(juce::Colour(this->uiSpec.foregroundColour));
      g.drawEllipse(this->pad, y, this->dotDiameter, this->dotDiameter, 1);
      float fillPad      = this->getWidth() * 4.0 / 35.0;
      float fillDiameter = this->dotDiameter - fillPad;
      if (fillDiameter < 0) { return; }
      float fillX = this->pad + fillPad / 2;
      float fillY = y + fillPad / 2;
      if (this->meterSpec.invert && i < 2) { continue; }
      if ((!this->meterSpec.invert && i >= this->nActiveDotsPeak)
          || (this->meterSpec.invert && i <= this->nActiveDotsPeak
              && this->nActiveDotsPeak != 0)) {
        uint8_t opacity = 255.0f * this->opacity;
        g.setColour(juce::Colour(
            this->uiSpec.foregroundColour & 0x00FFFFFF | (opacity << 24)));
        g.fillEllipse(fillX, fillY, fillDiameter, fillDiameter);
      }
      if (this->meterSpec.invert && i == this->nActiveDotsPeak + 1
          && this->nActiveDotsPeak != 0) {
        uint8_t opacity = 255.0f * (1 - this->fractPeak);
        g.setColour(juce::Colour(
            this->uiSpec.foregroundColour & 0x00FFFFFF | (opacity << 24)));
        g.fillEllipse(fillX, fillY, fillDiameter, fillDiameter);
      }
      if (!this->meterSpec.invert && i == this->nActiveDotsPeak - 1) {
        uint8_t opacity = 255.0f * this->fractPeak;
        g.setColour(juce::Colour(
            this->uiSpec.foregroundColour & 0x00FFFFFF | (opacity << 24)));
        g.fillEllipse(fillX, fillY, fillDiameter, fillDiameter);
      }
      if (!this->meterSpec.invert && i >= this->nActiveDotsRms) {
        uint8_t opacity = 255.0f * this->opacity;
        g.setColour(juce::Colour(
            this->uiSpec.foregroundColour & 0x00FFFFFF | (opacity << 24)));
        g.fillEllipse(fillX, fillY, fillDiameter, fillDiameter);
      }
      if (i == this->iHoldDot && this->meterSpec.hold_s
          && !(this->meterSpec.invert && i == 0)) {
        g.setColour(juce::Colour(
            this->uiSpec.foregroundColour)); // & 0x00FFFFFF | 0x8F000000));
        g.drawEllipse(fillX, fillY, fillDiameter, fillDiameter, 1);
      }
    }
  }

  void resized() override { repaint(); }

  void refresh(float xPeak, float xRms) {
    this->peakVal_lin = xPeak;
    this->rmsVal_lin  = xRms;
    this->refresh();
  }

  void refresh(bool repaint = true) {
    float ySens;
    ensureFinite(this->peakVal_lin);
    if (this->meterSpec.invert) {
      ySens = 1.0f - peakSensor.process(1.0f - this->peakVal_lin);
    } else {
      ySens = peakSensor.process(this->peakVal_lin);
    }
    float peak_db = NtFx::db(ySens);
    float rms_db  = NtFx::db(this->rmsVal_lin);

    if (peak_db < this->meterSpec.minVal_db) {
      peak_db = this->meterSpec.minVal_db;
    }
    if (peak_db > this->meterSpec.maxVal_db) {
      peak_db = this->meterSpec.maxVal_db;
    }
    if (rms_db < this->meterSpec.minVal_db) {
      rms_db = this->meterSpec.minVal_db;
    }
    if (rms_db > this->meterSpec.maxVal_db) {
      rms_db = this->meterSpec.maxVal_db;
    }

    this->nActiveDotsPeak = this->refreshNActiveDots(peak_db);
    this->nActiveDotsRms  = this->refreshNActiveDots(rms_db);
    this->refreshPeakHold(peak_db);
    this->fractPeak = gcem::abs(peak_db + this->nActiveDotsPeak * this->dbPrDot)
        / this->dbPrDot;
    jassert(this->fractPeak <= 1 && this->fractPeak >= 0);

    auto w = this->getWidth();
    if (!repaint || !w) { w = this->uiSpec.meterWidth; }
    this->pad         = w * 10.0 / this->uiSpec.meterWidth;
    this->dotDiameter = w * 15.0 / this->uiSpec.meterWidth;
    this->dotDist     = this->pad + this->dotDiameter;
    if (repaint) { this->repaint(); }
  }

  void updateRelease(float fs, float t_ms) {
    this->peakSensor.tPeak_ms = t_ms;
    this->peakSensor.reset(fs);
    this->nHold_frames =
        this->meterSpec.hold_s * this->uiSpec.meterRefreshRate_hz;
    this->dbPrDot =
        (this->meterSpec.maxVal_db - this->meterSpec.minVal_db) / this->nDots;
  }

  int refreshNActiveDots(float peak_db) {
    int nActiveDots;
    if (peak_db >= 0.01) {
      nActiveDots = this->nDots;
      // } else if (peak_db >= -1.0) {
      //   nActiveDots = this->nDots - 1;
      // } else if (peak_db >= -3.0) {
      //   nActiveDots = this->nDots - 2;
    } else {
      nActiveDots =
          (peak_db + this->meterSpec.maxVal_db - this->meterSpec.minVal_db)
          / this->dbPrDot;
    }
    return this->nDots - nActiveDots;
  }

  void refreshPeakHold(float peak_db) {
    if ((!this->meterSpec.invert && peak_db > this->holdVal_db)
        || (this->meterSpec.invert && peak_db < this->holdVal_db)) {
      this->holdVal_db         = peak_db;
      this->iHoldDot           = this->nActiveDotsPeak;
      this->holdCounter_frames = 0;
      return;
    }
    this->holdCounter_frames++;
    if (this->holdCounter_frames > this->nHold_frames) {
      this->holdCounter_frames = 0;
      if (this->meterSpec.invert) {
        this->holdVal_db = this->meterSpec.maxVal_db;
      } else {
        this->holdVal_db = this->meterSpec.minVal_db;
      }
      this->iHoldDot = this->nActiveDotsPeak;
    }
  }
};

struct MeterScale : public juce::Component {
  MonoMeter& meter;
  int fontSize { 0 };
  MeterScale(MonoMeter& m) : meter(m) { }
  void paint(juce::Graphics& g) override {
    auto offset = this->meter.pad + this->meter.dotDist;
    g.setColour(juce::Colour(meter.uiSpec.foregroundColour));
    g.setFont(this->fontSize);
    g.drawText("OVER", 0, offset, 1000, 10, juce::Justification::left, false);
    // auto y = this->meter.dotDist + offset;
    // g.drawText("- 1", 0, y, 1000, 10, juce::Justification::left, false);
    for (size_t i = 1; i < this->meter.nDots; i++) {
      auto y = i * this->meter.dotDist + offset;
      auto t = "- "
          + std::to_string(static_cast<int>(this->meter.dbPrDot * (i - 1)));
      g.drawText(t, 0, y, 1000, 10, juce::Justification::left, false);
    }
  }
  void resized() override {
    this->meter.refresh();
    this->repaint();
  }
};

struct StereoMeter : public juce::Component {
  MonoMeter l;
  MonoMeter r;
  UiSpec& spec;
  juce::Label label;
  int fontSize { 0 };
  float uiScale { 1 };
  bool hasScale { false };

  StereoMeter(MeterSpec& meterSpec, UiSpec& uiSpec)
      : l(meterSpec, uiSpec), r(meterSpec, uiSpec), spec(uiSpec),
        label(meterSpec.name, meterSpec.name) {
    this->addAndMakeVisible(this->label);
    this->addAndMakeVisible(this->l);
    this->addAndMakeVisible(this->r);
    this->l.label = "L";
    this->r.label = "R";
  }
  void resized() override {
    this->updateUi();
    this->repaint();
  }
  void updateUi() {
    this->l.fontSize = this->fontSize;
    this->r.fontSize = this->fontSize;
    auto area        = getLocalBounds();
    auto labelArea =
        area.removeFromTop(this->l.uiSpec.labelHeight * this->uiScale);
    this->label.setFont(juce::FontOptions(this->fontSize));
    this->label.setBounds(labelArea);
    this->label.setJustificationType(juce::Justification::centredBottom);
    auto lArea = area.removeFromLeft(area.getWidth() / 2.0);
    this->l.setBounds(lArea);
    area.setWidth(lArea.getWidth());
    this->r.setBounds(area);
  }
  template <typename signal_t>
  void refresh(Stereo<signal_t> xPeak, Stereo<signal_t> xRms) {
    this->l.refresh(xPeak.l, xRms.l);
    this->r.refresh(xPeak.r, xRms.r);
  }
};

struct MeterGroup : public juce::Component {
  std::vector<std::unique_ptr<StereoMeter>> meters;
  std::vector<std::unique_ptr<MeterScale>> scales;
  MeterGroup(UiSpec& uiSpec, std::vector<MeterSpec>& meterSpecs) {
    size_t i = 0;
    for (auto& spec : meterSpecs) {
      auto meter = std::make_unique<StereoMeter>(spec, uiSpec);
      this->addAndMakeVisible(meter.get());
      if (spec.hasScale) {
        meter->hasScale = true;
        auto scale      = std::make_unique<MeterScale>(meter->l);
        this->addAndMakeVisible(scale.get());
        scales.push_back(std::move(scale));
      }
      meters.push_back(std::move(meter));
    }
  }
  template <typename signal_t>
  void refresh(size_t idx, Stereo<signal_t> xPeak, Stereo<signal_t> xRms) {
    this->meters[idx]->refresh(xPeak, xRms);
  }
  void resized() override {
    this->updateUi();
    this->repaint();
  }
  int size() const noexcept { return this->meters.size(); }
  void updateUi() noexcept {
    auto area       = this->getLocalBounds();
    auto totalWidth = area.getWidth();
    auto scaleWidth =
        totalWidth / (this->meters.size() * 2 + this->scales.size());
    auto meterWidth = scaleWidth * 2;
    size_t iScale   = 0;
    for (auto& m : this->meters) {
      m->setBounds(area.removeFromLeft(meterWidth));
      if (m->hasScale) {
        auto scaleArea = area.removeFromLeft(scaleWidth);
        scaleArea.removeFromTop(m->label.getHeight());
        this->scales[iScale++]->setBounds(scaleArea);
      }
    }
  }
  void setFontSize(int size) {
    for (auto& m : meters) { m->fontSize = size; }
    for (auto& s : scales) { s->fontSize = size; }
  }
  void setUiScale(float uiScale) {
    for (auto& m : meters) { m->uiScale = uiScale; }
  }
  float getMinimalWidth() const noexcept {
    if (!this->meters.size()) { return 0; }
    return this->meters[0]->l.uiSpec.meterWidth
        * (this->meters.size() * 2 + this->scales.size());
  }
  float getMinimalHeight() const noexcept {
    if (!this->meters.size()) { return 0; }
    auto& m = this->meters[0]->l;
    m.refresh(false);
    return (m.uiSpec.labelHeight) * 2 + (m.nDots + 2) * m.dotDist + m.pad;
  }
  void updateRelease(float fs) {
    for (auto& m : this->meters) {
      m->l.updateRelease(fs, m->l.meterSpec.decay_s * 1000);
      m->r.updateRelease(fs, m->r.meterSpec.decay_s * 1000);
    }
  }
};
} // namespace NtFx
