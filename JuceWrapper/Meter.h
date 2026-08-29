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

#include "lib/Audio.h"
#include "lib/PeakSensor.h"
#include "lib/UiSpec.h"
#include "lib/gcem.h"
#include "lib/utils.h"

#include <juce_core/juce_core.h>
#include <juce_core/system/juce_PlatformDefs.h>
#include <juce_data_structures/juce_data_structures.h>
#include <juce_events/juce_events.h>
#include <juce_graphics/juce_graphics.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_gui_extra/juce_gui_extra.h>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace NtFx {
struct MeterBase : public juce::Component {
  PeakHoldSensor<192 * 8 * 100> peakSensor;
  MeterSpec& meterSpec;
  UiSpec& uiSpec;
  std::string label { "" };
  float pad { 10 };
  float dotDiameter { 0 };
  float dotDist { 0 };
  int nDots { 14 };
  int nActiveDotsPeak { 0 };
  float fractPeak { 0 };
  int nActiveDotsRms { 0 };
  float peakVal_lin { 0 };
  float rmsVal_lin { 0 };
  float dbPrDot { 0 };
  float opacity { 0.7f };
  float fontSize { 20 };
  int nHold_frames { 0 };
  int holdCounter_frames { 0 };
  float holdVal_db { 0 };
  int iHoldDot { 0 };
  bool hasScale { false };

  MeterBase(MeterSpec& _meterSpec, UiSpec& _uiSpec)
      : meterSpec(_meterSpec), uiSpec(_uiSpec),
        nDots(_uiSpec.meterHeight_dots) {
    this->updateRelease(48000);
    this->refresh();
  }
  ~MeterBase() override                  = default;
  MeterBase(const MeterBase&)            = delete;
  MeterBase& operator=(const MeterBase&) = delete;
  MeterBase(MeterBase&&)                 = delete;
  MeterBase& operator=(MeterBase&&)      = delete;

  virtual void fillDots(
      juce::Graphics& g, int i, float fillX, float fillY, float fillDiameter) {
    if (i >= this->nActiveDotsPeak) {
      auto _opacity = uint8_t(255.0f * this->opacity);
      g.setColour(juce::Colour((this->uiSpec.foregroundColour & 0x00FFFFFF)
          | uint32_t(_opacity << 24u)));
      g.fillEllipse(fillX, fillY, fillDiameter, fillDiameter);
    }
  }

  virtual void fillFract(
      juce::Graphics& g, int i, float fillX, float fillY, float fillDiameter) {
    if (i == this->nActiveDotsPeak - 1) {
      auto _opacity = uint8_t(255.0f * this->fractPeak);
      g.setColour(juce::Colour((this->uiSpec.foregroundColour & 0x00FFFFFF)
          | uint32_t(_opacity << 24)));
      g.fillEllipse(fillX, fillY, fillDiameter, fillDiameter);
    }
  }

  virtual void fillRms(
      juce::Graphics& g, int i, float fillX, float fillY, float fillDiameter) {
    if (i >= this->nActiveDotsRms) {
      // auto _opacity = uint8_t(255.0f * this->opacity);
      // g.setColour(juce::Colour((this->uiSpec.foregroundColour & 0x00FFFFFF)
      //     | uint32_t(_opacity << 24)));
      g.setColour(juce::Colour(0x000000FF));
      g.fillEllipse(fillX, fillY, fillDiameter, fillDiameter);
    }
  }

  virtual void drawPeakRing(
      juce::Graphics& g, int i, float fillX, float fillY, float fillDiameter) {
    if (i == this->iHoldDot && (this->meterSpec.hold_s != 0.0f)) {
      g.setColour(juce::Colour(
          this->uiSpec.foregroundColour)); // & 0x00FFFFFF | 0x8F000000));
      g.drawEllipse(fillX, fillY, fillDiameter, fillDiameter, 1);
    }
  }

  virtual float sensor() { return peakSensor.process(this->peakVal_lin); }

