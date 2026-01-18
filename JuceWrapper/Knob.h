#pragma once

#include <JuceHeader.h>

namespace NtFx {
// TODO: Make it a component instead like Toggle. Maybe...
struct KnobLookAndFeel : public juce::LookAndFeel_V4 {
  int fontSize = 1;
  uint32_t backgroundColour { 0xFFD3D3D3 };
  uint32_t foregroundColour { 0xFF000000 };

  juce::Font getLabelFont(juce::Label& l) override {
    return juce::Font(juce::FontOptions(this->fontSize));
  }
  void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
      float sliderPos, const float rotaryStartAngle, const float rotaryEndAngle,
      juce::Slider&) override {
    auto radius  = std::min(width / 2.5f, height / 2.5f) - 4.0f;
    auto centreX = x + width / 2.0f;
    auto centreY = y + height / 2.0f;
    auto angle   = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
    juce::Path knobPath;
    g.setColour(juce::Colour(this->backgroundColour));
    knobPath.addPolygon({ 0, 0 }, 8, radius);
    knobPath.applyTransform(
        juce::AffineTransform::rotation(angle).translated(centreX, centreY));
    g.fillPath(knobPath);
    auto pointerLength    = radius * 0.5;
    auto pointerThickness = 4.0;
    juce::Path pointerPath;
    g.setColour(juce::Colour(this->foregroundColour));
    pointerPath.addRectangle(
        -pointerThickness * 0.5f, -radius, pointerThickness, pointerLength);
    pointerPath.applyTransform(
        juce::AffineTransform::rotation(angle).translated(centreX, centreY));
    g.fillPath(pointerPath);
  }
};
} // namespace NtFx
