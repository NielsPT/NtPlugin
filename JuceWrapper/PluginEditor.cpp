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

#include "PluginProcessor.h"

#include "PluginEditor.h"

#include "Meter.h"
#include "RadioButtons.h"
#include "Toggle.h"
#include "lib/UiSpec.h"

#include "juce_audio_processors/juce_audio_processors.h"
#include "juce_core/system/juce_PlatformDefs.h"
#include "juce_events/juce_events.h"
#include "juce_graphics/juce_graphics.h"
#include "juce_gui_basics/juce_gui_basics.h"
#include "lib/utils.h"

#include <cstddef>
#include <memory>
#include <string>
#include <utility>
#include <vector>

// TODO: Major refactor. There are SO many repitions in this file.
enum TitleBarDropDowns { e_uiScale, e_theme, e_oversampling };
int NtFx::RadioButtonSet::s_id { 0 };

NtPluginAudioProcessorEditor::NtPluginAudioProcessorEditor(
    NtPluginAudioProcessor& p)
    : AudioProcessorEditor(&p), proc(p),
      meters(proc.plug.uiSpec, proc.plug.meters) {
  this->_updateColours();
  for (auto& k : this->proc.plug.primaryKnobs) { this->_initPrimaryKnob(k); }
  for (auto& k : this->proc.plug.secondaryKnobs) {
    this->_initSecondaryKnob(k);
  }
  for (size_t i = 0; i < this->proc.plug.knobGroups.size(); i++) {
    this->_initKnobGroup(i, this->proc.plug.knobGroups[i]);
  }
  for (auto& t : this->proc.plug.toggles) { this->_initToggle(t); }
  for (auto& g : this->proc.plug.toggleSets) { this->_initToggleGroup(g); }
  for (auto& d : this->proc.plug.dropdowns) { this->_initDropDown(d); }
  for (auto& d : this->proc.plug.radioButtons) { this->_initRadioButton(d); }
  for (auto& d : this->proc.titleBarSpec.dropdowns) {
    this->_initDropDown(d, true);
  }

  this->pluginNameLabel.setText(
      JucePlugin_Name, juce::NotificationType::dontSendNotification);
  this->pluginNameLabel.setJustificationType(juce::Justification::right);
  this->addAndMakeVisible(this->pluginNameLabel);

  this->_initWindowSize();
  this->_updateUiScale();
  this->_updateOversampling();
  this->_updateTheme();

  this->addAndMakeVisible(this->meters);
  this->startTimerHz(this->proc.plug.uiSpec.meterRefreshRate_hz);
  this->isInitialized = true;
  this->_updateUi();
}

NtPluginAudioProcessorEditor::~NtPluginAudioProcessorEditor() {
  for (auto& toggle : this->toggles) { toggle->setLookAndFeel(nullptr); }
  for (auto& slider : this->primaryKnobs) { slider->setLookAndFeel(nullptr); }
  for (auto& slider : this->secondaryKnobs) { slider->setLookAndFeel(nullptr); }
}

void NtPluginAudioProcessorEditor::_initDropDown(
    NtFx::DropDownSpec& spec, bool addToTitleBar) {
  auto p_box = std::make_unique<juce::ComboBox>();
  p_box->setTitle(spec.name);
  for (size_t i = 0; i < spec.options.size(); i++) {
    p_box->addItem(spec.options[i], i + 1);
  }
  p_box->setLookAndFeel(&this->dropDownLookAndFeel);
  p_box->setName(spec.name);
  p_box->addListener(this);
  p_box->setSelectedItemIndex(2, juce::NotificationType::dontSendNotification);
  this->addAndMakeVisible(*p_box);
  auto p_label = this->_makeLabel(spec.name);
  this->dropDownAttachments.emplace_back(
      new juce::AudioProcessorValueTreeState::ComboBoxAttachment(
          this->proc.paramLayout,
          NtFx::spacesToUnderscores(spec.name),
          *p_box));
  if (addToTitleBar) {
    p_box->setColour(
        juce::ComboBox::ColourIds::backgroundColourId, juce::Colours::darkgrey);
    p_box->setColour(
        juce::ComboBox::ColourIds::textColourId, juce::Colours::white);
    this->titleBarDropDowns.push_back(std::move(p_box));
    this->titleBarDropDownLabels.push_back(std::move(p_label));
  } else {
    this->dropDowns.push_back(std::move(p_box));
    this->dropDownLabels.push_back(std::move(p_label));
  }
}

