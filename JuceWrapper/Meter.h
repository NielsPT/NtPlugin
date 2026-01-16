#pragma once

#include <JuceHeader.h>
#include <cstddef>
#include <memory>
#include <string>
#include <vector>

#include "lib/Stereo.h"
#include "lib/UiSpec.h"

namespace NtFx {
struct MonoMeter : public juce::Component {
  float minVal_db        = -45;
  float maxVal_db        = 0;
  int pad                = 10;
  int dotDiameter        = 0;
  int dotDist            = 0;
  int nDots              = 14;
  bool invert            = false;
  int nActiveDots        = 0;
  float peakLast_db      = 0;
  int decayRate_db       = 0;
  float peakVal_lin      = 0;
  float dbPrDot          = 0;
  int nHold_frames       = 0;
  int holdCounter_frames = 0;
  float holdVal_db       = 0;
  int iHoldDot           = 0;
  std::string label      = "";
  bool isInitialized     = false;
  int fontSize           = 20;

  MonoMeter() {
    this->refresh();
    this->isInitialized = true;
  };
  ~MonoMeter() = default;

  void paint(juce::Graphics& g) override {
    if (!this->isInitialized) { return; }
    if (this->getWidth() <= 0) { return; }
    int y = this->pad;
    g.setColour(juce::Colours::white);
    g.setFont(this->fontSize);
    g.drawText(this->label,
        0,
        y,
        this->getWidth(),
        this->dotDiameter,
        juce::Justification::centred);
    for (size_t i = 1; i < this->nDots + 1; i++) {
      int y;
      y = this->pad + i * this->dotDist;
      g.setColour(juce::Colours::white);
      g.drawEllipse(this->pad, y, this->dotDiameter, this->dotDiameter, 1);
      float fillPad      = this->getWidth() * 4.0 / 35.0;
      float fillDiameter = this->dotDiameter - fillPad;
      if (fillDiameter < 0) { return; }
      float fillX = this->pad + fillPad / 2;
      float fillY = y + fillPad / 2;
      if ((!this->invert && i > this->nDots - this->nActiveDots)
          || (this->invert && i < this->nDots - this->nActiveDots)) {
        g.setColour(juce::Colours::lightgrey);
        g.fillEllipse(fillX, fillY, fillDiameter, fillDiameter);
      }
      if (i == this->iHoldDot) {
        g.setColour(juce::Colours::white);
        g.fillEllipse(fillX, fillY, fillDiameter, fillDiameter);
      }
    }
  }

  void resized() override { repaint(); }

  void refresh() {
    // this->nDots = (this->getHeight() - this->dotDiameter) / this->dotDist;
    if (!(this->peakLast_db == this->peakLast_db)) {
      this->peakLast_db = this->maxVal_db;
    }
    this->dbPrDot = (this->maxVal_db - this->minVal_db) / this->nDots;
    float peak_db = 20 * std::log10(this->peakVal_lin);
    if (this->peakVal_lin <= 0) { peak_db = -100; }
    if (this->invert) {
      if (peak_db > this->peakLast_db) {
        peak_db = this->peakLast_db + this->decayRate_db;
      }
    } else {
      if (peak_db < this->peakLast_db) {
        peak_db = this->peakLast_db - this->decayRate_db;
      }
    }
    if (peak_db < this->minVal_db) { peak_db = this->minVal_db; }
    if (peak_db > this->maxVal_db) { peak_db = this->maxVal_db; }
    this->peakLast_db = peak_db;
    if (peak_db >= 0.0) {
      this->nActiveDots = this->nDots;
    } else if (peak_db >= -1.0) {
      this->nActiveDots = this->nDots - 1;
    } else if (peak_db >= -3.0) {
      this->nActiveDots = this->nDots - 2;
    } else {
      this->nActiveDots =
          (peak_db + this->maxVal_db - this->minVal_db) / this->dbPrDot - 2;
      if (this->nActiveDots < 0) { this->nActiveDots = 0; }
    }

    // Refresh hold.
    if ((!this->invert && peak_db > this->holdVal_db)
        || (this->invert && peak_db < this->holdVal_db)) {
      this->holdVal_db         = peak_db;
      this->iHoldDot           = this->nDots - this->nActiveDots;
      this->holdCounter_frames = 0;
    } else {
      this->holdCounter_frames++;
      if (this->holdCounter_frames > this->nHold_frames) {
        this->holdCounter_frames = 0;
        if (this->invert) {
          this->holdVal_db = this->maxVal_db;
        } else {
          this->holdVal_db = this->minVal_db;
        }
        this->iHoldDot = 0;
      }
    }
    this->pad         = this->getWidth() * 10 / 35;
    this->dotDiameter = this->getWidth() * 15 / 35;
    this->dotDist     = this->pad + this->dotDiameter;
    repaint();
  }

