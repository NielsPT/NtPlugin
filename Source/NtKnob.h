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
    // auto rx      = centreX - radius;
    // auto ry      = centreY - radius;
    // auto rw      = radius * 2.0f;
    auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

    // auto r1 = juce::Rectangle<int>(-30, -5, 60, 10);
    // auto r2 = juce::Rectangle<int>(-5, -30, 10, 60);
    // juce::Path p1;
    // p1.addRectangle(r1);
    // p1.addRectangle(r2);
    // p1.applyTransform(
    //     juce::AffineTransform::rotation(angle + M_PI / 4).translated(centreX,
    //     centreY));
    // Path p2;
    // p2.addRectangle(-30, -5, 60, 10);
    // p2.addRectangle(-5, -40, 10, 70);
    // p2.applyTransform(
    //     juce::AffineTransform::rotation(angle).translated(centreX, centreY));
    // g.fillPath(p1);
    // g.fillPath(p2);
    // g.fillEllipse(centreX - 20, centreY - 20, 40, 40);
    Path p;
    // g.setColour(juce::Colours::white);
    // p.addPolygon({ 0.0, 0.0 }, 6, radius);
    // p.applyTransform(
    //     juce::AffineTransform::rotation(angle).translated(centreX, centreY));
    // juce::PathStrokeType stroke(4.0);
    // g.strokePath(p, stroke);
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
