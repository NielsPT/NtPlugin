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
  StereoMeterSpec& meterSpec;
  GuiSpec& guiSpec;
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

  MonoMeter(StereoMeterSpec& meterSpec, GuiSpec& guiSpec)
      : meterSpec(meterSpec), guiSpec(guiSpec) {
    this->refresh();
    this->isInitialized = true;
  };
  ~MonoMeter() = default;

  void paint(juce::Graphics& g) override {
    if (!this->isInitialized) { return; }
    if (this->getWidth() <= 0) { return; }
    int y = this->pad;
    g.setColour(juce::Colour(this->guiSpec.foregroundColour));
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
      g.setColour(juce::Colour(this->guiSpec.foregroundColour));
      g.drawEllipse(this->pad, y, this->dotDiameter, this->dotDiameter, 1);
      float fillPad      = this->getWidth() * 4.0 / 35.0;
      float fillDiameter = this->dotDiameter - fillPad;
      if (fillDiameter < 0) { return; }
      float fillX = this->pad + fillPad / 2;
      float fillY = y + fillPad / 2;
      if ((!this->invert && i > this->nDots - this->nActiveDots)
          || (this->invert && i < this->nDots - this->nActiveDots)) {
        g.setColour(
            juce::Colour(this->guiSpec.foregroundColour & 0x00FFFFFF | 0xDD000000));
        g.fillEllipse(fillX, fillY, fillDiameter, fillDiameter);
      }
      if (i == this->iHoldDot) {
        g.setColour(juce::Colour(this->guiSpec.foregroundColour));
        g.fillEllipse(fillX, fillY, fillDiameter, fillDiameter);
      }
    }
  }

  void resized() override { repaint(); }

  void refresh() {
    // this->nDots = (this->getHeight() - this->dotDiameter) / this->dotDist;
    if (!(this->peakLast_db == this->peakLast_db)) {
      this->peakLast_db = this->meterSpec.maxVal_db;
    }
    this->dbPrDot =
        (this->meterSpec.maxVal_db - this->meterSpec.minVal_db) / this->nDots;
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
    if (peak_db < this->meterSpec.minVal_db) { peak_db = this->meterSpec.minVal_db; }
    if (peak_db > this->meterSpec.maxVal_db) { peak_db = this->meterSpec.maxVal_db; }
    this->peakLast_db = peak_db;
    if (peak_db >= 0.0) {
      this->nActiveDots = this->nDots;
    } else if (peak_db >= -1.0) {
      this->nActiveDots = this->nDots - 1;
    } else if (peak_db >= -3.0) {
      this->nActiveDots = this->nDots - 2;
    } else {
      this->nActiveDots =
          (peak_db + this->meterSpec.maxVal_db - this->meterSpec.minVal_db)
              / this->dbPrDot
          - 2;
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
          this->holdVal_db = this->meterSpec.maxVal_db;
        } else {
          this->holdVal_db = this->meterSpec.minVal_db;
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
    float dbPerSecond =
        (this->meterSpec.maxVal_db - this->meterSpec.minVal_db) / tDecay_s;
    this->decayRate_db = dbPerSecond / refreshRate_hz;
  }

  void setPeakHold(float tHold_s, float refreshRate_hz) {
    this->nHold_frames = tHold_s * refreshRate_hz;
  }

  void setInvert(bool invert) {
    this->invert = invert;
    if (invert) {
      this->peakLast_db = this->meterSpec.minVal_db;
    } else {
      this->peakLast_db = this->meterSpec.maxVal_db;
    }
  };
};

struct MeterScale : public juce::Component {
  MonoMeter& meter;
  int fontSize { 0 };
  MeterScale(MonoMeter& m) : meter(m) { }
  void paint(juce::Graphics& g) override {
    auto offset = this->meter.pad + this->meter.dotDist;
    g.setColour(juce::Colour(meter.guiSpec.foregroundColour));
    g.setFont(this->fontSize);
    g.drawText("  0", 0, offset, 1000, 10, juce::Justification::left, false);
    auto y = this->meter.dotDist + offset;
    g.drawText("- 1", 0, y, 1000, 10, juce::Justification::left, false);
    for (size_t i = 2; i < this->meter.nDots; i++) {
      auto y = i * this->meter.dotDist + offset;
      auto t = "- " + std::to_string(static_cast<int>(this->meter.dbPrDot * (i - 1)));
      g.drawText(t, 0, y, 1000, 10, juce::Justification::left, false);
    }
  }
  void resized() override {
    this->meter.refresh();
    repaint();
  }
};

struct StereoMeter : public juce::Component {
  MonoMeter l;
  MonoMeter r;
  GuiSpec& spec;
  juce::Label label;
  int fontSize { 0 };
  bool hasScale { false };

  StereoMeter(StereoMeterSpec& meterSpec, GuiSpec& guiSpec)
      : l(meterSpec, guiSpec), r(meterSpec, guiSpec), spec(guiSpec),
        label(meterSpec.name, meterSpec.name) {
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
    this->l.fontSize = this->fontSize;
    this->r.fontSize = this->fontSize;
    auto area        = getLocalBounds();
    auto labelArea   = area.removeFromTop(this->getHeight() * 1.0 / (l.nDots + 1));
    this->label.setFont(juce::FontOptions(this->fontSize));
    this->label.setColour(
        juce::Label::ColourIds::textColourId, juce::Colour(spec.foregroundColour));
    this->label.setBounds(labelArea);
    this->label.setJustificationType(juce::Justification::centredBottom);
    auto lArea = area.removeFromLeft(area.getWidth() / 2.0);
    this->l.setBounds(lArea);
    area.setWidth(lArea.getWidth());
    this->r.setBounds(area);
  }
  template <typename signal_t>
  void refresh(Stereo<signal_t> val) {
    this->l.refresh(val.l);
    this->r.refresh(val.r);
  }
  // TODO: Take a ref to spec and read them your self.
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
  MeterGroup(GuiSpec& guiSpec) {
    size_t i = 0;
    for (auto& spec : guiSpec.meters) {
      auto meter = std::make_unique<StereoMeter>(spec, guiSpec);
      meter->setInvert(spec.invert);
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