void NtPluginAudioProcessorEditor::_initRadioButton(
    NtFx::RadioButtonSetSpec& spec) {
  auto group = this->_makeSmallToggleSet<NtFx::RadioButtonSet>(spec);
  for (size_t i = 0; i < spec.options.size(); i++) {
    this->toggleAttachments.emplace_back(
        new juce::AudioProcessorValueTreeState::ButtonAttachment(
            this->proc.paramLayout,
            NtFx::mangleName("radioButton", spec.name, spec.options[i]),
            *group->toggles[i].get()));
  }
  this->radioButtons.push_back(std::move(group));
  auto p_label = this->_makeLabel(spec.name);
  this->radioButtonLabels.push_back(std::move(p_label));
}

void NtPluginAudioProcessorEditor::_initToggleGroup(
    NtFx::ToggleSetSpec& r_spec) {
  auto group = this->_makeSmallToggleSet<NtFx::ToggleSet>(r_spec);
  for (size_t i = 0; i < r_spec.toggles.size(); i++) {
    this->toggleAttachments.emplace_back(
        new juce::AudioProcessorValueTreeState::ButtonAttachment(
            this->proc.paramLayout,
            NtFx::mangleName(
                "toggleGroup", r_spec.name, r_spec.toggles[i].name),
            *group->toggles[i].get()));
  }
  this->toggleSets.push_back(std::move(group));
  auto p_label = this->_makeLabel(r_spec.name);
  this->toggleSetLabels.push_back(std::move(p_label));
}

template <typename T, typename spec_t>
std::unique_ptr<T> NtPluginAudioProcessorEditor::_makeSmallToggleSet(
    spec_t& r_spec) {
  auto group = std::make_unique<T>(r_spec, this->proc.plug.uiSpec);
  group->setTitle(r_spec.name);
  group->setName(r_spec.name);
  group->addChangeListener(this);
  group->setColour(juce::Label::ColourIds::textColourId, juce::Colours::white);
  this->addAndMakeVisible(*group);
  return std::move(group);
}

void NtPluginAudioProcessorEditor::_initPrimaryKnob(NtFx::KnobSpec& r_spec) {
  auto name    = NtFx::spacesToUnderscores(r_spec.name);
  auto p_knob  = std::make_unique<juce::Slider>(name);
  auto p_label = this->_makeLabel(name);
  this->_initKnob(r_spec, p_knob, p_label);
  this->primaryKnobs.push_back(std::move(p_knob));
  this->primaryKnobLabels.push_back(std::move(p_label));
}

void NtPluginAudioProcessorEditor::_initSecondaryKnob(NtFx::KnobSpec& r_spec) {
  auto name    = NtFx::spacesToUnderscores(r_spec.name);
  auto p_knob  = std::make_unique<juce::Slider>(name);
  auto p_label = this->_makeLabel(name);
  this->_initKnob(r_spec, p_knob, p_label);
  this->secondaryKnobs.push_back(std::move(p_knob));
  this->secondaryKnobLabels.push_back(std::move(p_label));
}

void NtPluginAudioProcessorEditor::_initKnobGroup(
    int iGroup, NtFx::KnobGroupSpec& r_spec) {
  this->knobGroups.push_back({ });
  this->groupKnobLabels.push_back({ });
  auto p_label = this->_makeLabel(r_spec.name);
  this->knobGroupLabels.push_back(std::move(p_label));
  for (int i = 0; i < r_spec.primaryKnobs.size(); i++) {
    this->_initGroupKnob(iGroup, r_spec.primaryKnobs[i]);
  }
}

void NtPluginAudioProcessorEditor::_initGroupKnob(
    int iGroup, NtFx::KnobSpec& r_spec) {
  auto p_knob  = std::make_unique<juce::Slider>(NtFx::mangleName(
      "knobGroup", this->proc.plug.knobGroups[iGroup].name, r_spec.name));
  auto p_label = this->_makeLabel(r_spec.name);
  this->_initKnob(r_spec, p_knob, p_label);
  this->knobGroups[iGroup].push_back(std::move(p_knob));
  this->groupKnobLabels[iGroup].push_back(std::move(p_label));
}

void NtPluginAudioProcessorEditor::_initKnob(NtFx::KnobSpec& r_spec,
    std::unique_ptr<juce::Slider>& r_slider,
    std::unique_ptr<juce::Label>& r_label) {
  if (!r_spec.p_val) { return; }
  r_slider->setLookAndFeel(&this->knobLookAndFeel);
  r_slider->setTextValueSuffix(r_spec.suffix);
  r_slider->setSliderStyle(juce::Slider::SliderStyle::Rotary);
  r_slider->addListener(this);
  this->addAndMakeVisible(r_slider.get());
  DBG("Creating attachment for slider '" + r_slider->getName() + "'.");
  this->knobAttachments.emplace_back(
      new juce::AudioProcessorValueTreeState::SliderAttachment(
          this->proc.paramLayout, r_slider->getName(), *r_slider));
  r_slider->setRange(r_spec.minVal, r_spec.maxVal);
  if (r_spec.midPoint) { r_slider->setSkewFactorFromMidPoint(r_spec.midPoint); }
}