  void refresh(float level) {
    this->peakVal_lin = level;
    this->refresh();
  }

  void setDecay(float tDecay_s, float refreshRate_hz) {
    float dbPerSecond  = (this->maxVal_db - this->minVal_db) / tDecay_s;
    this->decayRate_db = dbPerSecond / refreshRate_hz;
  }

  void setPeakHold(float tHold_s, float refreshRate_hz) {
    this->nHold_frames = tHold_s * refreshRate_hz;
  }

  void setInvert(bool invert) {
    this->invert = invert;
    if (invert) {
      this->peakLast_db = this->minVal_db;
    } else {
      this->peakLast_db = this->maxVal_db;
    }
  };
};

struct MeterScale : public juce::Component {
  MonoMeter& _m;
  int fontSize { 0 };
  MeterScale(MonoMeter& m) : _m(m) { }
  void paint(juce::Graphics& g) override {
    auto offset = this->_m.pad + this->_m.dotDist;
    g.setColour(juce::Colours::white);
    g.setFont(this->fontSize);
    g.drawText("  0", 0, offset, 1000, 10, juce::Justification::left, false);
    auto y = this->_m.dotDist + offset;
    g.drawText("- 1", 0, y, 1000, 10, juce::Justification::left, false);
    for (size_t i = 2; i < this->_m.nDots; i++) {
      auto y = i * this->_m.dotDist + offset;
      auto t = "- " + std::to_string(static_cast<int>(this->_m.dbPrDot * (i - 1)));
      g.drawText(t, 0, y, 1000, 10, juce::Justification::left, false);
    }
  }
  void resized() override {
    this->_m.refresh();
    repaint();
  }
};

struct StereoMeter : public juce::Component {
  MonoMeter l;
  MonoMeter r;
  juce::Label label;
  int fontSize { 0 };
  bool hasScale { false };

  StereoMeter(std::string label) : label(label, label) {
    this->addAndMakeVisible(this->label);
    this->addAndMakeVisible(this->l);
    this->addAndMakeVisible(this->r);
    this->l.label = "L";
    this->r.label = "R";
  }
  void resized() override {
    this->drawGui();
    this->repaint();
  }
  void drawGui() {
    l.fontSize     = this->fontSize;
    r.fontSize     = this->fontSize;
    auto area      = getLocalBounds();
    auto labelArea = area.removeFromTop(this->getHeight() * 1.0 / (l.nDots + 1));
    this->label.setFont(juce::FontOptions(this->fontSize));
    this->label.setBounds(labelArea);
    this->label.setJustificationType(juce::Justification::centredBottom);
    auto lArea = area.removeFromLeft(area.getWidth() / 2.0);
    l.setBounds(lArea);
    area.setWidth(lArea.getWidth());
    r.setBounds(area);
  }
  template <typename signal_t>
  void refresh(Stereo<signal_t> val) {
    this->l.refresh(val.l);
    this->r.refresh(val.r);
  }
  void setInvert(bool val) {
    l.setInvert(val);
    r.setInvert(val);
  }
  void setDecay(float a, float b) {
    l.setDecay(a, b);
    r.setDecay(a, b);
  }
  void setPeakHold(float a, float b) {
    l.setPeakHold(a, b);
    r.setPeakHold(a, b);
  }
};

struct MeterGroup : public juce::Component {
  std::vector<std::unique_ptr<StereoMeter>> meters;
  std::vector<std::unique_ptr<MeterScale>> scales;
  MeterGroup(std::vector<MeterSpec> specs) {
    size_t i = 0;
    for (auto spec : specs) {
      auto meter = std::make_unique<StereoMeter>(spec.name);
      meter->setInvert(spec.invert);
      meter->l.minVal_db = spec.minVal_db;
      meter->r.minVal_db = spec.minVal_db;
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
  void setDecay(float a, float b) {
    for (auto& m : meters) { m->setDecay(a, b); }
  }
  void setPeakHold(float a, float b) {
    for (auto& m : meters) { m->setPeakHold(a, b); }
  }
  template <typename signal_t>
  void refresh(size_t idx, Stereo<signal_t> val) {
    this->meters[idx]->refresh(val);
  }
  void resized() override {
    this->drawGui();
    this->repaint();
  }
  int size() const noexcept { return this->meters.size(); }
  void drawGui() noexcept {
    auto area       = this->getLocalBounds();
    auto totalWidth = area.getWidth();
    auto scaleWidth = totalWidth / 8;
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
  void setHeight_dots(int nDots) {
    for (auto& m : meters) {
      m->l.nDots = nDots;
      m->r.nDots = nDots;
    }
  }
};
} // namespace NtFx
