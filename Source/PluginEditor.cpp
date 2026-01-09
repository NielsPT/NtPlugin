/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include <string>

#include "PluginEditor.h"
#include "PluginProcessor.h"

//==============================================================================
NtCompressorAudioProcessorEditor::NtCompressorAudioProcessorEditor(
    NtCompressorAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p) {
  // : AudioProcessorEditor(&p), audioProcessor(p), meterScale(meters[0]) {
  // : AudioProcessorEditor(&p), audioProcessor(p), meterScale(meters[0].l),
  //   meters { std::string("IN"), std::string("OUT"), std::string("GR") } {

  this->getLookAndFeel().setColour(
      juce::TextButton::ColourIds::buttonColourId, juce::Colours::black);
  this->getLookAndFeel().setColour(
      juce::TextButton::ColourIds::buttonOnColourId, juce::Colours::grey);
  this->getLookAndFeel().setColour(
      juce::ResizableWindow::ColourIds::backgroundColourId, juce::Colours::black);

  for (size_t i = 0; i < this->audioProcessor.plug.floatParameters.size(); i++) {
    std::string name = this->audioProcessor.plug.floatParameters[i].name;
    this->allSliders.emplace_back(new juce::Slider(name));
    this->allSliderLabels.emplace_back(new juce::Label(name));
  }

  for (size_t i = 0; i < this->audioProcessor.plug.floatParametersSmall.size(); i++) {
    std::string name = this->audioProcessor.plug.floatParametersSmall[i].name;
    this->allSmallSliders.emplace_back(new juce::Slider(name));
    this->allSmallSliderLabels.emplace_back(new juce::Label(name));
  }

  for (size_t i = 0; i < this->audioProcessor.plug.boolParameters.size(); i++) {
    std::string name = this->audioProcessor.plug.boolParameters[i].name;
    this->allToggles.emplace_back(new juce::TextButton(name));
  }

  // for (size_t i = 0; i < this->meterLabelStrings.size(); i++) {
  //   this->allMeterLabels.emplace_back(new juce::Label(this->meterLabelStrings[i]));
  // }

  this->allSliderAttachments.reserve(this->allSliders.size());
  for (size_t i = 0; i < this->allSliders.size(); i++) {
    this->initSlider(&this->audioProcessor.plug.floatParameters[i],
        this->allSliders[i],
        this->allSliderLabels[i]);
  }

  for (size_t i = 0; i < this->allSmallSliders.size(); i++) {
    this->initSlider(&this->audioProcessor.plug.floatParametersSmall[i],
        this->allSmallSliders[i],
        this->allSmallSliderLabels[i]);
  }

  this->allToggleAttachments.reserve(this->allToggles.size());
  for (size_t i = 0; i < this->allToggles.size(); i++) {
    this->initToggle(&this->audioProcessor.plug.boolParameters[i], this->allToggles[i]);
  }

  // for (size_t i = 0; i < this->allMeterLabels.size(); i++) {
  //   this->addAndMakeVisible(*this->allMeterLabels[i]);
  //   this->allMeterLabels[i]->setText(
  //       this->meterLabelStrings[i], juce::NotificationType::dontSendNotification);
  // }

  // TODO: GuiSpec in plug. Contains size, maxrows, etc.

  int nRows, nCols;
  this->calcSliderRowsCols(this->allSliders.size(), nRows, nCols);
  auto height = 0;
  height += nRows * 200;
  if (this->audioProcessor.plug.floatParametersSmall.size() != 0) { height += 150; }
  if (this->audioProcessor.plug.boolParameters.size() != 0) { height += 75; }
  this->setSize(1000, height);

  // for (size_t i = 0; i < this->meters.size(); i++) {
  //   this->addAndMakeVisible(this->meters[i]);
  //   this->meters[i].setDecay(1, meterRefreshRate_hz);
  //   this->meters[i].setPeakHold(2, meterRefreshRate_hz);
  // }
  // this->meters[2].setInvert(true);
  this->addAndMakeVisible(this->meters);
  int meterRefreshRate_hz = 20;
  this->meters.setDecay(1, meterRefreshRate_hz);
  this->meters.setPeakHold(2, meterRefreshRate_hz);
  this->startTimerHz(meterRefreshRate_hz);
  this->isInitialized = true;
  this->drawGui();
}