void NtPluginAudioProcessorEditor::_initToggle(NtFx::ToggleSpec& r_spec) {
  auto p_toggle =
      std::make_unique<NtFx::Toggle>(NtFx::spacesToUnderscores(r_spec.name));
  if (r_spec.p_val) { this->__initToggle(p_toggle.get(), r_spec); }
  this->toggles.push_back(std::move(p_toggle));
}

void NtPluginAudioProcessorEditor::__initToggle(
    NtFx::Toggle* p_toggle, NtFx::ToggleSpec& r_spec) {
  this->addAndMakeVisible(p_toggle);
  p_toggle->setClickingTogglesState(true);
  p_toggle->setToggleable(true);
  p_toggle->addListener(this);
  p_toggle->setButtonText(r_spec.name);
  this->toggleAttachments.emplace_back(
      new juce::AudioProcessorValueTreeState::ButtonAttachment(
          this->proc.paramLayout,
          NtFx::spacesToUnderscores(r_spec.name),
          *p_toggle));
}

void NtPluginAudioProcessorEditor::_initWindowSize() {
  int nRows, nCols;
  this->_calcSliderRowsCols(this->primaryKnobs.size(),
      nRows,
      nCols,
      this->proc.plug.uiSpec.maxRows,
      this->proc.plug.uiSpec.maxColumns);
  this->_initWindowWidth(nCols);
  this->_initWindowHeight(nRows);
}

void NtPluginAudioProcessorEditor::_initWindowWidth(int nCols) {
  auto width = 0;
  if (this->proc.plug.uiSpec.includeMeters) {
    width += this->meters.getMinimalWidth();
  }
  if (this->proc.plug.radioButtons.size()
      || this->proc.plug.toggleSets.size()) {
    width += this->proc.plug.uiSpec.radioButtonAreaWidth;
  }
  auto primKnobsWidth = nCols * this->proc.plug.uiSpec.knobWidth;
  width += primKnobsWidth;
  auto secKnobWidth = this->proc.plug.secondaryKnobs.size()
      * this->proc.plug.uiSpec.secondaryKnobWidth;
  if (secKnobWidth > primKnobsWidth) { width += secKnobWidth - primKnobsWidth; }
  width += this->proc.plug.knobGroups.size() * this->proc.plug.uiSpec.groupWidth
      + this->proc.plug.uiSpec.groupPad;
  this->unscaledWindowWidth = width;
}

void NtPluginAudioProcessorEditor::_initWindowHeight(int nRows) {
  auto height = 0;
  if (this->proc.plug.uiSpec.includeTitleBar) {
    height += this->proc.plug.uiSpec.titleBarHeight;
  }
  auto primHeight = nRows * this->proc.plug.uiSpec.knobHeight;
  height += primHeight;
  if (this->proc.plug.secondaryKnobs.size() != 0) {
    height += this->proc.plug.uiSpec.secondaryKnobHeight;
  }
  if (this->proc.plug.toggles.size() != 0) {
    height += this->proc.plug.uiSpec.toggleHeight;
  }
  if (this->proc.plug.knobGroups.size()) {
    height += this->proc.plug.uiSpec.labelHeight;
    int max = 0;
    for (auto& g : this->proc.plug.knobGroups) {
      if (g.primaryKnobs.size() > max) { max = g.primaryKnobs.size(); }
    }
    auto groupsHeight = (this->proc.plug.uiSpec.groupKnobHeight
                            + this->proc.plug.uiSpec.groupPad)
            * (max % 2 + max / 2)
        + this->proc.plug.uiSpec.groupEvenColOffset * max % 2;
    if (groupsHeight > primHeight) { height += groupsHeight - primHeight; }
  }
  if (this->proc.plug.uiSpec.includeMeters) {
    auto minHeight = this->meters.getMinimalHeight();
    if (height < minHeight) { height = minHeight; }
  }
  this->unscaledWindowHeight = height;
}