  void paint(juce::Graphics& g) override {
    if (this->getWidth() <= 0) { return; }
    g.setColour(juce::Colour(this->uiSpec.foregroundColour));
    g.setFont(this->fontSize);
    g.drawText(this->label,
        0,
        int(this->pad),
        this->getWidth(),
        int(this->fontSize),
        juce::Justification::centred);
    for (int i = 0; i < this->nDots; i++) {
      auto y = float(this->pad + (float(i) + 1.0f) * this->dotDist);
      g.setColour(juce::Colour(this->uiSpec.foregroundColour));
      g.drawEllipse(this->pad, y, this->dotDiameter, this->dotDiameter, 1);
      auto fillPad      = float(this->getWidth()) * 4.0f / 35.0f;
      auto fillDiameter = this->dotDiameter - fillPad;
      if (fillDiameter < 0) { return; }
      auto fillX = this->pad + fillPad / 2;
      auto fillY = y + fillPad / 2;
      this->fillDots(g, i, fillX, fillY, fillDiameter);
      this->fillFract(g, i, fillX, fillY, fillDiameter);
      this->fillRms(g, i, fillX, fillY, fillDiameter);
      this->drawPeakRing(g, i, fillX, fillY, fillDiameter);
    }
  }

  void resized() override { this->repaint(); }

  void refresh(float xPeak, float xRms) {
    this->peakVal_lin = xPeak;
    this->rmsVal_lin  = xRms;
    this->refresh();
  }

  void refresh(bool repaint = true) {
    ensureFinite(this->peakVal_lin);
    float ySens   = this->sensor();
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
    this->nActiveDotsPeak = this->calcActiveDots(peak_db);
    this->nActiveDotsRms  = this->calcActiveDots(rms_db);
    this->refreshPeakHold(peak_db);
    this->fractPeak =
        gcem::abs(peak_db + float(this->nActiveDotsPeak) * this->dbPrDot)
        / this->dbPrDot;
    jassert(this->fractPeak <= 1 && this->fractPeak >= 0);

    auto w = float(this->getWidth());
    if (!repaint || w == 0.0f) { w = this->uiSpec.meterWidth; }
    this->pad         = w * 10.0f / this->uiSpec.meterWidth;
    this->dotDiameter = w * 15.0f / this->uiSpec.meterWidth;
    this->dotDist     = this->pad + this->dotDiameter;
    if (repaint) { this->repaint(); }
  }

  void updateRelease(signal_t fs) {
    this->peakSensor.tRel_ms  = this->meterSpec.decay_s * 1000;
    this->peakSensor.tHold_ms = 100;
    this->peakSensor.reset(fs);
    this->nHold_frames =
        int(this->meterSpec.hold_s * this->uiSpec.meterRefreshRate_hz);
    this->dbPrDot = (this->meterSpec.maxVal_db - this->meterSpec.minVal_db)
        / float(this->nDots);
  }

  virtual int calcActiveDots(float peak_db) {
    int nActiveDots =
        int((peak_db + this->meterSpec.maxVal_db - this->meterSpec.minVal_db)
            / this->dbPrDot);
    return this->nDots - nActiveDots;
  }

  virtual void refreshPeakHold(float peak_db) {
    if (peak_db > this->holdVal_db) {
      this->holdVal_db         = peak_db;
      this->iHoldDot           = this->nActiveDotsPeak - 1;
      this->holdCounter_frames = 0;
      return;
    }
    this->holdCounter_frames++;
    if (this->holdCounter_frames > this->nHold_frames) {
      this->holdCounter_frames = 0;
      this->holdVal_db         = this->meterSpec.minVal_db;
      this->iHoldDot           = this->nActiveDotsPeak - 1;
    }
  }
};

struct Meter : public MeterBase {
  Meter(MeterSpec& _meterSpec, UiSpec& _uiSpec)
      : MeterBase(_meterSpec, _uiSpec) { }
};

struct GrMeter : public MeterBase {
  GrMeter(MeterSpec& _meterSpec, UiSpec& _uiSpec)
      : MeterBase(_meterSpec, _uiSpec) { }
  void fillDots(juce::Graphics& g,
      int i,
      float fillX,
      float fillY,
      float fillDiameter) override {
    if (i < this->nActiveDotsPeak) {
      auto _opacity = uint8_t(255.0f * this->opacity);
      g.setColour(juce::Colour((this->uiSpec.foregroundColour & 0x00FFFFFF)
          | uint32_t(_opacity << 24u)));
      g.fillEllipse(fillX, fillY, fillDiameter, fillDiameter);
    }
  }

