/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include "Knob.h"
#include "Meter.h"
#include "PluginProcessor.h"
#include "Toggle.h"

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

//==============================================================================
/**
 */
struct NtCompressorAudioProcessorEditor : public juce::AudioProcessorEditor,
                                          private juce::Timer,
                                          private juce::Slider::Listener,
                                          private juce::ToggleButton::Listener,
                                          private juce::ComboBox::Listener {

  NtCompressorAudioProcessorEditor(NtCompressorAudioProcessor&);
  ~NtCompressorAudioProcessorEditor() override;

  //==============================================================================
  void paint(juce::Graphics&) override;
  void resized() override;

  NtCompressorAudioProcessor& proc;
  NtFx::MeterGroup meters;
  NtFx::KnobLookAndFeel knobLookAndFeel;

  // TODO: allToggleLabels and on/off text for toggles.
  std::vector<std::unique_ptr<juce::Slider>> allPrimaryKnobs;
  std::vector<std::unique_ptr<juce::Label>> allPrimaryKnobLabels;
  std::vector<std::unique_ptr<juce::Slider>> allSecondaryKnobs;
  std::vector<std::unique_ptr<juce::Label>> allSecondaryKnobLabels;
  std::vector<std::unique_ptr<NtFx::Toggle>> allToggles;
  std::vector<std::unique_ptr<juce::ComboBox>> allDropDowns;
  std::vector<std::unique_ptr<juce::Label>> allDropDownLabels;
  std::vector<std::unique_ptr<juce::ComboBox>> titleBarDropDowns;
  std::vector<std::unique_ptr<juce::Label>> titleBarDropDownLabels;
  std::vector<
      std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>>
      allKnobAttachments;
  std::vector<
      std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>>
      allToggleAttachments;
  std::vector<
      std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment>>
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
  void updateUi();
  void updateTitleBar(juce::Rectangle<int>& area);
  void updateMeters(juce::Rectangle<int>& area);
  void updateBottomRow(juce::Rectangle<int>& area);
  void updateSecondaryKnobs(juce::Rectangle<int>& area);
  void updatePrimaryKnobs(juce::Rectangle<int>& area);
  void initPrimaryKnob(NtFx::KnobSpec<float>& p_spec);
  void initSecondaryKnob(NtFx::KnobSpec<float>& p_spec);
  void _initKnob(NtFx::KnobSpec<float>& p_spec,
      std::unique_ptr<juce::Slider>& p_slider,
      std::unique_ptr<juce::Label>& p_label);
  void initToggle(NtFx::ToggleSpec& spec);
  void initDropDown(NtFx::DropDownSpec& p_spec, bool addToTitleBar = false);
  void updateColours();
  void updateUiScale();
  void updateOversampling();
  void updateTheme();
  void calcSliderRowsCols(
      int nSliders, int& nRows, int& nColumns, int maxRows, int maxColumns);
  void makeGrid(juce::Rectangle<int>& area,
      float nRows,
      float nCols,
      float colWidth,
      float rowHeight,
      float pad);
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NtCompressorAudioProcessorEditor)
};