void NtPluginAudioProcessorEditor::_calcSliderRowsCols(
    int nKnobs, int& nRows, int& nColumns, int maxRows, int maxColumns) {
  if (nKnobs > maxRows * maxColumns) {
    juce::NativeMessageBox::showMessageBoxAsync(
        juce::MessageBoxIconType::WarningIcon,
        "Bad Grid Layout",
        "Too many parameters. Max is " + std::to_string(maxRows * maxColumns)
            + ".");
    return;
  }
  int bestRows    = 1;
  int bestColumns = nKnobs;
  int minCells    = std::numeric_limits<int>::max();
  for (int r = 1; r <= maxRows; ++r) {
    int c = (nKnobs + r - 1) / r;
    if (c > maxColumns) { continue; }
    int cells = r * c;
    if (cells < minCells) {
      minCells    = cells;
      bestRows    = r;
      bestColumns = c;
    }
  }
  nRows    = bestRows;
  nColumns = bestColumns;
}

void NtPluginAudioProcessorEditor::paint(juce::Graphics& g) {
  g.fillAll(juce::Colour(this->proc.plug.uiSpec.backgroundColour));
  g.setColour(juce::Colours::darkgrey);
  for (size_t i = 0; i < grayAreas.size(); i++) {
    g.fillRect(this->grayAreas[i]);
  }
  float pad = 15;
  g.setColour(juce::Colour(this->proc.plug.uiSpec.foregroundColour));
  for (auto area : this->borderedAreas) {
    g.drawRoundedRectangle(area.toFloat(), pad * this->uiScale, this->uiScale);
  }
}

void NtPluginAudioProcessorEditor::resized() {
  DBG("Resized");
  this->_updateUi();
}

void NtPluginAudioProcessorEditor::_updateUi() {
  if (!this->isInitialized) { return; }
  this->grayAreas.clear();
  this->borderedAreas.clear();
  this->knobLookAndFeel.fontSize =
      this->proc.plug.uiSpec.defaultFontSize * this->uiScale;
  this->knobLookAndFeel.uiScale      = this->uiScale;
  this->dropDownLookAndFeel.fontSize = this->proc.plug.uiSpec.defaultFontSize
      * this->uiScale * this->titleBarScale;
  this->dropDownLookAndFeel.uiScale = this->uiScale * this->titleBarScale;

  auto area = this->getLocalBounds();
  if (this->proc.plug.uiSpec.includeTitleBar) { this->_updateTitleBar(area); }
  this->pad = 10 * this->uiScale;
  area.reduce(this->pad, this->pad);
  if (this->proc.plug.uiSpec.includeMeters
      && this->proc.plug.meters.size() != 0) {
    this->_updateMeters(area);
  }
  if (this->proc.plug.radioButtons.size()
      || this->proc.plug.toggleSets.size()) {
    this->_placeSmallTogglesArea(area);
  }
  if (this->proc.plug.toggles.size()) { this->_placeBottomRow(area); }
  if (this->proc.plug.uiSpec.includeSecondaryKnobs
      && this->proc.plug.secondaryKnobs.size()) {
    this->_updateSecondaryKnobs(area);
  }
  if (this->proc.plug.knobGroups.size()) { this->_updateKnobGroups(area); }
  if (this->proc.plug.primaryKnobs.size()) { this->_updatePrimaryKnobs(area); }
  this->repaint();
}

void NtPluginAudioProcessorEditor::_updateTitleBar(juce::Rectangle<int>& area) {
  auto pad = 4.0f * this->uiScale;
  auto titleBarArea =
      area.removeFromTop(this->proc.plug.uiSpec.titleBarHeight * this->uiScale);
  this->grayAreas.push_back(titleBarArea);
  titleBarArea.reduce(pad, pad);
  for (int i = 0; i < this->proc.titleBarSpec.dropdowns.size(); i++) {
    auto font       = juce::FontOptions(this->proc.plug.uiSpec.defaultFontSize
        * this->uiScale * this->titleBarScale);
    auto labelWidth = (juce::TextLayout::getStringWidth(juce::AttributedString(
                          this->proc.titleBarSpec.dropdowns[i].name)))
        * this->uiScale;
    auto options           = this->proc.titleBarSpec.dropdowns[i].options;
    float minDropDownWidth = 0;
    for (auto option : options) {
      auto w = juce::TextLayout::getStringWidth(juce::AttributedString(option));
      if (w > minDropDownWidth) { minDropDownWidth = w; }
    }
    auto dropDownWidth = (minDropDownWidth + 50) * this->uiScale;
    this->titleBarDropDownLabels[i]->setFont(font);
    this->titleBarDropDownLabels[i]->setColour(
        juce::Label::ColourIds::textColourId, juce::Colours::white);
    this->titleBarDropDownLabels[i]->setBounds(
        titleBarArea.removeFromLeft(labelWidth));
    this->titleBarDropDowns[i]->setBounds(
        titleBarArea.removeFromLeft(dropDownWidth));
  }
  this->pluginNameLabel.setFont(
      juce::FontOptions(this->proc.plug.uiSpec.defaultFontSize * this->uiScale,
          juce::Font::FontStyleFlags::italic));
  this->pluginNameLabel.setBounds(titleBarArea);
}

