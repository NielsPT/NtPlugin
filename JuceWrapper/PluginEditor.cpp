/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include <algorithm>
#include <string>

#include "Meter.h"
#include "PluginEditor.h"
#include "PluginProcessor.h"
#include "Toggle.h"
#include "juce_gui_basics/juce_gui_basics.h"

//==============================================================================
NtCompressorAudioProcessorEditor::NtCompressorAudioProcessorEditor(
    NtCompressorAudioProcessor& p)
    : AudioProcessorEditor(&p), proc(p), meters(proc.plug.guiSpec) {
  this->getLookAndFeel().setColour(
      juce::ResizableWindow::ColourIds::backgroundColourId,
      juce::Colours::black);

  for (auto& k : this->proc.plug.primaryKnobs) { this->initPrimaryKnob(k); }
  for (auto& k : this->proc.plug.secondaryKnobs) { this->initSecondaryKnob(k); }
  for (auto& t : this->proc.plug.toggles) { this->initToggle(t); }
  for (auto& d : this->proc.titleBarSpec.dropDowns) { this->initDropDown(d); }

  int nRows, nCols;
  this->calcSliderRowsCols(this->allPrimaryKnobs.size(),
      nRows,
      nCols,
      this->proc.plug.guiSpec.maxRows,
      this->proc.plug.guiSpec.maxColumns);
  auto height = 0;
  if (this->proc.plug.guiSpec.includeTitleBar) {
    height += this->proc.plug.guiSpec.titleBarHeight;
  }
  height += nRows * this->proc.plug.guiSpec.knobHeight;
  if (this->proc.plug.secondaryKnobs.size() != 0) {
    height += this->proc.plug.guiSpec.secondaryKnobHeight;
  }
  if (this->proc.plug.toggles.size() != 0) {
    height += this->proc.plug.guiSpec.toggleHeight;
  }
  if (this->proc.plug.guiSpec.includeMeters) {
    auto minHeight = this->meters.getMinimalHeight();
    if (height < minHeight) { height = minHeight; }
  }
  this->unscaledWindowHeight = height;
  this->updateUiScale();

  this->knobLookAndFeel.foregroundColour =
      this->proc.plug.guiSpec.backgroundColour;
  this->knobLookAndFeel.backgroundColour =
      this->proc.plug.guiSpec.foregroundColour & 0x00FFFFFF | 0xDD000000;
  this->addAndMakeVisible(this->meters);
  this->startTimerHz(this->proc.plug.guiSpec.meterRefreshRate_hz);
  this->isInitialized = true;
  this->updateUi();
}

NtCompressorAudioProcessorEditor::~NtCompressorAudioProcessorEditor() {
  for (auto& toggle : this->allToggles) { toggle->setLookAndFeel(nullptr); }
  for (auto& slider : this->allPrimaryKnobs) {
    slider->setLookAndFeel(nullptr);
  }
  for (auto& slider : this->allSecondaryKnobs) {
    slider->setLookAndFeel(nullptr);
  }
}

void NtCompressorAudioProcessorEditor::initDropDown(NtFx::DropDownSpec& spec) {
  auto p_box = std::make_unique<juce::ComboBox>();
  p_box->setTitle(spec.name);
  for (size_t i = 0; i < spec.options.size(); i++) {
    std::string option = spec.options[i];
    std::replace(option.begin(), option.end(), '_', ' ');
    std::transform(option.begin(), option.end(), option.begin(), ::toupper);
    p_box->addItem(option, i + 1);
  }
  p_box->setSelectedItemIndex(2, juce::NotificationType::dontSendNotification);
  p_box->setColour(
      juce::ComboBox::ColourIds::backgroundColourId, juce::Colours::darkgrey);
  p_box->addListener(this);
  this->addAndMakeVisible(*p_box);
  auto p_label = std::make_unique<juce::Label>(spec.name);
  p_label->setJustificationType(juce::Justification::right);
  std::string name = spec.name;
  p_label->setText(name, juce::NotificationType::dontSendNotification);
  this->addAndMakeVisible(*p_label);
  this->allDropDownAttachments.emplace_back(
      new juce::AudioProcessorValueTreeState::ComboBoxAttachment(
          this->proc.parameters, spec.name, *p_box));
  this->allDropDowns.push_back(std::move(p_box));
  this->allDropDownLables.push_back(std::move(p_label));
}