void NtCompressorAudioProcessorEditor::initSlider(
    NtFx::FloatParameterSpec<float>* p_spec,
    juce::Slider* p_slider,
    juce::Label* p_label) {
  p_slider->setLookAndFeel(&this->knobLookAndFeel);
  p_slider->setTextBoxStyle(juce::Slider::TextBoxBelow,
      false,
      this->sliderWidth * 2 - 2 * this->entryPad,
      this->entryHeight);
  constexpr bool placeAbove = false;
  p_label->attachToComponent(p_slider, placeAbove);
  p_label->setJustificationType(juce::Justification::centred);
  std::string name(p_spec->name);
  std::replace(name.begin(), name.end(), '_', ' ');
  p_label->setText(name, juce::NotificationType::dontSendNotification);
  p_slider->setTextValueSuffix(p_spec->suffix);
  p_slider->setSliderStyle(juce::Slider::SliderStyle::Rotary);
  p_slider->addListener(this);
  addAndMakeVisible(p_slider);
  addAndMakeVisible(p_label);
  this->allSliderAttachments.emplace_back(
      new juce::AudioProcessorValueTreeState::SliderAttachment(
          this->audioProcessor.parameters, p_spec->name, *p_slider));
  p_slider->setRange(p_spec->minVal, p_spec->maxVal);
  if (p_spec->skew) { p_slider->setSkewFactorFromMidPoint(p_spec->skew); }
}

void NtCompressorAudioProcessorEditor::initToggle(
    NtFx::BoolParameterSpec* p_spec, juce::TextButton* p_button) {
  addAndMakeVisible(p_button);
  p_button->setClickingTogglesState(true);
  p_button->setToggleable(true);
  p_button->addListener(this);
  std::string name(p_spec->name);
  std::replace(name.begin(), name.end(), '_', ' ');
  p_button->setButtonText(name);
  this->allToggleAttachments.emplace_back(
      new juce::AudioProcessorValueTreeState::ButtonAttachment(
          this->audioProcessor.parameters, p_spec->name, *p_button));
}

NtCompressorAudioProcessorEditor::~NtCompressorAudioProcessorEditor() { }

void NtCompressorAudioProcessorEditor::paint(juce::Graphics& g) {
  g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
  size_t pad = 10;
  g.setColour(juce::Colours::white);
  for (auto area : this->borderedAreas) {
    g.drawRoundedRectangle(area.toFloat(), pad, 1.0);
  }
}

void NtCompressorAudioProcessorEditor::resized() { this->drawGui(); }

void NtCompressorAudioProcessorEditor::drawGui() {
  if (!this->isInitialized) { return; }
  auto area  = this->getLocalBounds();
  size_t pad = 10;
  area.removeFromTop(pad);
  area.removeFromLeft(pad);
  area.removeFromBottom(pad);
  area.removeFromRight(pad);
  // auto meterWidth = this->meters[0].getWidth();
  auto meterWidth = area.getWidth() / 15;
  auto meterArea  = area.removeFromLeft(meterWidth * (this->meters.size() + 0.5));
  this->meters.setBounds(meterArea);
  this->borderedAreas.push_back(meterArea);
  // for (size_t i = 0; i < this->meters.size(); i++) {
  //   this->meters[i].setBounds(meterArea.removeFromLeft(meterWidth));
  // }

  // this->meterScale.setBounds(meterArea);

  auto nSliders = this->audioProcessor.plug.floatParameters.size();
  int nColumns;
  int nRows;
  this->calcSliderRowsCols(nSliders, nRows, nColumns);
  auto totalHeight = area.getHeight();
  // TODO: ToggleHeight
  auto togglesArea   = area.removeFromBottom(75);
  auto nSmallSliders = this->audioProcessor.plug.floatParametersSmall.size();
  if (nSmallSliders) {
    auto smallSlidersArea = area.removeFromBottom(150);
    this->borderedAreas.push_back(smallSlidersArea);
    smallSlidersArea.removeFromLeft(pad);
    smallSlidersArea.removeFromRight(pad);
    smallSlidersArea.removeFromTop(pad);
    smallSlidersArea.removeFromBottom(pad);

    for (size_t i = 0; i < nSmallSliders; i++) {
      auto smallSliderArea = smallSlidersArea.removeFromLeft(100);
      auto labelArea       = smallSliderArea.removeFromTop(this->entryHeight);
      this->allSmallSliders[i]->setBounds(smallSliderArea);
    }
  }

  auto knobsArea = area;
  this->borderedAreas.push_back(knobsArea);
  size_t iSlider     = 0;
  size_t columnWidth = knobsArea.getWidth() / nColumns;
  size_t rowHeight   = knobsArea.getHeight() / nRows;
  for (size_t i = 0; i < nRows; i++) {
    auto rowArea = knobsArea.removeFromTop(rowHeight);
    // this->borderedAreas.push_back(rowArea);
    rowArea.removeFromTop(pad);
    rowArea.removeFromBottom(pad);
    for (size_t j = 0; j < nColumns; j++) {
      if (iSlider >= nSliders) { break; }
      auto sliderArea = rowArea.removeFromLeft(columnWidth);
      // this->borderedAreas.push_back(sliderArea);
      auto labelArea = sliderArea.removeFromTop(this->entryHeight);
      this->allSliders[iSlider]->setBounds(sliderArea);
      iSlider++;
    }
  }

  auto togglePad = 20;
  this->borderedAreas.push_back(togglesArea);
  togglesArea.removeFromTop(togglePad);
  togglesArea.removeFromBottom(togglePad);
  auto nToggles = this->audioProcessor.plug.boolParameters.size();
  columnWidth   = togglesArea.getWidth() / nToggles;
  for (size_t i = 0; i < nToggles; i++) {
    auto toggleArea = togglesArea.removeFromLeft(columnWidth);
    toggleArea.removeFromLeft(togglePad);
    toggleArea.removeFromRight(togglePad);
    this->allToggles[i]->setBounds(toggleArea);
  }
  this->repaint();
}