void NtPluginAudioProcessorEditor::_updateMeters(juce::Rectangle<int>& area) {
  this->meters.setOnlyShowLeft(this->proc.monoMode);
  auto meterArea =
      area.removeFromLeft(this->meters.getMinimalWidth() * this->uiScale);
  this->meters.setFontSize(
      this->proc.plug.uiSpec.defaultFontSize * this->uiScale * 0.9);
  this->meters.setUiScale(this->uiScale);
  this->meters.updateRelease(this->proc.plug.uiSpec.meterRefreshRate_hz);
  this->meters.setBounds(meterArea);
  this->borderedAreas.push_back(meterArea);
}

void NtPluginAudioProcessorEditor::_placeSmallTogglesArea(
    juce::Rectangle<int>& area) {
  auto _area = area.removeFromRight(
      this->proc.plug.uiSpec.radioButtonAreaWidth * this->uiScale);
  this->borderedAreas.push_back(_area);
  float pad = 7 * this->uiScale;
  _area.removeFromLeft(pad);
  this->_placeSmallToggles(_area,
      this->proc.plug.radioButtons.size(),
      this->radioButtonLabels,
      this->radioButtons);
  this->_placeSmallToggles(_area,
      this->proc.plug.toggleSets.size(),
      this->toggleSetLabels,
      this->toggleSets);
}

template <typename T>
void NtPluginAudioProcessorEditor::_placeSmallToggles(
    juce::Rectangle<int>& area,
    int size,
    std::vector<std::unique_ptr<juce::Label>>& labels,
    std::vector<std::unique_ptr<T>>& toggles) {
  for (size_t i = 0; i < size; i++) {
    labels[i]->setFont(juce::FontOptions(
        this->proc.plug.uiSpec.defaultFontSize * this->uiScale));
    area.removeFromTop(pad);
    labels[i]->setBounds(
        area.removeFromTop(this->proc.plug.uiSpec.labelHeight * this->uiScale));
    toggles[i]->uiScale = this->uiScale;
    toggles[i]->updateUi();
    toggles[i]->setBounds(area.removeFromTop(toggles[i]->toggles.size()
        * this->proc.plug.uiSpec.radioButtonHeight * this->uiScale));
  }
}

void NtPluginAudioProcessorEditor::_placeBottomRow(juce::Rectangle<int>& area) {
  auto bottomRowArea = area.removeFromBottom(
      this->proc.plug.uiSpec.toggleHeight * this->uiScale);
  this->borderedAreas.push_back(bottomRowArea);
  auto nToggles    = this->proc.plug.toggles.size();
  auto nDropdowns  = this->proc.plug.dropdowns.size();
  auto nElements   = nToggles + nDropdowns * 2;
  auto columnWidth = bottomRowArea.getWidth() / nElements;
  this->_placeDropdowns(bottomRowArea, columnWidth);
  this->_placeToggles(bottomRowArea, columnWidth);
}

void NtPluginAudioProcessorEditor::_placeDropdowns(
    juce::Rectangle<int>& area, size_t columnWidth) {
  for (size_t i = 0; i < this->proc.plug.dropdowns.size(); i++) {
    auto dropdownArea = area.removeFromLeft(columnWidth * 2);
    auto labelArea    = dropdownArea.removeFromLeft(columnWidth);
    dropdownArea.reduce(this->pad, this->pad);
    labelArea.reduce(this->pad, this->pad);
    this->dropDowns[i]->setBounds(dropdownArea);
    this->dropDowns[i]->setColour(juce::ComboBox::ColourIds::textColourId,
        juce::Colour(this->proc.plug.uiSpec.foregroundColour));
    this->dropDowns[i]->setColour(juce::ComboBox::ColourIds::arrowColourId,
        juce::Colour(this->proc.plug.uiSpec.foregroundColour));
    this->dropDowns[i]->setColour(juce::ComboBox::ColourIds::backgroundColourId,
        juce::Colour(this->proc.plug.uiSpec.backgroundColour));
    this->dropDownLabels[i]->setBounds(labelArea);
    this->dropDownLabels[i]->setFont(juce::FontOptions(
        this->proc.plug.uiSpec.defaultFontSize * this->uiScale));
  }
}
void NtPluginAudioProcessorEditor::_placeToggles(
    juce::Rectangle<int>& area, size_t columnWidth) {
  for (size_t i = 0; i < this->proc.plug.toggles.size(); i++) {
    auto toggleArea = area.removeFromLeft(columnWidth);
    toggleArea.reduce(this->pad, this->pad);
    this->toggles[i]->setBounds(toggleArea);
    this->toggles[i]->fontSize =
        this->proc.plug.uiSpec.defaultFontSize * this->uiScale;
    this->toggles[i]->colour = this->proc.plug.uiSpec.foregroundColour;
  }
}

