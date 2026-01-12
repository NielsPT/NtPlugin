/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#include "PluginProcessor.h"
// #include "sound_meter/sound_meter.h"

#include "lib/NtKnob.h"
#include "lib/NtMeter.h"

//==============================================================================
/**
 */
class NtCompressorAudioProcessorEditor : public juce::AudioProcessorEditor,
                                         private juce::Timer,
                                         private juce::Slider::Listener,
                                         private juce::ToggleButton::Listener {
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

  std::vector<juce::Slider*> allSliders;
  std::vector<juce::Label*> allSliderLabels;
  std::vector<juce::Slider*> allSmallSliders;
  std::vector<juce::Label*> allSmallSliderLabels;
  // TODO: allToggleLabels and on/off text for toggles.
  std::vector<juce::TextButton*> allToggles;
  std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>>
      allSliderAttachments;
  std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>>
      allToggleAttachments;
  std::vector<juce::Rectangle<int>> borderedAreas;

  float sliderWidth     = 50;
  float entryHeight     = 20;
  float entryPad        = 10;
  float buttonHeight    = 30;
  float buttonWidth     = 100;
  float toggleHeight    = 75;
  float knobHeight      = 200;
  float smallKnobWidth  = 100;
  float smallKnobHeight = 150;

  bool popupIsDisplayed = false;
  bool isInitialized    = false;

  void displayErrorValPopup(int varId);
  void sliderValueChanged(juce::Slider* slider) override;
  void buttonClicked(juce::Button* button) override;
  void timerCallback() override;
  void drawGui();
  void initSlider(NtFx::FloatParameterSpec<float>* p_spec,
      juce::Slider* p_slider,
      juce::Label* p_label);
  void initToggle(NtFx::BoolParameterSpec* p_spec, juce::TextButton* p_button);

  void calcSliderRowsCols(int nSliders, int& nRows, int& nColumns);
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NtCompressorAudioProcessorEditor)
};
