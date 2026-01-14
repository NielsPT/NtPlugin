/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include <algorithm>
#include <string>

#include "PluginEditor.h"
#include "PluginProcessor.h"

//==============================================================================
NtCompressorAudioProcessorEditor::NtCompressorAudioProcessorEditor(
    NtCompressorAudioProcessor& p)
    : AudioProcessorEditor(&p), proc(p) {
  this->getLookAndFeel().setColour(
      juce::ResizableWindow::ColourIds::backgroundColourId, juce::Colours::black);

  for (auto& k : this->proc.plug.primaryKnobs) { this->initPrimaryKnob(k); }
  for (auto& k : this->proc.plug.secondaryKnobs) { this->initSecondaryKnob(k); }
  for (auto& t : this->proc.plug.toggles) { this->initToggle(t); }
  for (auto& d : this->proc.titleBarSpec.dropDowns) { this->initDropDown(d); }

  // TODO: GuiSpec in plug. Contains size, maxrows, colours, etc.

  int nRows, nCols;
  this->calcSliderRowsCols(this->allPrimaryKnobs.size(),
      nRows,
      nCols,
      this->proc.plug.guiSpec.maxRows,
      this->proc.plug.guiSpec.maxColumns);
  auto height = this->titleBarAreaHeight;
  height += nRows * this->knobHeight;
  if (this->proc.plug.secondaryKnobs.size() != 0) {
    height += this->secondaryKnobHeight;
  }
  if (this->proc.plug.toggles.size() != 0) { height += this->toggleHeight; }
  this->unscaledWindowHeight = height;
  this->updateUiScale();
  this->addAndMakeVisible(this->meters);
  int meterRefreshRate_hz = 20;
  this->meters.setDecay(1, meterRefreshRate_hz);
  this->meters.setPeakHold(2, meterRefreshRate_hz);
  this->startTimerHz(meterRefreshRate_hz);
  this->isInitialized = true;
  this->drawGui();
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
  // std::replace(name.begin(), name.end(), '_', ' ');
  // std::transform(name.begin(), name.end(), name.begin(), ::toupper);
  p_label->setText(name, juce::NotificationType::dontSendNotification);
  this->addAndMakeVisible(*p_label);
  this->allDropDownAttachments.emplace_back(
      new juce::AudioProcessorValueTreeState::ComboBoxAttachment(
          this->proc.parameters, spec.name, *p_box));
  this->allDropDowns.push_back(std::move(p_box));
  this->allDropDownLables.push_back(std::move(p_label));
}

void NtCompressorAudioProcessorEditor::initPrimaryKnob(NtFx::KnobSpec<float>& spec) {
  auto p_knob  = std::make_unique<juce::Slider>(spec.name);
  auto p_label = std::make_unique<juce::Label>(spec.name);
  this->_initKnob(spec, p_knob, p_label);
  this->allPrimaryKnobs.push_back(std::move(p_knob));
  this->allPrimaryKnobLabels.push_back(std::move(p_label));
}

void NtCompressorAudioProcessorEditor::initSecondaryKnob(NtFx::KnobSpec<float>& spec) {
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
  // constexpr bool placeAbove = false;
  // p_label->attachToComponent(p_slider.get(), placeAbove);
  p_label->setJustificationType(juce::Justification::centred);
  std::string name(spec.name);
  std::replace(name.begin(), name.end(), '_', ' ');
  p_label->setText(name, juce::NotificationType::dontSendNotification);
  p_slider->setTextValueSuffix(spec.suffix);
  p_slider->setSliderStyle(juce::Slider::SliderStyle::Rotary);
  p_slider->addListener(this);
  addAndMakeVisible(p_slider.get());
  addAndMakeVisible(p_label.get());
  this->allKnobAttachments.emplace_back(
      new juce::AudioProcessorValueTreeState::SliderAttachment(
          this->proc.parameters, spec.name, *p_slider));
  p_slider->setRange(spec.minVal, spec.maxVal);
  if (spec.skew) { p_slider->setSkewFactorFromMidPoint(spec.skew); }
}

void NtCompressorAudioProcessorEditor::initToggle(NtFx::ToggleSpec& spec) {
  auto p_button = std::make_unique<juce::TextButton>(spec.name);
  addAndMakeVisible(p_button.get());
  p_button->setClickingTogglesState(true);
  p_button->setToggleable(true);
  p_button->addListener(this);
  p_button->setColour(
      juce::TextButton::ColourIds::textColourOffId, juce::Colours::grey);
  std::string name(spec.name);
  std::replace(name.begin(), name.end(), '_', ' ');
  p_button->setButtonText(name);
  this->allToggleAttachments.emplace_back(
      new juce::AudioProcessorValueTreeState::ButtonAttachment(
          this->proc.parameters, spec.name, *p_button));
  this->allToggles.push_back(std::move(p_button));
}