// TODO: DRY. make a general _upadeteKnob function.
void NtPluginAudioProcessorEditor::_updateSecondaryKnobs(
    juce::Rectangle<int>& area) {
  auto secondaryKnobsArea = area.removeFromBottom(
      this->proc.plug.uiSpec.secondaryKnobHeight * this->uiScale);
  this->borderedAreas.push_back(secondaryKnobsArea);
  secondaryKnobsArea.reduce(this->pad, this->pad);
  for (size_t i = 0; i < this->proc.plug.secondaryKnobs.size(); i++) {
    auto knobArea = secondaryKnobsArea.removeFromLeft(
        this->proc.plug.uiSpec.secondaryKnobWidth * this->uiScale);
    auto labelArea = knobArea.removeFromTop(
        this->proc.plug.uiSpec.labelHeight * this->uiScale);
    this->secondaryKnobLabels[i]->setBounds(labelArea);
    this->secondaryKnobs[i]->setBounds(knobArea);
    this->secondaryKnobLabels[i]->setFont(juce::FontOptions(
        this->proc.plug.uiSpec.defaultFontSize * this->uiScale));
    this->secondaryKnobs[i]->setTextBoxStyle(juce::Slider::TextBoxBelow,
        false,
        80 * this->uiScale,
        this->proc.plug.uiSpec.labelHeight * this->uiScale);
    this->secondaryKnobs[i]->setEnabled(
        this->proc.plug.secondaryKnobs[i].isActive);
  }
}

void NtPluginAudioProcessorEditor::_updateKnobGroups(
    juce::Rectangle<int>& area) {
  for (int i = 0; i < this->proc.plug.knobGroups.size(); i++) {
    auto groupArea = area.removeFromLeft(this->proc.plug.uiSpec.groupWidth);
    this->borderedAreas.push_back(groupArea);
    auto groupLableArea =
        groupArea.removeFromTop(this->proc.plug.uiSpec.labelHeight);
    this->knobGroupLabels[i]->setBounds(groupLableArea);
    int nLhs = 0;
    int nRhs = 0;
    auto& g  = this->proc.plug.knobGroups[i];
    for (size_t j = 0; j < g.primaryKnobs.size(); j++) {
      if (!(j % 2)) {
        nLhs++;
      } else {
        nRhs++;
      }
    }
    auto lArea = groupArea.removeFromLeft(
        this->proc.plug.uiSpec.groupKnobWidth * this->uiScale);
    this->_placeGruopKnobColumn(lArea, nLhs, i, false);
    groupArea.removeFromTop(
        this->proc.plug.uiSpec.groupEvenColOffset * this->uiScale);
    this->_placeGruopKnobColumn(groupArea, nRhs, i, true);
  }
}

void NtPluginAudioProcessorEditor::_placeGruopKnobColumn(
    juce::Rectangle<int>& groupArea, int n, int i, bool even) {
  for (size_t j = 0; j < n; j++) {
    auto kArea = groupArea.removeFromTop(
        this->proc.plug.uiSpec.groupKnobHeight * this->uiScale);
    kArea.removeFromTop(10 * this->uiScale);
    auto labelArea =
        kArea.removeFromTop(this->proc.plug.uiSpec.labelHeight * this->uiScale);
    if (!this->groupKnobLabels[i][j * 2 + even]) { continue; }
    this->groupKnobLabels[i][j * 2 + even]->setBounds(labelArea);
    this->knobGroups[i][j * 2 + even]->setBounds(kArea);
    this->groupKnobLabels[i][j * 2 + even]->setFont(juce::FontOptions(
        this->proc.plug.uiSpec.defaultFontSize * this->uiScale));
    this->knobGroups[i][j * 2 + even]->setTextBoxStyle(
        juce::Slider::TextBoxBelow,
        false,
        80 * this->uiScale,
        this->proc.plug.uiSpec.labelHeight * this->uiScale);
    this->knobGroups[i][j * 2 + even]->setEnabled(
        this->proc.plug.knobGroups[i].primaryKnobs[j * 2 + even].isActive);
  }
}