  void fillFract(juce::Graphics& g,
      int i,
      float fillX,
      float fillY,
      float fillDiameter) override {
    if (i == this->nActiveDotsPeak) {
      auto _opacity = uint8_t(255.0f * this->fractPeak);
      g.setColour(juce::Colour((this->uiSpec.foregroundColour & 0x00FFFFFF)
          | uint32_t(_opacity << 24)));
      g.fillEllipse(fillX, fillY, fillDiameter, fillDiameter);
    }
  }

  void fillRms(juce::Graphics&, int, float, float, float) override { }

  float sensor() override {
    return 1.0f - peakSensor.process(1.0f - this->peakVal_lin);
  }

  void drawPeakRing(juce::Graphics& g,
      int i,
      float fillX,
      float fillY,
      float fillDiameter) override {
    if (i == this->iHoldDot && (this->meterSpec.hold_s != 0.0f)) {
      g.setColour(juce::Colour(
          this->uiSpec.foregroundColour)); // & 0x00FFFFFF | 0x8F000000));
      g.drawEllipse(fillX, fillY, fillDiameter, fillDiameter, 1);
    }
  }

  int calcActiveDots(float peak_db) override {
    int nActiveDots =
        int((peak_db + this->meterSpec.maxVal_db - this->meterSpec.minVal_db)
            / this->dbPrDot);
    return this->nDots - nActiveDots - 1;
  }

  void refreshPeakHold(float peak_db) override {
    if (peak_db < this->holdVal_db) {
      this->holdVal_db         = peak_db;
      this->iHoldDot           = this->nActiveDotsPeak - 1;
      this->holdCounter_frames = 0;
      return;
    }
    this->holdCounter_frames++;
    if (this->holdCounter_frames > this->nHold_frames) {
      this->holdCounter_frames = 0;
      this->holdVal_db         = this->meterSpec.maxVal_db;
      this->iHoldDot           = this->nActiveDotsPeak - 1;
    }
  }
};

struct MeterScale : public juce::Component {
  MeterBase& meter;
  MeterScale(MeterBase& m) : meter(m) { }
  void paint(juce::Graphics& g) override {
    auto offset = this->meter.pad + this->meter.dotDist;
    g.setColour(juce::Colour(meter.uiSpec.foregroundColour));
    g.setFont(this->meter.fontSize);
    for (size_t i = 0; i < size_t(this->meter.nDots); i++) {
      auto y = float(i) * this->meter.dotDist + offset;
      auto t = "- "
          + std::to_string(static_cast<int>(this->meter.dbPrDot * float(i)));
      g.drawText(t, 0, int(y), 1000, 10, juce::Justification::left, false);
    }
  }
  void resized() override {
    this->meter.refresh();
    this->repaint();
  }
};

struct StereoMeterBase : public juce::Component {
  float fontSize { 0 };
  float uiScale { 1 };
  bool hasScale { false };
  bool onlyShowLeft { false };

  StereoMeterBase()                                  = default;
  ~StereoMeterBase() override                        = default;
  StereoMeterBase(const StereoMeterBase&)            = delete;
  StereoMeterBase& operator=(const StereoMeterBase&) = delete;
  StereoMeterBase(StereoMeterBase&&)                 = delete;
  StereoMeterBase& operator=(StereoMeterBase&&)      = delete;
  virtual MeterBase& leftMeter() noexcept            = 0;
  virtual MeterBase& rightMeter() noexcept           = 0;
  virtual void refresh(Audio xPeak, Audio xRms)      = 0;
  virtual void updateRelease(signal_t fs)            = 0;
  virtual int labelHeight() const noexcept           = 0;
};

template <typename T>
struct StereoMeter : public StereoMeterBase {
  T l;
  T r;
  UiSpec& spec;
  juce::Label label;

