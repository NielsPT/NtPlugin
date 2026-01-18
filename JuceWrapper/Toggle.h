#pragma once
#include "JuceHeader.h"
#include "juce_core/juce_core.h"
#include "juce_graphics/juce_graphics.h"
#include "juce_gui_basics/juce_gui_basics.h"

namespace NtFx {
class Toggle : public juce::TextButton {
public:
  //==============================================================================
  Toggle(const juce::String& buttonName = {}) : juce::TextButton(buttonName) { }

  float fontSize { 0 };
  uint32_t colour { 0xFFFFFFFF };

  //==============================================================================
  /** Override to paint the button exactly as you want. */
  void paintButton(juce::Graphics& g, const bool isMouseOver,
      const bool isMouseDown) override {
    auto h            = this->getHeight();
    auto w            = this->getWidth();
    float outPadScale = 0.5;
    float dRing       = h * outPadScale;
    auto x            = (h - dRing) / 2;
    auto y            = (h - dRing) / 2;
    g.setColour(juce::Colour(colour));
    juce::Rectangle<float> r1(x, y, dRing, dRing);
    g.drawEllipse(r1, h / 50.0);
    float inPadScale = outPadScale * 0.6;
    float dDot       = h * inPadScale;
    float xDot       = x + (dRing - dDot) / 2;
    float yDot       = y + (dRing - dDot) / 2;
    juce::Rectangle<float> r2(xDot, yDot, dDot, dDot);
    if (this->getToggleState()) {
      g.fillEllipse(r2);
    } else {
      g.drawEllipse(r2, h / 100.0);
    }
    x = (h + dRing) / 2 + h * 0.1;
    juce::Rectangle<float> r3(x, 0, 1000, h);
    juce::String t = this->getButtonText();
    g.setFont(this->fontSize);
    g.drawText(t, r3, juce::Justification::centredLeft);
  }

  //==============================================================================
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Toggle)
};
}