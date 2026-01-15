/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include "lib/Knob.h"
#include "lib/Meter.h"

#include "PluginProcessor.h"
#include <JuceHeader.h>

struct ButtonLookAndFeel : public juce::LookAndFeel_V4 {
  int fontSize;
  // This is the worst design choice of an API, I can think of. Man, where is
  // TextButton.setFont?
  ButtonLookAndFeel() { }
  juce::Font getTextButtonFont(juce::TextButton&, int) override {
    return juce::Font(juce::FontOptions(fontSize));
  }
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ButtonLookAndFeel)
};

//==============================================================================
/**
 */
class NtCompressorAudioProcessorEditor : public juce::AudioProcessorEditor,
                                         private juce::Timer,
                                         private juce::Slider::Listener,
                                         private juce::ToggleButton::Listener,
                                         private juce::ComboBox::Listener {
public:
  NtCompressorAudioProcessorEditor(NtCompressorAudioProcessor&);
  ~NtCompressorAudioProcessorEditor() override;

  //==============================================================================
  void paint(juce::Graphics&) override;
  void resized() override;

private:
  NtCompressorAudioProcessor& proc;
  NtFx::MeterAreaInOutGr meters;
  NtFx::KnobLookAndFeel knobLookAndFeel;
  ButtonLookAndFeel toggleLookAndFeel;

  // TODO: allToggleLabels and on/off text for toggles.
  std::vector<std::unique_ptr<juce::Slider>> allPrimaryKnobs;
  std::vector<std::unique_ptr<juce::Label>> allPrimaryKnobLabels;
  std::vector<std::unique_ptr<juce::Slider>> allSecondaryKnobs;
  std::vector<std::unique_ptr<juce::Label>> allSecondaryKnobLabels;
  std::vector<std::unique_ptr<juce::TextButton>> allToggles;
  std::vector<std::unique_ptr<juce::ComboBox>> allDropDowns;
  std::vector<std::unique_ptr<juce::Label>> allDropDownLables;
  std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>>
      allKnobAttachments;
  std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>>
      allToggleAttachments;
  std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment>>
      allDropDownAttachments;

  std::vector<juce::Rectangle<int>> borderedAreas;
  std::vector<juce::Rectangle<int>> grayAreas;

  float unscaledWindowHeight = 0;
  float uiScale              = 1;
  bool isInitialized         = false;

  void sliderValueChanged(juce::Slider* slider) override;
  void buttonClicked(juce::Button* button) override;
  void comboBoxChanged(juce::ComboBox* p_box) override;
  void timerCallback() override;
  void drawGui();
  void drawTitleBar(juce::Rectangle<int>& area);
  void drawMeters(juce::Rectangle<int>& area);
  void drawToggles(juce::Rectangle<int>& area);
  void drawSecondaryKnobs(juce::Rectangle<int>& area);
  void drawPrimaryKnobs(juce::Rectangle<int>& area);
  void initPrimaryKnob(NtFx::KnobSpec<float>& p_spec);
  void initSecondaryKnob(NtFx::KnobSpec<float>& p_spec);
  void _initKnob(NtFx::KnobSpec<float>& p_spec,
      std::unique_ptr<juce::Slider>& p_slider,
      std::unique_ptr<juce::Label>& p_label);
  void initToggle(NtFx::ToggleSpec& spec);
  void initDropDown(NtFx::DropDownSpec& p_spec);
  void updateUiScale();
  void calcSliderRowsCols(
      int nSliders, int& nRows, int& nColumns, int maxRows, int maxColumns);
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NtCompressorAudioProcessorEditor)
};