void NtCompressorAudioProcessorEditor::initPrimaryKnob(
    NtFx::KnobSpec<float>& spec) {
  auto p_knob  = std::make_unique<juce::Slider>(spec.name);
  auto p_label = std::make_unique<juce::Label>(spec.name);
  this->_initKnob(spec, p_knob, p_label);
  this->allPrimaryKnobs.push_back(std::move(p_knob));
  this->allPrimaryKnobLabels.push_back(std::move(p_label));
}

void NtCompressorAudioProcessorEditor::initSecondaryKnob(
    NtFx::KnobSpec<float>& spec) {
  std::string name = spec.name;
  auto p_knob      = std::make_unique<juce::Slider>(name);
  auto p_label     = std::make_unique<juce::Label>(name);
  this->_initKnob(spec, p_knob, p_label);
  this->allSecondaryKnobs.push_back(std::move(p_knob));
  this->allSecondaryKnobLabels.push_back(std::move(p_label));
}

void NtCompressorAudioProcessorEditor::_initKnob(NtFx::KnobSpec<float>& spec,
    std::unique_ptr<juce::Slider>& p_slider,
    std::unique_ptr<juce::Label>& p_label) {
  p_slider->setLookAndFeel(&this->knobLookAndFeel);
  p_label->setJustificationType(juce::Justification::centred);
  std::string name(spec.name);
  std::replace(name.begin(), name.end(), '_', ' ');
  p_label->setText(name, juce::NotificationType::dontSendNotification);
  p_label->setColour(juce::Label::ColourIds::textColourId,
      juce::Colour(this->proc.plug.guiSpec.foregroundColour));
  p_slider->setTextValueSuffix(spec.suffix);
  p_slider->setSliderStyle(juce::Slider::SliderStyle::Rotary);
  p_slider->addListener(this);
  p_slider->setColour(juce::Slider::ColourIds::textBoxTextColourId,
      juce::Colour(this->proc.plug.guiSpec.foregroundColour));
  p_slider->setColour(juce::Slider::ColourIds::textBoxBackgroundColourId,
      juce::Colour(this->proc.plug.guiSpec.backgroundColour));
  addAndMakeVisible(p_slider.get());
  addAndMakeVisible(p_label.get());
  this->allKnobAttachments.emplace_back(
      new juce::AudioProcessorValueTreeState::SliderAttachment(
          this->proc.parameters, spec.name, *p_slider));
  p_slider->setRange(spec.minVal, spec.maxVal);
  if (spec.midPoint) { p_slider->setSkewFactorFromMidPoint(spec.midPoint); }
}

void NtCompressorAudioProcessorEditor::initToggle(NtFx::ToggleSpec& spec) {
  auto p_button = std::make_unique<NtFx::Toggle>(spec.name);
  addAndMakeVisible(p_button.get());
  p_button->setClickingTogglesState(true);
  p_button->setToggleable(true);
  p_button->addListener(this);
  std::string name(spec.name);
  std::replace(name.begin(), name.end(), '_', ' ');
  p_button->setButtonText(name);
  this->allToggleAttachments.emplace_back(
      new juce::AudioProcessorValueTreeState::ButtonAttachment(
          this->proc.parameters, spec.name, *p_button));
  this->allToggles.push_back(std::move(p_button));
}

void NtCompressorAudioProcessorEditor::paint(juce::Graphics& g) {
  g.fillAll(juce::Colour(this->proc.plug.guiSpec.backgroundColour));
  g.setColour(juce::Colours::darkgrey);
  for (size_t i = 0; i < grayAreas.size(); i++) {
    g.fillRect(this->grayAreas[i]);
  }
  float pad = 15;
  g.setColour(juce::Colour(this->proc.plug.guiSpec.foregroundColour));
  for (auto area : this->borderedAreas) {
    g.drawRoundedRectangle(area.toFloat(), pad * this->uiScale, this->uiScale);
  }
}

void NtCompressorAudioProcessorEditor::resized() {
  DBG("Resized");
  this->updateUi();
}