void NtPluginAudioProcessorEditor::_updatePrimaryKnobs(
    juce::Rectangle<int>& area) {
  auto pad    = 10 * this->uiScale;
  auto nKnobs = this->proc.plug.primaryKnobs.size();
  int nColumns;
  int nRows;
  this->_calcSliderRowsCols(nKnobs,
      nRows,
      nColumns,
      this->proc.plug.uiSpec.maxRows,
      this->proc.plug.uiSpec.maxColumns);
  auto knobsArea = area;
  this->borderedAreas.push_back(knobsArea);
  size_t iKnob     = 0;
  auto columnWidth = knobsArea.getWidth() / nColumns;
  auto rowHeight   = knobsArea.getHeight() / nRows;
  if (rowHeight > this->proc.plug.uiSpec.knobHeight * this->uiScale) {
    rowHeight = this->proc.plug.uiSpec.knobHeight * this->uiScale;
  }
  for (size_t i = 0; i < nRows; i++) {
    auto rowArea = knobsArea.removeFromTop(rowHeight);
    rowArea.removeFromTop(pad);
    rowArea.removeFromBottom(pad);
    for (size_t j = 0; j < nColumns; j++) {
      if (iKnob >= nKnobs) { break; }
      auto knobArea  = rowArea.removeFromLeft(columnWidth);
      auto labelArea = knobArea.removeFromTop(
          this->proc.plug.uiSpec.labelHeight * this->uiScale);
      this->primaryKnobLabels[iKnob]->setBounds(labelArea);
      this->primaryKnobs[iKnob]->setBounds(knobArea);
      this->primaryKnobs[iKnob]->setEnabled(
          this->proc.plug.primaryKnobs[iKnob].isActive);
      iKnob++;
    }
  }
  for (auto& k : this->primaryKnobs) {
    k->setTextBoxStyle(juce::Slider::TextBoxBelow,
        false,
        80 * this->uiScale,
        this->proc.plug.uiSpec.labelHeight * this->uiScale);
  }
  for (auto& l : this->primaryKnobLabels) {
    l->setFont(juce::FontOptions(
        this->proc.plug.uiSpec.defaultFontSize * this->uiScale));
  }
}

void NtPluginAudioProcessorEditor::_updateColours() {
  this->knobLookAndFeel.backgroundColour =
      this->proc.plug.uiSpec.backgroundColour;
  this->knobLookAndFeel.foregroundColour =
      this->proc.plug.uiSpec.foregroundColour;
  this->knobLookAndFeel.setColour(
      juce::Slider::ColourIds::textBoxBackgroundColourId,
      juce::Colour(this->proc.plug.uiSpec.backgroundColour));
  this->knobLookAndFeel.setColour(juce::Slider::ColourIds::textBoxTextColourId,
      juce::Colour(this->proc.plug.uiSpec.foregroundColour));
  this->getLookAndFeel().setColour(juce::Slider::ColourIds::textBoxTextColourId,
      juce::Colour(this->proc.plug.uiSpec.foregroundColour));
  this->getLookAndFeel().setColour(
      juce::Slider::ColourIds::textBoxBackgroundColourId,
      juce::Colour(this->proc.plug.uiSpec.backgroundColour));
  this->getLookAndFeel().setColour(juce::Label::ColourIds::textColourId,
      juce::Colour(this->proc.plug.uiSpec.foregroundColour));
  this->knobLookAndFeel.setColour(juce::ComboBox::ColourIds::backgroundColourId,
      juce::Colour(this->proc.plug.uiSpec.backgroundColour));
  this->knobLookAndFeel.setColour(juce::ComboBox::ColourIds::textColourId,
      juce::Colour(this->proc.plug.uiSpec.foregroundColour));
  for (auto& knob : this->primaryKnobs) { knob->lookAndFeelChanged(); }
  for (auto& knob : this->secondaryKnobs) { knob->lookAndFeelChanged(); }
}

void NtPluginAudioProcessorEditor::timerCallback() {
  for (size_t i = 0; i < this->meters.meters.size(); i++) {
    this->meters.refresh(
        i, this->proc.plug.getAndResetPeakLevel(i), this->proc.plug.getRms(i));
  }
  if (this->proc.plug.uiNeedsUpdate) {
    this->_updateUi();
    this->proc.plug.uiNeedsUpdate = false;
  }
}

void NtPluginAudioProcessorEditor::sliderValueChanged(juce::Slider* p_slider) {
  auto name   = p_slider->getName().toStdString();
  auto* p_val = this->proc.plug.getKnobValuePtr(name);
  if (!p_val) {
    DBG("Knob name in UI not found in plugin knobs: '" + name + '.');
    return;
  }
  *p_val = p_slider->getValue();
  this->proc.plug.update();
}

