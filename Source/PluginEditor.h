/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#include "PluginProcessor.h"
// #include "sound_meter/sound_meter.h"

#include "NtKnob.h"
#include "NtMeter.h"
typedef juce::AudioProcessorValueTreeState::SliderAttachment SA;
typedef juce::AudioProcessorValueTreeState::ButtonAttachment BA;

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
  // This reference is provided as a quick way for your editor to
  // access the processor object that created it.
  NtCompressorAudioProcessor& audioProcessor;
  std::array<NtFx::MonoMeter, 6> meters;
  NtFx::MonoMeterDbScale meterScale;
  NtFx::KnobLookAndFeel knobLookAndFeel;

  // sd::SoundMeter::MetersComponent outputMeters;
  // sd::SoundMeter::MetersComponent gainReductionMeter;

  juce::Slider threshSlider;
  juce::Slider ratioSlider;
  juce::Slider kneeSlider;
  juce::Slider tAttSlider;
  juce::Slider tRelSlider;
  juce::Slider tRmsSlider;
  juce::Slider makeupSlider;
  juce::Slider mixSlider;

  std::unique_ptr<SA> p_threshSliderAttachment;
  std::unique_ptr<SA> p_ratioSliderAttachment;
  std::unique_ptr<SA> p_kneeSliderAttachment;
  std::unique_ptr<SA> p_tAttSliderAttachment;
  std::unique_ptr<SA> p_tRelSliderAttachment;
  std::unique_ptr<SA> p_tRmsSliderAttachment;
  std::unique_ptr<SA> p_makeupSliderAttachment;
  std::unique_ptr<SA> p_mixSliderAttachment;

  juce::TextButton bypassToggle;
  juce::TextButton feedbackToggle;
  juce::TextButton linToggle;
  juce::TextButton rmsToggle;

  std::unique_ptr<BA> p_bypassToggleAttachment;
  std::unique_ptr<BA> p_feedbackToggleAttachment;
  std::unique_ptr<BA> p_linToggleAttachment;
  std::unique_ptr<BA> p_rmsToggleAttachment;

  juce::TextButton resetButton;

  juce::Label threshLabel;
  juce::Label ratioLabel;
  juce::Label kneeLabel;
  juce::Label tAttLabel;
  juce::Label tRelLabel;
  juce::Label tRmsLabel;
  juce::Label makeupLabel;
  juce::Label mixLabel;

  juce::Label inMeterLabel;
  juce::Label outMeterLabel;
  juce::Label grMeterLabel;

  // juce::Label frameCounterLabel;
  // int frameCounter = 0;

  std::vector<juce::Slider*> allSliders;
  std::vector<juce::Label*> allSliderLabels;
  std::vector<juce::Label*> allMeterLabels;
  std::vector<juce::TextButton*> allToggles;
  std::vector<std::unique_ptr<SA>*> allSliderAttachments;
  std::vector<std::unique_ptr<BA>*> allToggleAttachments;

  float sliderWidth     = 50;
  float entryHeight     = 20;
  float entryPad        = 10;
  float buttonHeight    = 30;
  float buttonWidth     = 100;
  bool popupIsDisplayed = false;

  void displayErrorValPopup(std::string message);
  void sliderValueChanged(juce::Slider* slider) override;
  // void toggleValueChanged(juce::ToggleButton* toggle) override;
  void buttonClicked(juce::Button* button) override;
  void timerCallback() override;
  void drawGui();
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NtCompressorAudioProcessorEditor)
};