void NtCompressorAudioProcessorEditor::updateUi() {
  if (!this->isInitialized) { return; }
  this->grayAreas.clear();
  this->borderedAreas.clear();
  auto area = this->getLocalBounds();
  if (this->proc.plug.guiSpec.includeTitleBar) { this->updateTitleBar(area); }
  auto pad = 10 * this->uiScale;
  area.reduce(pad, pad);
  if (this->proc.plug.guiSpec.includeMeters) { this->updateMeters(area); }
  if (this->proc.plug.toggles.size()) { this->updateToggles(area); }
  this->knobLookAndFeel.fontSize =
      this->proc.plug.guiSpec.defaultFontSize * this->uiScale;
  if (this->proc.plug.guiSpec.includeSecondaryKnobs
      && this->proc.plug.secondaryKnobs.size()) {
    this->updateSecondaryKnobs(area);
  }
  this->updatePrimaryKnobs(area);
  this->repaint();
}

void NtCompressorAudioProcessorEditor::updateTitleBar(
    juce::Rectangle<int>& area) {
  auto pad          = 3.0f * this->uiScale;
  auto titleBarArea = area.removeFromTop(
      this->proc.plug.guiSpec.titleBarHeight * this->uiScale);
  this->grayAreas.push_back(titleBarArea);
  titleBarArea.reduce(pad, pad);
  for (int i = 0; i < this->proc.titleBarSpec.dropDowns.size(); i++) {
    this->allDropDownLables[i]->setFont(juce::FontOptions(
        this->proc.plug.guiSpec.defaultFontSize * this->uiScale * 0.6));
    this->allDropDownLables[i]->setBounds(
        titleBarArea.removeFromLeft(100 * this->uiScale));
    this->allDropDowns[i]->setBounds(
        titleBarArea.removeFromLeft(100 * this->uiScale));
  }
}
void NtCompressorAudioProcessorEditor::updateMeters(
    juce::Rectangle<int>& area) {
  auto meterArea =
      area.removeFromLeft(this->meters.getMinimalWidth() * this->uiScale);
  this->meters.setFontSize(
      this->proc.plug.guiSpec.defaultFontSize * this->uiScale * 0.9);
  this->meters.setBounds(meterArea);
  this->borderedAreas.push_back(meterArea);
}

void NtCompressorAudioProcessorEditor::updateToggles(
    juce::Rectangle<int>& area) {
  // TODO: wrap to next row if too many.
  auto togglesArea = area.removeFromBottom(
      this->proc.plug.guiSpec.toggleHeight * this->uiScale);
  auto togglePad = 10 * this->uiScale;
  this->borderedAreas.push_back(togglesArea);
  auto nToggles    = this->proc.plug.toggles.size();
  auto columnWidth = togglesArea.getWidth() / nToggles;
  for (size_t i = 0; i < nToggles; i++) {
    auto toggleArea = togglesArea.removeFromLeft(columnWidth);
    this->allToggles[i]->setBounds(toggleArea);
    this->allToggles[i]->fontSize =
        this->proc.plug.guiSpec.defaultFontSize * this->uiScale;
    this->allToggles[i]->colour = this->proc.plug.guiSpec.foregroundColour;
  }
}
void NtCompressorAudioProcessorEditor::updateSecondaryKnobs(
    juce::Rectangle<int>& area) {
  // TODO: wrap to next row if too many.
  auto pad                = 10 * this->uiScale;
  auto secondaryKnobsArea = area.removeFromBottom(
      this->proc.plug.guiSpec.secondaryKnobHeight * this->uiScale);
  this->borderedAreas.push_back(secondaryKnobsArea);
  secondaryKnobsArea.reduce(pad, pad);
  for (size_t i = 0; i < this->proc.plug.secondaryKnobs.size(); i++) {
    auto knobArea = secondaryKnobsArea.removeFromLeft(
        this->proc.plug.guiSpec.secondaryKnobWidth * this->uiScale);
    auto labelArea = knobArea.removeFromTop(
        this->proc.plug.guiSpec.labelHeight * this->uiScale);
    this->allSecondaryKnobLabels[i]->setBounds(labelArea);
    this->allSecondaryKnobs[i]->setBounds(knobArea);
    this->allSecondaryKnobLabels[i]->setFont(juce::FontOptions(
        this->proc.plug.guiSpec.defaultFontSize * this->uiScale));
    this->allSecondaryKnobs[i]->setTextBoxStyle(juce::Slider::TextBoxBelow,
        false,
        80 * this->uiScale,
        this->proc.plug.guiSpec.labelHeight * this->uiScale);
  }
}

