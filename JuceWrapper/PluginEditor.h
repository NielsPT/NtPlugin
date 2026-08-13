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

#include "PluginProcessor.h"

#include "LookAndFeel.h"
#include "Meter.h"
#include "RadioButtons.h"
#include "Toggle.h"
#include "lib/UiSpec.h"

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

#include <cstddef>
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
  NtPluginAudioProcessor& proc;
  NtFx::MeterGroup meters;
  NtFx::KnobLookAndFeel knobLookAndFeel;
  NtFx::TitleBarLookAndFeel dropDownLookAndFeel;
  std::vector<std::unique_ptr<juce::Slider>> primaryKnobs;
  std::vector<std::unique_ptr<juce::Label>> primaryKnobLabels;
  std::vector<std::unique_ptr<juce::Slider>> secondaryKnobs;
  std::vector<std::unique_ptr<juce::Label>> secondaryKnobLabels;
  std::vector<std::unique_ptr<juce::Label>> knobGroupLabels;
  std::vector<std::vector<std::unique_ptr<juce::Slider>>> knobGroups;
  std::vector<std::vector<std::unique_ptr<juce::Label>>> groupKnobLabels;
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

  int unscaledWindowHeight { 0 };
  int unscaledWindowWidth { 0 };
  float uiScale { 1 };
  float titleBarScale { 0.7f };
  float _pad { 10 };
  bool isInitialized { false };

  juce::Label pluginNameLabel;

  NtPluginAudioProcessorEditor(NtPluginAudioProcessor&);
  NtPluginAudioProcessorEditor(NtPluginAudioProcessorEditor&&) = delete;
  ~NtPluginAudioProcessorEditor() override;
  NtPluginAudioProcessorEditor& operator=(
      NtPluginAudioProcessorEditor&&) = delete;

  // "Public".
  void paint(juce::Graphics&) override;
  void resized() override;

  // Callbacks.
  void sliderValueChanged(juce::Slider* slider) override;
  void buttonClicked(juce::Button* button) override;
  void changeListenerCallback(juce::ChangeBroadcaster* source) override;
  void comboBoxChanged(juce::ComboBox* p_box) override;
  void timerCallback() override;

  // Initializers.
  void _initPrimaryKnob(NtFx::KnobSpec& p_spec);
  void _initSecondaryKnob(NtFx::KnobSpec& p_spec);
  void _initKnobGroup(size_t iGroup, NtFx::KnobGroupSpec& p_group);
  void _initGroupKnob(size_t iGroup, NtFx::KnobSpec& p_spec);
  void _initKnob(
      NtFx::KnobSpec& p_spec, std::unique_ptr<juce::Slider>& p_slider);
  void _makeToggle(NtFx::ToggleSpec& spec);
  void _initToggle(NtFx::Toggle* p_toggle, NtFx::ToggleSpec& spec);
  void _initToggleGroup(NtFx::ToggleSetSpec& spec);
  void _initDropDown(NtFx::DropDownSpec& p_spec, bool addToTitleBar = false);
  void _initRadioButton(NtFx::RadioButtonSetSpec& spec);
  void _initWindowSize();
  void _initWindowWidth(int nCols);
  void _initWindowHeight(int nRows);

  // UI update.
  void _updateUi();
  void _placeKnobGroups(juce::Rectangle<int>& area);
  void _placeTitleBar(juce::Rectangle<int>& area);
  void _placeMeters(juce::Rectangle<int>& area);
  void _placeSecondaryKnobs(juce::Rectangle<int>& area);
  void _placePrimaryKnobs(juce::Rectangle<int>& area);
  void _updateColours();
  void _updateUiScale();
  void _updateOversampling();
  void _updateTheme();

  int _getNKnobsInLargestKnobGroup();
  void _placeGruopKnobColumn(juce::Rectangle<int>& groupArea,
      size_t n,
      size_t i,
      size_t nCols,
      bool even);
  void _placeSmallTogglesArea(juce::Rectangle<int>& area);
  void _placeBottomRow(juce::Rectangle<int>& area);
  void _placeDropdowns(juce::Rectangle<int>& area, int columnWidth);
  void _placeToggles(juce::Rectangle<int>& area, int columnWidth);
  template <typename T>
  void _placeSmallToggles(juce::Rectangle<int>& area,
      size_t size,
      std::vector<std::unique_ptr<juce::Label>>& labels,
      std::vector<std::unique_ptr<T>>& toggles);

  // Helpers.
  template <typename T, typename spec_t>
  std::unique_ptr<T> _makeSmallToggleSet(spec_t& spec);
  void _calcSliderRowsCols(size_t nSliders,
      size_t& nRows,
      size_t& nColumns,
      size_t maxRows,
      size_t maxColumns);
  void _makeGrid(juce::Rectangle<int>& area,
      float nRows,
      float nCols,
      float colWidth,
      float rowHeight,
      float pad);
  std::unique_ptr<juce::Label> _makeLabel(const std::string name);
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NtPluginAudioProcessorEditor)
};