NtCompressorAudioProcessorEditor::~NtCompressorAudioProcessorEditor() {
  for (auto& toggle : this->allToggles) { toggle->setLookAndFeel(nullptr); }
  for (auto& slider : this->allPrimaryKnobs) { slider->setLookAndFeel(nullptr); }
  for (auto& slider : this->allSecondaryKnobs) { slider->setLookAndFeel(nullptr); }
}

void NtCompressorAudioProcessorEditor::paint(juce::Graphics& g) {
  g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
  g.setColour(juce::Colours::darkgrey);
  for (size_t i = 0; i < grayAreas.size(); i++) { g.fillRect(this->grayAreas[i]); }

  size_t pad = 10;
  g.setColour(juce::Colours::white);
  // g.drawLine(this->grayAreas[0].getBottomLeft().getX(),
  //     this->grayAreas[0].getBottomLeft().getY(),
  //     this->grayAreas[0].getBottomRight().getX(),
  //     this->grayAreas[0].getBottomRight().getY());
  for (auto area : this->borderedAreas) {
    g.drawRoundedRectangle(area.toFloat(), pad * this->uiScale, this->uiScale);
  }
}

void NtCompressorAudioProcessorEditor::resized() { this->drawGui(); }

// TODO: refactor this function. It's HUGE.
void NtCompressorAudioProcessorEditor::drawGui() {
  if (!this->isInitialized) { return; }
  // Reset
  this->grayAreas.clear();
  this->borderedAreas.clear();
  auto area = this->getLocalBounds();

  // Title Bar
  // TODO: loop over all dropdowns based on titleBarSpec.
  auto titleBarArea = area.removeFromTop(this->titleBarAreaHeight * this->uiScale);
  this->grayAreas.push_back(titleBarArea);
  int pad = 3 * this->uiScale;
  titleBarArea.reduce(pad, pad);
  for (auto& label : this->allDropDownLables) {
    label->setFont(this->defaultFontSize * this->uiScale * 0.6);
  }
  auto uiScaleDropDownLableArea = titleBarArea.removeFromLeft(60 * this->uiScale);
  this->allDropDownLables[0]->setBounds(uiScaleDropDownLableArea);
  auto uiScaleDropDownArea = titleBarArea.removeFromLeft(100 * this->uiScale);
  this->allDropDowns[0]->setBounds(uiScaleDropDownArea);
  auto oversamplingDropDownLableArea = titleBarArea.removeFromLeft(100 * this->uiScale);
  this->allDropDownLables[1]->setBounds(oversamplingDropDownLableArea);
  auto oversamplingDropDownArea = titleBarArea.removeFromLeft(100 * this->uiScale);
  this->allDropDowns[1]->setBounds(oversamplingDropDownArea);

  // Meters
  // TODO: constants?
  pad = 10 * this->uiScale;
  area.reduce(pad, pad);
  auto meterWidth = area.getWidth() / 15;
  auto meterArea  = area.removeFromLeft(meterWidth * (this->meters.size() + 1));
  this->meters.setFontSize(this->defaultFontSize * this->uiScale * 0.9);
  this->meters.setBounds(meterArea);
  this->borderedAreas.push_back(meterArea);

  // Toggels
  // TODO: All these numbers. Store them somewhere. Could be a sizes struct?
  auto togglesArea = area.removeFromBottom(this->toggleHeight * this->uiScale);
  this->toggleLookAndFeel.fontSize = this->defaultFontSize * this->uiScale;
  auto togglePad                   = 20 * this->uiScale;
  this->borderedAreas.push_back(togglesArea);
  togglesArea.removeFromTop(togglePad);
  togglesArea.removeFromBottom(togglePad);
  auto nToggles    = this->proc.plug.toggles.size();
  auto columnWidth = togglesArea.getWidth() / nToggles;
  for (size_t i = 0; i < nToggles; i++) {
    auto toggleArea = togglesArea.removeFromLeft(columnWidth);
    toggleArea.removeFromLeft(togglePad);
    toggleArea.removeFromRight(togglePad);
    this->allToggles[i]->setBounds(toggleArea);
    this->allToggles[i]->setLookAndFeel(&this->toggleLookAndFeel);
  }

  // All knobs
  this->knobLookAndFeel.fontSize = this->defaultFontSize * this->uiScale;

  // Secondary Knobs
  auto nSecondaryKnobs = this->proc.plug.secondaryKnobs.size();
  if (nSecondaryKnobs) {
    auto secondaryKnobsArea =
        area.removeFromBottom(this->secondaryKnobHeight * this->uiScale);
    this->borderedAreas.push_back(secondaryKnobsArea);
    secondaryKnobsArea.reduce(pad, pad);
    for (size_t i = 0; i < nSecondaryKnobs; i++) {
      auto knobArea =
          secondaryKnobsArea.removeFromLeft(this->secondaryKnobWidth * this->uiScale);
      auto labelArea = knobArea.removeFromTop(this->labelHeight * this->uiScale);
      this->allSecondaryKnobLabels[i]->setBounds(labelArea);
      this->allSecondaryKnobs[i]->setBounds(knobArea);
      this->allSecondaryKnobLabels[i]->setFont(this->defaultFontSize * this->uiScale);
      this->allSecondaryKnobs[i]->setTextBoxStyle(juce::Slider::TextBoxBelow,
          false,
          80 * this->uiScale,
          this->labelHeight * this->uiScale);
    }
  }

  // Primary knobs
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
  size_t iSlider = 0;
  columnWidth    = knobsArea.getWidth() / nColumns;
  auto rowHeight = knobsArea.getHeight() / nRows;
  for (size_t i = 0; i < nRows; i++) {
    auto rowArea = knobsArea.removeFromTop(rowHeight);
    rowArea.removeFromTop(pad);
    rowArea.removeFromBottom(pad);
    for (size_t j = 0; j < nColumns; j++) {
      if (iSlider >= nKnobs) { break; }
      auto knobArea  = rowArea.removeFromLeft(columnWidth);
      auto labelArea = knobArea.removeFromTop(this->labelHeight * this->uiScale);
      this->allPrimaryKnobLabels[iSlider]->setBounds(labelArea);
      this->allPrimaryKnobLabels[iSlider]->setFont(
          this->defaultFontSize * this->uiScale);
      this->allPrimaryKnobs[iSlider]->setBounds(knobArea);
      this->allPrimaryKnobs[iSlider]->setTextBoxStyle(juce::Slider::TextBoxBelow,
          false,
          80 * this->uiScale,
          this->labelHeight * this->uiScale);
      iSlider++;
    }
  }

  // Update UI
  this->repaint();
}

