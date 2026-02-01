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
// TODO: A prettier knob.
// TODO: Make it a component instead like Toggle. Maybe...
// TODO: OR build an NrPluginLookAndFeel that holds all of it.
struct KnobLookAndFeel : public juce::LookAndFeel_V4 {
  float fontSize = 1;
  float uiScale  = 1;
  uint32_t backgroundColour { 0xFFD3D3D3 };
  uint32_t foregroundColour { 0xFF000000 };

  juce::Font getLabelFont(juce::Label& l) override {
    return juce::Font(juce::FontOptions(this->fontSize));
  }
  void drawComboBox(juce::Graphics& g,
      int width,
      int height,
      bool,
      int,
      int,
      int,
      int,
      juce::ComboBox& box) override {
    // TODO: This AL slop does not work. The text does scale, but it's not
    // setFont that does it... ?
    // g.setFont(juce::Font(juce::FontOptions(fontSize * 0.6)));
    LookAndFeel_V4::drawComboBox(g, width, height, true, 0, 0, 0, 0, box);
    // TODO: This AL slop does not work.
    // const float arrowX    = width - 25.0f * uiScale;
    // const float arrowY    = height * 0.5f;
    // const float arrowSize = 10.0f * uiScale;

    // juce::Path arrowPath;
    // arrowPath.addTriangle(arrowX,
    //     arrowY - arrowSize,
    //     arrowX + arrowSize,
    //     arrowY - arrowSize,
    //     arrowX + 0.5f * arrowSize,
    //     arrowY + arrowSize);

    // g.setColour(box.findColour(juce::ComboBox::buttonColourId));
    // g.fillPath(arrowPath);
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
