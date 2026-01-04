/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginEditor.h"

#include "PluginProcessor.h"
#include <string>

//==============================================================================
NtCompressorAudioProcessorEditor::NtCompressorAudioProcessorEditor(
    NtCompressorAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p), meterScale(meters[0]) {

  getLookAndFeel().setColour(
      juce::TextButton::ColourIds::buttonColourId, juce::Colours::black);
  getLookAndFeel().setColour(
      juce::TextButton::ColourIds::buttonOnColourId, juce::Colours::grey);
  getLookAndFeel().setColour(
      juce::ResizableWindow::ColourIds::backgroundColourId, juce::Colours::black);

  int meterRefreshRate_hz = 20;
  // TODO: GuiSpec in plug. Contains size, maxrows, etc.
  setSize(1000, 700);

  startTimerHz(meterRefreshRate_hz);
  // TODO: stereo meters.

  auto meterDist = 20;

  this->meters[NtFx::MeterIdx::xL].padLeft  = meterDist;
  this->meters[NtFx::MeterIdx::grL].padLeft = meterDist;
  this->meters[NtFx::MeterIdx::yL].padLeft  = meterDist;
  this->meters[NtFx::MeterIdx::yR].padRight = meterDist;
  this->meters[NtFx::MeterIdx::xL].setInvert(false);
  this->meters[NtFx::MeterIdx::xR].setInvert(false);
  this->meters[NtFx::MeterIdx::yL].setInvert(false);
  this->meters[NtFx::MeterIdx::yR].setInvert(false);

  this->meters[NtFx::MeterIdx::xL].label  = "L";
  this->meters[NtFx::MeterIdx::xR].label  = "R";
  this->meters[NtFx::MeterIdx::grL].label = "L";
  this->meters[NtFx::MeterIdx::grR].label = "R";
  this->meters[NtFx::MeterIdx::yL].label  = "L";
  this->meters[NtFx::MeterIdx::yR].label  = "R";
  this->meters[NtFx::MeterIdx::grL].setInvert(true);
  this->meters[NtFx::MeterIdx::grR].setInvert(true);
  // this->meters[NtFx::MeterIdx::grL].nDots  = 10;
  // this->meters[NtFx::MeterIdx::grR].nDots  = 10;
  // this->meters[NtFx::MeterIdx::grL].minVal_db = -30;
  // this->meters[NtFx::MeterIdx::grR].minVal_db = -30;
  for (size_t i = 0; i < this->meters.size(); i++) {
    this->addAndMakeVisible(this->meters[i]);
    this->meters[i].setDecay(1, meterRefreshRate_hz);
    this->meters[i].setPeakHold(2, meterRefreshRate_hz);
  }

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

  for (size_t i = 0; i < this->meterLabelStrings.size(); i++) {
    this->allMeterLabels.emplace_back(new juce::Label(this->meterLabelStrings[i]));
  }

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

  for (size_t i = 0; i < this->allMeterLabels.size(); i++) {
    this->addAndMakeVisible(*this->allMeterLabels[i]);
    this->allMeterLabels[i]->setText(
        this->meterLabelStrings[i], juce::NotificationType::dontSendNotification);
  }
  this->addAndMakeVisible(this->meterScale);
  // this->addAndMakeVisible(this->frameCounterLabel);
  // this->ratioSlider.setSkewFactorFromMidPoint(2.0);
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
  auto meterWidth = this->meters[0].getWidth();
  auto meterArea  = area.removeFromLeft((this->meters.size() + 1) * meterWidth);
  this->borderedAreas.push_back(meterArea);
  auto labelArea = meterArea.removeFromTop(this->entryHeight * 2);
  labelArea.removeFromTop(this->entryHeight);
  for (size_t i = 0; i < this->allMeterLabels.size(); i++) {
    auto oneLabelArea = labelArea.removeFromLeft(2 * meterWidth);
    this->allMeterLabels[i]->setBounds(oneLabelArea);
    this->allMeterLabels[i]->setJustificationType(juce::Justification::centredBottom);
  }
  for (size_t i = 0; i < this->meters.size(); i++) {
    auto oneMeterArea = meterArea.removeFromLeft(meterWidth);
    this->borderedAreas.push_back(oneMeterArea);
    this->meters[i].setBounds(oneMeterArea);
  }
  this->meterScale.setBounds(meterArea);
  auto nSliders = this->audioProcessor.plug.floatParameters.size();
  int nColumns;
  int nRows;
  // if (nSliders < 7) {
  //   nRows    = 1;
  //   nColumns = nSliders;
  // } else if (nSliders < 9) {
  //   nRows    = 2;
  //   nColumns = 4;
  // } else if (nSliders < 11) {
  //   nRows    = 2;
  //   nColumns = 5;
  // } else if (nSliders < 13) {
  //   nRows    = 3;
  //   nColumns = 4;
  // } else if (nSliders < 16) {
  //   nRows    = 3;
  //   nColumns = 5;
  // } else if (nSliders < 19) {
  //   nRows    = 3;
  //   nColumns = 6;
  // } else if (nSliders < 21) {
  //   nRows    = 4;
  //   nColumns = 5;
  // } else if (nSliders < 25) {
  //   nRows    = 4;
  //   nColumns = 6;
  // } else {
  //   juce::NativeMessageBox::showMessageBoxAsync(juce::MessageBoxIconType::WarningIcon,
  //       "Bad Grid Layout",
  //       "Too many parameters. Max is 24.");
  //   return;
  // }
  this->calcSliderRowsCols(nSliders, nRows, nColumns);
  auto totalHeight      = area.getHeight();
  auto togglesArea      = area.removeFromBottom(totalHeight / 8);
  auto smallSlidersArea = area.removeFromBottom(totalHeight / 4);
  auto knobsArea        = area;
  size_t iSlider        = 0;
  size_t columnWidth    = knobsArea.getWidth() / nColumns;
  size_t rowHeight      = knobsArea.getHeight() / nRows;
  for (size_t i = 0; i < nRows; i++) {
    auto rowArea = knobsArea.removeFromTop(rowHeight);
    rowArea.removeFromTop(pad);
    rowArea.removeFromBottom(pad);
    for (size_t j = 0; j < nColumns; j++) {
      if (iSlider >= nSliders) { break; }
      auto sliderArea = rowArea.removeFromLeft(columnWidth);
      auto labelArea  = sliderArea.removeFromTop(this->entryHeight);
      this->allSliders[iSlider]->setBounds(sliderArea);
      iSlider++;
    }
  }
  auto nSmallSliders           = this->audioProcessor.plug.floatParametersSmall.size();
  size_t rowHeightSmallSliders = rowHeight;
  smallSlidersArea.removeFromLeft(pad);
  smallSlidersArea.removeFromRight(pad);
  this->borderedAreas.push_back(smallSlidersArea);

  for (size_t i = 0; i < nSmallSliders; i++) {
    auto smallSliderArea = smallSlidersArea.removeFromLeft(100);
    auto labelArea       = smallSliderArea.removeFromTop(this->entryHeight);
    this->allSmallSliders[i]->setBounds(smallSliderArea);
  }

  auto togglePad = 20;
  togglesArea.removeFromTop(togglePad);
  togglesArea.removeFromBottom(togglePad);
  auto nToggles = this->audioProcessor.plug.boolParameters.size();
  for (size_t i = 0; i < nToggles; i++) {
    auto toggleArea = togglesArea.removeFromLeft(columnWidth);
    toggleArea.removeFromLeft(togglePad);
    toggleArea.removeFromRight(togglePad);
    this->allToggles[i]->setBounds(toggleArea);
  }
  repaint();
}

void NtCompressorAudioProcessorEditor::timerCallback() {
  for (size_t i = 0; i < this->meters.size(); i++) {
    this->meters[i].refresh(this->audioProcessor.plug.getAndResetPeakLevel(i));
  }
  // this->frameCounterLabel.setText("Frames: " + std::to_string(this->frameCounter++),
  //     juce::NotificationType::sendNotification);

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
  const int maxRows    = 4;
  const int maxColumns = 6;
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
