/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#include "PluginProcessor.h"

#include "lib/Knob.h"
#include "lib/Meter.h"

struct ButtonLookAndFeel : public LookAndFeel_V4 {
  int fontSize;
  // This is the worst design choice of an API, I can think of. Man, where is
  // TextButton.setFont?
  ButtonLookAndFeel(int fontSize) : fontSize(fontSize) { }
  Font getTextButtonFont(TextButton&, int) override { return Font(fontSize); }
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
  NtCompressorAudioProcessor& audioProcessor;
  NtFx::MeterAreaInOutGr meters;
  NtFx::KnobLookAndFeel knobLookAndFeel;

  // TODO: allToggleLabels and on/off text for toggles.
  std::vector<std::unique_ptr<juce::Slider>> allPrimaryKnobs;
  std::vector<std::unique_ptr<juce::Label>> allPrimaryKnobLabels;
  std::vector<std::unique_ptr<juce::Slider>> allSmallKnobs;
  std::vector<std::unique_ptr<juce::Label>> allSmallKnobLabels;
  std::vector<std::unique_ptr<juce::TextButton>> allToggles;
  std::vector<std::unique_ptr<juce::ComboBox>> allDropDowns;
  std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>>
      allKnobAttachments;
  std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>>
      allToggleAttachments;
  std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment>>
      allDropDownAttachments;

  std::vector<juce::Rectangle<int>> borderedAreas;
  std::vector<juce::Rectangle<int>> grayAreas;

  float unscaledWindowHeight = 0;
  float defaultWindowWidth   = 1000;
  float labelHeight          = 20;
  float toggleHeight         = 75;
  float knobHeight           = 200;
  float smallKnobWidth       = 100;
  float smallKnobHeight      = 150;
  float titleBarAreaHeight   = 25;
  float uiScale              = 1;

  int defaultFontSize = 18;

  bool popupIsDisplayed = false;
  bool isInitialized    = false;

  ButtonLookAndFeel buttonLookAndFeel;

  void displayErrorValPopup(int varId);
  void sliderValueChanged(juce::Slider* slider) override;
  void buttonClicked(juce::Button* button) override;
  void comboBoxChanged(juce::ComboBox* p_box) override;
  void timerCallback() override;
  void drawGui();
  void initKnob(NtFx::FloatParameterSpec<float>* p_spec,
      std::unique_ptr<juce::Slider>& p_slider,
      std::unique_ptr<juce::Label>& p_label);
  void initToggle(
      NtFx::BoolParameterSpec* p_spec, std::unique_ptr<juce::TextButton>& p_button);
  void initDropDown(NtFx::DropDownSpec* p_spec);
  void updateUiScale();

  void calcSliderRowsCols(int nSliders, int& nRows, int& nColumns);
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NtCompressorAudioProcessorEditor)
};