  StereoMeter(MeterSpec& meterSpec, UiSpec& uiSpec)
      : StereoMeterBase(), l(meterSpec, uiSpec), r(meterSpec, uiSpec),
        spec(uiSpec), label(meterSpec.name, meterSpec.name) {
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
    if (this->onlyShowLeft) {
      this->l.setBounds(area);
      this->l.label = this->l.meterSpec.name;
      return;
    }
    auto labelArea =
        area.removeFromTop(int(this->l.uiSpec.labelHeight * this->uiScale));
    this->label.setFont(juce::FontOptions(this->fontSize));
    this->label.setBounds(labelArea);
    this->label.setJustificationType(juce::Justification::centredBottom);
    auto lArea = area.removeFromLeft(area.getWidth() / 2);
    this->l.setBounds(lArea);
    this->r.setBounds(area);
  }
  void refresh(Audio xPeak, Audio xRms) override {
    this->l.refresh(xPeak.l, xRms.l);
    this->r.refresh(xPeak.r, xRms.r);
  }
  MeterBase& leftMeter() noexcept override { return this->l; }
  MeterBase& rightMeter() noexcept override { return this->r; }
  void updateRelease(signal_t fs) override {
    this->l.updateRelease(fs);
    this->r.updateRelease(fs);
  }
  int labelHeight() const noexcept override { return this->label.getHeight(); }
};

struct MeterGroup : public juce::Component {
  std::vector<std::unique_ptr<StereoMeterBase>> meters;
  std::vector<std::unique_ptr<MeterScale>> scales;
  int nChs { 2 };
  MeterGroup(UiSpec& uiSpec, std::vector<MeterSpec>& meterSpecs) {
    for (auto& spec : meterSpecs) {
      std::unique_ptr<StereoMeterBase> meter;
      if (spec.invert) {
        meter = std::make_unique<StereoMeter<GrMeter>>(spec, uiSpec);
      } else {
        meter = std::make_unique<StereoMeter<Meter>>(spec, uiSpec);
      }
      this->addAndMakeVisible(meter.get());
      if (spec.hasScale) {
        meter->hasScale = true;
        auto scale      = std::make_unique<MeterScale>(meter->leftMeter());
        this->addAndMakeVisible(scale.get());
        scales.push_back(std::move(scale));
      }
      meters.push_back(std::move(meter));
    }
  }
  void resized() override {
    this->updateUi();
    this->repaint();
  }
  virtual void setFontSize(float size) {
    for (auto& m : this->meters) { m->fontSize = size; }
  }
  void refresh(size_t idx, Audio xPeak, Audio xRms) {
    this->meters[idx]->refresh(xPeak, xRms);
  }
  void updateUi() noexcept {
    auto area       = this->getLocalBounds();
    auto totalWidth = area.getWidth();
    auto scaleWidth = totalWidth
        / (int(this->meters.size()) * this->nChs + int(this->scales.size()));
    auto meterWidth = scaleWidth * this->nChs;
    size_t iScale   = 0;
    for (auto& m : this->meters) {
      m->setBounds(area.removeFromLeft(meterWidth));
      if (m->hasScale) {
        auto scaleArea = area.removeFromLeft(scaleWidth);
        scaleArea.removeFromTop(m->labelHeight());
        this->scales[iScale++]->setBounds(scaleArea);
      }
    }
  }
  void setUiScale(float uiScale) {
    for (auto& m : this->meters) { m->uiScale = uiScale; }
  }
  void setOnlyShowLeft(bool onlyShowLeft) {
    for (auto& m : this->meters) { m->onlyShowLeft = onlyShowLeft; }
    if (onlyShowLeft) { this->nChs = 1; }
  }
  void updateRelease(signal_t fs) {
    for (auto& m : this->meters) { m->updateRelease(fs); }
  }
  float getMinimalWidth() const noexcept {
    if (!this->meters.size()) { return 0; }
    return this->meters[0]->leftMeter().uiSpec.meterWidth
        * (float(this->meters.size()) * float(this->nChs)
            + float(this->scales.size()));
  }
  float getMinimalHeight() const noexcept {
    if (!this->meters.size()) { return 0; }
    auto& m = this->meters[0]->leftMeter();
    m.refresh(false);
    return (m.uiSpec.labelHeight) * float(this->nChs)
        + float(m.nDots + 2) * float(m.dotDist) + float(m.pad);
    // TODO: Or make these float as well like uiSpec.
  }
};
} // namespace NtFx
