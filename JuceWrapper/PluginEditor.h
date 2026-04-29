/*
 * Copyright (C) 2026 Niels Thøgersen, NTlyd
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Affero General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option) any
 * later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 * Modified from JUCE template.
 **/

#pragma once

#include "JuceWrapper/RadioButtons.h"
#include "LookAndFeel.h"
#include "Meter.h"
#include "PluginProcessor.h"
#include "Toggle.h"
#include "lib/UiSpec.h"

#include <cstddef>
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
#include <memory>
#include <string>
#include <vector>

#ifndef NTFX_PLUGIN
  #error NTFX_PLUGIN is not defined. Please add '-DNTFX_PLUGIN=[your plugin \
  name]' to cmake configure.
#endif

struct NtPluginAudioProcessorEditor : public juce::AudioProcessorEditor,
                                      private juce::Timer,
                                      private juce::Slider::Listener,
                                      private juce::ToggleButton::Listener,
                                      private juce::ComboBox::Listener,
                                      private juce::ChangeListener {

  NtPluginAudioProcessorEditor(NtPluginAudioProcessor&);
  ~NtPluginAudioProcessorEditor() override;

  //==============================================================================
  void paint(juce::Graphics&) override;
  void resized() override;

  NtPluginAudioProcessor& proc;
  NtFx::MeterGroup meters;
  NtFx::KnobLookAndFeel knobLookAndFeel;
  NtFx::TitleBarLookAndFeel dropDownLookAndFeel;

  std::vector<std::unique_ptr<juce::Slider>> primaryKnobs;
  std::vector<std::unique_ptr<juce::Label>> primaryKnobLabels;
  std::vector<std::unique_ptr<juce::Slider>> secondaryKnobs;
  std::vector<std::unique_ptr<juce::Label>> secondaryKnobLabels;
  std::vector<std::unique_ptr<NtFx::Toggle>> toggles;
  std::vector<std::unique_ptr<juce::ComboBox>> dropDowns;
  std::vector<std::unique_ptr<juce::Label>> dropDownLabels;
  std::vector<std::unique_ptr<NtFx::RadioButtonSet>> radioButtons;
  std::vector<std::unique_ptr<juce::Label>> radioButtonLabels;
  std::vector<std::unique_ptr<NtFx::ToggleSet>> toggleSets;
  std::vector<std::unique_ptr<juce::Label>> toggleSetLabels;
  std::vector<std::unique_ptr<juce::ComboBox>> titleBarDropDowns;
  std::vector<std::unique_ptr<juce::Label>> titleBarDropDownLabels;
  std::vector<
      std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>>
      knobAttachments;
  std::vector<
      std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>>
      toggleAttachments;
  std::vector<
      std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment>>
      dropDownAttachments;

  std::vector<juce::Rectangle<int>> borderedAreas;
  std::vector<juce::Rectangle<int>> grayAreas;

  float unscaledWindowHeight = 0;
  float uiScale              = 1;
  float titleBarScale        = 0.7;
  float pad                  = 10;
  bool isInitialized         = false;

  juce::Label pluginNameLabel;

  void sliderValueChanged(juce::Slider* slider) override;
  void buttonClicked(juce::Button* button) override;
  void changeListenerCallback(juce::ChangeBroadcaster* source) override;
  void comboBoxChanged(juce::ComboBox* p_box) override;
  void timerCallback() override;
  void updateUi();
  void updateTitleBar(juce::Rectangle<int>& area);
  void updateMeters(juce::Rectangle<int>& area);
  void placeSmallTogglesArea(juce::Rectangle<int>& area);
  void placeBottomRow(juce::Rectangle<int>& area);
  void updateSecondaryKnobs(juce::Rectangle<int>& area);
  void updatePrimaryKnobs(juce::Rectangle<int>& area);
  void initPrimaryKnob(NtFx::KnobSpec<float>& p_spec);
  void initSecondaryKnob(NtFx::KnobSpec<float>& p_spec);
  void _initKnob(NtFx::KnobSpec<float>& p_spec,
      std::unique_ptr<juce::Slider>& p_slider,
      std::unique_ptr<juce::Label>& p_label);
  void initToggle(NtFx::ToggleSpec& spec);
  void _initToggle(NtFx::Toggle* p_toggle, NtFx::ToggleSpec& spec);
  void initToggleGroup(NtFx::ToggleSetSpec& spec);
  void initDropDown(NtFx::DropDownSpec& p_spec, bool addToTitleBar = false);
  void initRadioButton(NtFx::RadioButtonSetSpec& spec);
  template <typename T, typename spec_t>
  std::unique_ptr<T> makeSmallToggleSet(spec_t& spec);
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
  std::unique_ptr<juce::Label> makeLabel(const std::string name);
  void placeDropdowns(juce::Rectangle<int>& area, size_t columnWidth);
  void placeToggles(juce::Rectangle<int>& area, size_t columnWidth);
  template <typename T>
  void placeSmallToggles(juce::Rectangle<int>& area,
      int size,
      std::vector<std::unique_ptr<juce::Label>>& labels,
      std::vector<std::unique_ptr<T>>& toggles);
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NtPluginAudioProcessorEditor)
};
