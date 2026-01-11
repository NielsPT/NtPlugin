#pragma once

#include <math.h>

#include <JuceHeader.h>
namespace NtFx {

class KnobLookAndFeel : public juce::LookAndFeel_V4 {
public:
  KnobLookAndFeel() {
    setColour(juce::Slider::ColourIds::thumbColourId, juce::Colours::white);
  }
  void drawRotarySlider(juce::Graphics& g,
      int x,
      int y,
      int width,
      int height,
      float sliderPos,
      const float rotaryStartAngle,
      const float rotaryEndAngle,
      juce::Slider&) override {
    int radius  = juce::jmin(width / 3.0, height / 3.0) - 4.0;
    int centreX = x + width / 2;
    int centreY = y + height / 2;
    auto angle  = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
    Path p;
    g.setColour(juce::Colours::lightgrey);
    p.addPolygon({ 0.0, 0.0 }, 8, radius);
    p.applyTransform(
        juce::AffineTransform::rotation(angle).translated(centreX, centreY));
    g.fillPath(p);
    auto pointerLength    = radius * 0.5;
    auto pointerThickness = 4.0;
    g.setColour(juce::Colours::black);
    p.addRectangle(-pointerThickness * 0.5f, -radius, pointerThickness, pointerLength);
    p.applyTransform(
        juce::AffineTransform::rotation(angle).translated(centreX, centreY));
    g.fillPath(p);
  }

private:
};
} // namespace NtFx