void NtCompressorAudioProcessorEditor::timerCallback() {
  for (size_t i = 0; i < this->meters.size(); i++) {
    this->meters.refresh(i, this->audioProcessor.plug.getAndResetPeakLevel(i));
  }

  if (!NtFx::reportNotFiniteState) { return; }
  if (this->popupIsDisplayed) {
    if (this->audioProcessor.plug.getAndResetErrorVal() == NtFx::ErrorVal::e_none) {
      this->popupIsDisplayed = false;
    }
    return;
  }
  switch (this->audioProcessor.plug.getAndResetErrorVal()) {
  case NtFx::ErrorVal::e_rmsSensor:
    this->displayErrorValPopup("rmsSensor");
    break;
  case NtFx::ErrorVal::e_x:
    this->displayErrorValPopup("x");
    break;
  case NtFx::ErrorVal::e_fbState:
    this->displayErrorValPopup("fbState");
    break;
  case NtFx::ErrorVal::e_x_sc:
    this->displayErrorValPopup("x_sc");
    break;
  case NtFx::ErrorVal::e_yFilterLast:
    this->displayErrorValPopup("yFilterLast");
    break;
  case NtFx::ErrorVal::e_ySensLast:
    this->displayErrorValPopup("ySensLast");
    break;
  case NtFx::ErrorVal::e_nRms:
    this->displayErrorValPopup("nRms");
    break;
  case NtFx::ErrorVal::e_iRms:
    this->displayErrorValPopup("iRms");
    break;
  case NtFx::ErrorVal::e_rmsAccum:
    this->displayErrorValPopup("rmsAccum");
    break;
  case NtFx::ErrorVal::e_gr:
    this->displayErrorValPopup("gr");
    break;
  case NtFx::ErrorVal::e_meter:
    this->displayErrorValPopup("meter");
    break;
  case NtFx::ErrorVal::e_none:
    this->popupIsDisplayed = false;
    break;
  default:
    juce::NativeMessageBox::showMessageBoxAsync(juce::MessageBoxIconType::WarningIcon,
        "Nonefinite Value",
        "Unknown: " + std::to_string(this->audioProcessor.plug.getAndResetErrorVal()));
    break;
  }
}

void NtCompressorAudioProcessorEditor::displayErrorValPopup(std::string message) {
  juce::NativeMessageBox::showMessageBoxAsync(
      juce::MessageBoxIconType::WarningIcon, "Nonefinite Value", message);
  this->popupIsDisplayed = true;
}

void NtCompressorAudioProcessorEditor::sliderValueChanged(juce::Slider* p_slider) {
  auto name   = p_slider->getName().toStdString();
  auto* p_val = this->audioProcessor.plug.getFloatValByName(name);
  if (!p_val) {
    juce::NativeMessageBox::showMessageBoxAsync(
        juce::MessageBoxIconType::WarningIcon, "Bad Name", "Float " + name);
    return;
  }
  *p_val = p_slider->getValue();
  this->audioProcessor.plug.update();
}

void NtCompressorAudioProcessorEditor::buttonClicked(juce::Button* p_button) {
  auto name   = p_button->getName().toStdString();
  auto* p_val = this->audioProcessor.plug.getBoolValByName(name);
  if (!p_val) {
    juce::NativeMessageBox::showMessageBoxAsync(
        juce::MessageBoxIconType::WarningIcon, "Bad Name", "Bool " + name);
    return;
  }
  *p_val = p_button->getToggleState();
  this->audioProcessor.plug.update();
}

void NtCompressorAudioProcessorEditor::calcSliderRowsCols(
    int nSliders, int& nRows, int& nColumns) {
  int maxRows    = 3;
  int maxColumns = 6;
  // if (nSliders > 12) { maxColumns = 6; }
  if (nSliders > 18) { maxColumns = 8; }
  if (nSliders > maxRows * maxColumns) {
    juce::NativeMessageBox::showMessageBoxAsync(juce::MessageBoxIconType::WarningIcon,
        "Bad Grid Layout",
        "Too many parameters. Max is 24.");
    return;
  }

  int bestRows    = 1;
  int bestColumns = nSliders;
  int minCells    = std::numeric_limits<int>::max();

  for (int r = 1; r <= maxRows; ++r) {
    int c = (nSliders + r - 1) / r;
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