void NtCompressorAudioProcessorEditor::updatePrimaryKnobs(
    juce::Rectangle<int>& area) {
  auto pad    = 10 * this->uiScale;
  auto nKnobs = this->proc.plug.primaryKnobs.size();
  int nColumns;
  int nRows;
  this->calcSliderRowsCols(nKnobs,
      nRows,
      nColumns,
      this->proc.plug.guiSpec.maxRows,
      this->proc.plug.guiSpec.maxColumns);
  auto knobsArea = area;
  this->borderedAreas.push_back(knobsArea);
  size_t iSlider   = 0;
  auto columnWidth = knobsArea.getWidth() / nColumns;
  auto rowHeight   = knobsArea.getHeight() / nRows;
  // TODO: We're multiplying by uiScale a milion places. Can it be more
  // abstract?
  if (rowHeight > this->proc.plug.guiSpec.knobHeight * this->uiScale) {
    rowHeight = this->proc.plug.guiSpec.knobHeight * this->uiScale;
  }
  for (size_t i = 0; i < nRows; i++) {
    auto rowArea = knobsArea.removeFromTop(rowHeight);
    rowArea.removeFromTop(pad);
    rowArea.removeFromBottom(pad);
    for (size_t j = 0; j < nColumns; j++) {
      if (iSlider >= nKnobs) { break; }
      auto knobArea  = rowArea.removeFromLeft(columnWidth);
      auto labelArea = knobArea.removeFromTop(
          this->proc.plug.guiSpec.labelHeight * this->uiScale);
      this->allPrimaryKnobLabels[iSlider]->setBounds(labelArea);
      this->allPrimaryKnobs[iSlider]->setBounds(knobArea);
      iSlider++;
    }
  }
  // TODO: add bool scalingChanged and check it.
  for (auto& k : this->allPrimaryKnobs) {
    k->setTextBoxStyle(juce::Slider::TextBoxBelow,
        false,
        80 * this->uiScale,
        this->proc.plug.guiSpec.labelHeight * this->uiScale);
  }
  for (auto& l : this->allPrimaryKnobLabels) {
    l->setFont(juce::FontOptions(
        this->proc.plug.guiSpec.defaultFontSize * this->uiScale));
  }
}

void NtCompressorAudioProcessorEditor::timerCallback() {
  for (size_t i = 0; i < this->meters.size(); i++) {
    this->meters.refresh(i, this->proc.plug.getAndResetPeakLevel(i));
  }
}

void NtCompressorAudioProcessorEditor::sliderValueChanged(
    juce::Slider* p_slider) {
  auto name   = p_slider->getName().toStdString();
  auto* p_val = this->proc.plug.getFloatValuePtr(name);
  if (!p_val) {
    jassert("Knob name in UI not found in plugin knobs.");
    return;
  }
  *p_val = p_slider->getValue();
  this->proc.plug.updateCoeffs();
}

void NtCompressorAudioProcessorEditor::buttonClicked(juce::Button* p_button) {
  auto name   = p_button->getName().toStdString();
  auto* p_val = this->proc.plug.getBoolValuePtr(name);
  if (!p_val) {
    jassert("Toggle name in UI not found in plugin toggles.");
    return;
  }
  *p_val = p_button->getToggleState();
  this->proc.plug.updateCoeffs();
}

void NtCompressorAudioProcessorEditor::comboBoxChanged(juce::ComboBox* p_box) {
  if (!this->isInitialized) { return; }
  if (p_box == this->allDropDowns[0].get()) { this->updateUiScale(); }
  if (p_box == this->allDropDowns[1].get()) { this->updateOversampling(); }
}

void NtCompressorAudioProcessorEditor::updateUiScale() {
  auto p_box = this->allDropDowns[0].get();
  switch (p_box->getSelectedId()) {
  case 1:
    this->uiScale = 0.5;
    break;
  case 2:
    this->uiScale = 0.75;
    break;
  case 3:
    this->uiScale = 1.0;
    break;
  case 4:
    this->uiScale = 1.25;
    break;
  case 5:
    this->uiScale = 1.5;
    break;
  case 6:
    this->uiScale = 1.75;
    break;
  case 7:
    this->uiScale = 2.0;
    break;
  default:
    return;
  }
  this->setSize(this->proc.plug.guiSpec.defaultWindowWidth * this->uiScale,
      this->unscaledWindowHeight * this->uiScale);
}

void NtCompressorAudioProcessorEditor::updateOversampling() {
  auto p_box = this->allDropDowns[1].get();
  this->proc.updateOversampling(p_box->getSelectedId());
}

void NtCompressorAudioProcessorEditor::calcSliderRowsCols(
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
