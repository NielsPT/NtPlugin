#pragma once
#include "juce_core/juce_core.h"
#include "juce_graphics/juce_graphics.h"
#include "juce_gui_basics/juce_gui_basics.h"
#include <JuceHeader.h>

// struct ToggleLookAndFeel : public juce::LookAndFeel_V4 {
//   int fontSize;
//   // This is the worst design choice of an API, I can think of. Man, where is
//   // TextButton.setFont?
//   ToggleLookAndFeel() { }
//   juce::Font getTextButtonFont(juce::TextButton&, int) override {
//     return juce::Font(juce::FontOptions(fontSize));
//   }

//   JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ToggleLookAndFeel)
// };

class CustomTextButton : public juce::TextButton {
public:
  //==============================================================================
  CustomTextButton(const juce::String& buttonName = {})
      : juce::TextButton(buttonName) { }

  float fontSize { 0 };

  //==============================================================================
  /** Override to paint the button exactly as you want. */
  void paintButton(
      juce::Graphics& g, const bool isMouseOver, const bool isMouseDown) override {
    auto h            = this->getHeight();
    auto w            = this->getWidth();
    float outPadScale = 0.5;
    float dRing       = h * outPadScale;
    auto x            = (h - dRing) / 2;
    auto y            = (h - dRing) / 2;
    g.setColour(juce::Colours::white);
    juce::Rectangle<float> r1(x, y, dRing, dRing);
    g.drawEllipse(r1, 1);
    if (this->getToggleState()) {
      float inPadScale = outPadScale * 0.6;
      float dDot       = h * inPadScale;
      float xDot       = x + (dRing - dDot) / 2;
      float yDot       = y + (dRing - dDot) / 2;
      juce::Rectangle<float> r2(xDot, yDot, dDot, dDot);
      g.fillEllipse(r2);
    }
    x = (h + dRing) / 2 + h * 0.1;
    juce::Rectangle<float> r3(x, 0, 1000, h);
    juce::String t = this->getButtonText();
    g.setFont(this->fontSize);
    g.drawText(t, r3, juce::Justification::centredLeft);
  }

  //==============================================================================
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CustomTextButton)
};