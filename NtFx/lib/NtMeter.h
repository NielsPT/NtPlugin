#pragma once

#include <JuceHeader.h>
#include <array>
#include <string>

#include "Stereo.h"

namespace NtFx {
struct MonoMeter : public juce::Component {
  float minVal_db        = -45;
  float maxVal_db        = 0;
  int pad                = 10;
  int dotWidth           = 0;
  int dotDist            = 0;
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
    // TODO: stop if nothings changed.
    int y = this->pad;
    g.setColour(juce::Colours::white);
    g.setFont(this->fontSize);
    g.drawText(this->label,
        0,
        y,
        this->getWidth(),
        this->dotWidth,
        juce::Justification::centred);
    for (size_t i = 1; i < this->nDots + 1; i++) {
      int y;
      y = this->pad + i * this->dotDist;
      g.setColour(juce::Colours::white);
      g.drawEllipse(this->pad, y, this->dotWidth, this->dotWidth, 1);
      float fillPad  = this->getWidth() * 4.0 / 35.0;
      float fillSize = this->dotWidth - fillPad;
      if (fillSize < 0) { return; }
      float fillX = this->pad + fillPad / 2;
      float fillY = y + fillPad / 2;
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
    this->nActiveDots =
        (peak_db + this->maxVal_db - this->minVal_db) / this->dbPrDot - 1;
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
    this->pad      = this->getWidth() * 10 / 35;
    this->dotWidth = this->getWidth() * 15 / 35;
    this->dotDist  = this->pad + this->dotWidth;

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

struct MonoMeterDbScale : public juce::Component {
  MonoMeter& _m;
  MonoMeterDbScale(MonoMeter& m) : _m(m) { }
  void paint(juce::Graphics& g) override {
    auto offset = this->_m.pad + this->_m.dotDist;
    for (size_t i = 0; i < this->_m.nDots; i++) {
      auto y        = i * this->_m.dotDist + offset;
      std::string t = "- " + std::to_string(static_cast<int>(this->_m.dbPrDot * i));
      g.setColour(juce::Colours::white);
      g.setFont(_m.fontSize);
      g.drawText(t, 0, y, 40, 10, juce::Justification::centred);
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
  int fontSize = 20;

  StereoMeter(std::string label) : label(label, label) {
    this->addAndMakeVisible(this->label);
    this->addAndMakeVisible(this->l);
    this->addAndMakeVisible(this->r);
    l.label = "L";
    r.label = "R";
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
    this->label.setFont(this->fontSize);
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

struct MeterAreaInOutGr : public juce::Component {
  StereoMeter in;
  StereoMeter out;
  StereoMeter gr;
  MonoMeterDbScale scale;
  MeterAreaInOutGr() : in("IN"), out("OUT"), gr("GR"), scale(in.l) {
    this->gr.setInvert(true);
    this->addAndMakeVisible(in);
    this->addAndMakeVisible(out);
    this->addAndMakeVisible(gr);
    this->addAndMakeVisible(scale);
  }
  void setDecay(float a, float b) {
    this->in.setDecay(a, b);
    this->out.setDecay(a, b);
    this->gr.setDecay(a, b);
  }
  void setPeakHold(float a, float b) {
    this->in.setPeakHold(a, b);
    this->out.setPeakHold(a, b);
    this->gr.setPeakHold(a, b);
  }
  template <typename signal_t>
  void refreshIn(Stereo<signal_t> val) {
    this->in.refresh(val);
  }
  template <typename signal_t>
  void refreshOut(Stereo<signal_t> val) {
    this->out.refresh(val);
  }
  template <typename signal_t>
  void refreshGr(Stereo<signal_t> val) {
    this->gr.refresh(val);
  }
  template <typename signal_t>
  void refresh(size_t idx, Stereo<signal_t> val) {
    switch (idx) {
    case 0:
      this->in.refresh(val);
      break;
    case 1:
      this->out.refresh(val);
      break;
    case 2:
      this->gr.refresh(val);
      break;
    default:
      break;
    }
  }
  void resized() override {
    this->drawGui();
    this->repaint();
  }
  int size() noexcept { return 3; }
  void drawGui() noexcept {
    auto area       = this->getLocalBounds();
    auto totalWidth = area.getWidth();
    auto scaleWidth = totalWidth / 8;
    auto meterWidth = scaleWidth * 2;
    this->in.setBounds(area.removeFromLeft(meterWidth));
    this->out.setBounds(area.removeFromLeft(meterWidth));
    this->gr.setBounds(area.removeFromLeft(meterWidth));
    area.removeFromTop(this->in.label.getHeight());
    this->scale.setBounds(area);
  }
  void setFontSize(int size) {
    this->in.fontSize  = size;
    this->out.fontSize = size;
    this->gr.fontSize  = size;
  }
};
} // namespace ntFX
