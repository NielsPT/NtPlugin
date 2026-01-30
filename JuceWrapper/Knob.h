#pragma once

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

namespace NtFx {
// TODO: Make it a component instead like Toggle. Maybe...
struct KnobLookAndFeel : public juce::LookAndFeel_V4 {
  int fontSize  = 1;
  float uiScale = 1;
  uint32_t backgroundColour { 0xFFD3D3D3 };
  uint32_t foregroundColour { 0xFF000000 };

  juce::Font getLabelFont(juce::Label& l) override {
    return juce::Font(juce::FontOptions(this->fontSize));
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
    auto centreX = x + width / 2.0f;
    auto centreY = y + height / 2.0f;
    auto angle =
        rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
    juce::Path knobPath;
    auto outerRadius = std::min(width / 2.5f, height / 2.5f) - 4.0f;
    knobPath.addPolygon({ 0, 0 }, 8, outerRadius);
    knobPath.applyTransform(
        juce::AffineTransform::rotation(angle).translated(centreX, centreY));
    g.setColour(juce::Colour(this->foregroundColour));
    g.strokePath(knobPath, juce::PathStrokeType(2 * this->uiScale));
    g.setColour(juce::Colour(this->backgroundColour));
    g.fillPath(knobPath);
    auto pointerLength    = outerRadius * 0.5;
    auto pointerThickness = 4.0;
    juce::Path pointerPath;
    g.setColour(juce::Colour(this->foregroundColour));
    pointerPath.addRectangle(-pointerThickness * 0.5f,
        -outerRadius,
        pointerThickness,
        pointerLength);
    pointerPath.applyTransform(
        juce::AffineTransform::rotation(angle).translated(centreX, centreY));
    g.fillPath(pointerPath);
  }
};
} // namespace NtFx
