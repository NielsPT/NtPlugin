#pragma once

#include <JuceHeader.h>
#include <array>
#include <string>

#include "Stereo.h"

namespace NtFx {

struct _MeterBase : public juce::Component { };

struct MonoMeter : public juce::Component {
  float minVal_db = -45;
  float maxVal_db = 0;
  int pad         = 10;
  // int padRight           = 10;
  int padTop             = 40;
  int padBottom          = 40;
  int dotWidth           = 15;
  int dotDist            = dotWidth + 10;
  int nDots              = 15;
  bool invert            = false;
  int nActiveDots        = 0;
  int peakLast_db        = 0;
  int decayRate_db       = 0;
  float peakVal_lin      = 0;
  float dbPrDot          = 0;
  int nHold_frames       = 0;
  int holdCounter_frames = 0;
  float holdVal_db       = 0;
  int iHoldDot           = 0;
  std::string label      = "";

  MonoMeter()  = default;
  ~MonoMeter() = default;

  void visibilityChanged() override { }

  void lookAndFeelChanged() override { }

  void paint(juce::Graphics& g) override {
    // TODO: stop if nothings changed.
    int y = this->padTop;
    g.setColour(juce::Colours::white);
    g.drawText(this->label,
        0,
        y,
        this->getWidth(),
        this->dotWidth,
        juce::Justification::centred);
    for (size_t i = 1; i < this->nDots + 1; i++) {
      int y;
      y = this->padTop + i * this->dotDist;
      g.setColour(juce::Colours::white);
      g.drawEllipse(this->pad, y, this->dotWidth, this->dotWidth, 1);
      float fillPad  = this->getWidth() * 4.0 / 35.0;
      float fillSize = this->dotWidth - fillPad; // 15 - 4
      float fillX    = this->pad + fillPad / 2;
      float fillY    = y + fillPad / 2;
      if ((!this->invert && i > this->nDots - this->nActiveDots)
          || (this->invert && i < this->nDots - this->nActiveDots)) {
        g.setColour(juce::Colours::lightgrey);
        g.fillEllipse(fillX, fillY, fillSize, fillSize);
      }
      if (i == this->iHoldDot) {
        g.setColour(juce::Colours::white);
        g.fillEllipse(fillX, fillY, fillSize, fillSize);
      }
    }
  }

  void resized() override { repaint(); }

  void refresh() {
    if (this->peakLast_db != this->peakLast_db) { this->peakLast_db = this->maxVal_db; }
    this->dbPrDot = (this->maxVal_db - this->minVal_db) / this->nDots;
    float peak_db = 20 * std::log10(this->peakVal_lin);
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
    this->nActiveDots =
        (peak_db + this->maxVal_db - this->minVal_db) / this->dbPrDot - 1;
    // if (this->invert) { this->nActiveDots -= 1; }
    if (this->nActiveDots < 0) { this->nActiveDots = 0; }
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
    this->pad       = this->getWidth() * 10 / 35;
    this->dotWidth  = this->getWidth() * 15 / 35;
    this->padTop    = this->pad;
    this->padBottom = this->pad;
    this->dotDist   = this->pad + this->dotWidth;

    repaint();
  }

  void refresh(float level) {
    this->peakVal_lin = level;
    this->refresh();
  }

  // int getWidth() { return this->pad + this->padRight + this->dotWidth; }
  // int getWidth() {retrun 35;}

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

struct MonoMeterDbScale : public juce::Component {
  MonoMeter& _m;
  MonoMeterDbScale(MonoMeter& m) : _m(m) { }
  void visibilityChanged() override { }
  void lookAndFeelChanged() override { }
  void paint(juce::Graphics& g) override {
    for (size_t i = 1; i < this->_m.nDots + 1; i++) {
      int y = this->_m.padTop + i * this->_m.dotDist;
      std::string t =
          "- " + std::to_string(static_cast<int>(this->_m.dbPrDot * (i - 1)));
      g.setColour(juce::Colours::white);
      g.drawText(t, 0, y, 40, 10, juce::Justification::centredTop);
    }
  }
  void resized() override { repaint(); }
};

struct StereoMeter : public juce::Component {
  MonoMeter l;
  MonoMeter r;
  juce::Label label;

  StereoMeter(std::string label) : label(label, label) {
    this->addAndMakeVisible(this->label);
    this->addAndMakeVisible(this->l);
    this->addAndMakeVisible(this->r);
    l.label = "L";
    r.label = "R";
  }
  // TODO
  void visibilityChanged() override { }
  void lookAndFeelChanged() override { }
  void resized() override {
    this->drawGui();
    this->repaint();
  }
  void drawGui() {
    auto area      = getLocalBounds();
    auto labelArea = area.removeFromTop(this->getHeight() * 1.0 / (l.nDots + 1));
    this->label.setBounds(labelArea);
    this->label.setJustificationType(juce::Justification::centredBottom);
    auto lArea = area.removeFromLeft(area.getWidth() / 2.0);
    l.setBounds(lArea);
    area.setWidth(lArea.getWidth());
    r.setBounds(area);
  }
  template <typename T>
  void refresh(Stereo<T> val) {
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

class MeterArea {
  // TODO
};

} // namespace ntFX