void NtPluginAudioProcessorEditor::buttonClicked(juce::Button* p_button) {
  auto name   = p_button->getName().toStdString();
  auto* p_val = this->proc.plug.getToggleValuePtr(name);
  if (!p_val) {
    DBG("Toggle name in UI not found in plugin toggles: '" + name + "'.");
    return;
  }
  *p_val = p_button->getToggleState();
  this->proc.plug.update();
}

void NtPluginAudioProcessorEditor::changeListenerCallback(
    juce::ChangeBroadcaster* p_b) {
  for (size_t i = 0; i < this->proc.plug.radioButtons.size(); i++) {
    auto& r = this->radioButtons[i];
    if (p_b != r.get()) { continue; }
    this->proc.plug.update();
    this->proc.plug.uiNeedsUpdate = true;
    return;
  }
  // TODO: Look at this mess...
  for (size_t i = 0; i < this->proc.plug.toggleSets.size(); i++) {
    auto& g = this->toggleSets[i];
    if (p_b != g.get()) { continue; }
    for (size_t j = 0; j < this->proc.plug.toggleSets[i].toggles.size(); j++) {
      auto& t = this->proc.plug.toggleSets[i].toggles[j];
      if (!t.p_val) {
        DBG("Toggle value in group is null.");
        continue;
      }
      *t.p_val = g->toggles[j]->getToggleState();
    }
    this->proc.plug.update();
    this->proc.plug.uiNeedsUpdate = true;
    return;
  }
  DBG("RadioButton name in UI not found in plugin toggles.");
}

void NtPluginAudioProcessorEditor::comboBoxChanged(juce::ComboBox* p_box) {
  if (this->titleBarDropDowns.size() < 3) { return; }
  if (p_box == this->titleBarDropDowns[e_uiScale].get()) {
    this->_updateUiScale();
  }
  if (p_box == this->titleBarDropDowns[e_theme].get()) { this->_updateTheme(); }
  if (p_box == this->titleBarDropDowns[e_oversampling].get()) {
    this->_updateOversampling();
  }
  auto name  = p_box->getName().toStdString();
  auto p_val = this->proc.plug.getDropDownValuePtr(name);
  if (!p_val) {
    DBG("Failed to get dropdown value for '" + name + "'.");
    return;
  }
  *p_val = p_box->getSelectedId() - 1;
  this->proc.plug.update();
}

void NtPluginAudioProcessorEditor::_updateUiScale() {
  auto p_box    = this->titleBarDropDowns[e_uiScale].get();
  this->uiScale = 0.5 + 0.25 * (p_box->getSelectedId() - 1);
  this->setSize(this->unscaledWindowWidth * this->uiScale,
      this->unscaledWindowHeight * this->uiScale);
}

void NtPluginAudioProcessorEditor::_updateOversampling() {
  auto p_box = this->titleBarDropDowns[e_oversampling].get();
  auto mode  = p_box->getSelectedId();
  if (this->proc.src.mode == mode) { return; }
  this->proc.updateOversampling(mode);
}

void NtPluginAudioProcessorEditor::_updateTheme() {
  if (!(this->proc.plug.uiSpec.backgroundColour == 0xFF000000
          || this->proc.plug.uiSpec.backgroundColour == 0xFFFFFFFF)
      || !(this->proc.plug.uiSpec.foregroundColour == 0xFF000000
          || this->proc.plug.uiSpec.foregroundColour == 0xFFFFFFFF)) {
    return;
  }
  auto p_box = this->titleBarDropDowns[e_theme].get();
  auto val   = p_box->getSelectedId();
  switch (val) {
  case 1:
    this->proc.plug.uiSpec.foregroundColour = 0xFF000000;
    this->proc.plug.uiSpec.backgroundColour = 0xFFFFFFFF;
    break;
  case 2:
    this->proc.plug.uiSpec.foregroundColour = 0xFFFFFFFF;
    this->proc.plug.uiSpec.backgroundColour = 0xFF000000;
    break;
  default:
    break;
  }
  this->_updateColours();
  this->_updateUi();
}

std::unique_ptr<juce::Label> NtPluginAudioProcessorEditor::_makeLabel(
    const std::string name) {
  auto labelText = name;
  std::replace(labelText.begin(), labelText.end(), '_', ' ');
  auto p_label = std::make_unique<juce::Label>(name);
  p_label->setText(labelText, juce::NotificationType::dontSendNotification);
  p_label->setJustificationType(juce::Justification::centred);
  this->addAndMakeVisible(*p_label);
  return std::move(p_label);
}