void NtCompressorAudioProcessorEditor::timerCallback() {
  for (size_t i = 0; i < this->meters.size(); i++) {
    this->meters.refresh(i, this->proc.plug.getAndResetPeakLevel(i));
  }
  int badVarId = this->proc.plug.getAndResetErrorVal();
  if (this->popupIsDisplayed) {
    if (badVarId == 0) { this->popupIsDisplayed = false; }
    return;
  }
  this->displayErrorValPopup(badVarId);
}

void NtCompressorAudioProcessorEditor::displayErrorValPopup(int varId) {
  if (varId == 0) { return; }
  this->popupIsDisplayed = true;
  std::string message    = "NaN in " + std::to_string(varId);
  DBG(message);
  if (!reportNotFiniteState) { return; }
  juce::NativeMessageBox::showMessageBoxAsync(
      juce::MessageBoxIconType::WarningIcon, "Nonefinite Value", message);
}

void NtCompressorAudioProcessorEditor::sliderValueChanged(juce::Slider* p_slider) {
  auto name   = p_slider->getName().toStdString();
  auto* p_val = this->proc.plug.getFloatParamByName(name);
  if (!p_val) {
    juce::NativeMessageBox::showMessageBoxAsync(
        juce::MessageBoxIconType::WarningIcon, "Bad Name", "Float " + name);
    return;
  }
  *p_val = p_slider->getValue();
  this->proc.plug.updateCoeffs();
}

void NtCompressorAudioProcessorEditor::buttonClicked(juce::Button* p_button) {
  auto name   = p_button->getName().toStdString();
  auto* p_val = this->proc.plug.getBoolParamByName(name);
  if (!p_val) {
    juce::NativeMessageBox::showMessageBoxAsync(
        juce::MessageBoxIconType::WarningIcon, "Bad Name", "Bool " + name);
    return;
  }
  *p_val = p_button->getToggleState();
  this->proc.plug.updateCoeffs();
}

void NtCompressorAudioProcessorEditor::comboBoxChanged(juce::ComboBox* p_box) {
  if (!this->isInitialized) { return; }
  if (p_box == this->allDropDowns[0].get()) { this->updateUiScale(); }
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
    this->uiScale = 2.0;
    break;
  default:
    return;
  }
  this->setSize(this->proc.plug.guiSpec.defaultWindowWidth * this->uiScale,
      this->unscaledWindowHeight * this->uiScale);
}

void NtCompressorAudioProcessorEditor::calcSliderRowsCols(
    int nKnobs, int& nRows, int& nColumns, int maxRows, int maxColumns) {
  if (nKnobs > maxRows * maxColumns) {
    juce::NativeMessageBox::showMessageBoxAsync(juce::MessageBoxIconType::WarningIcon,
        "Bad Grid Layout",
        "Too many parameters. Max is 24.");
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
