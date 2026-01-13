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
    : AudioProcessorEditor(&p), audioProcessor(p), buttonLookAndFeel(defaultFontSize) {
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

  // TODO: GuiSpec in plug. Contains size, maxrows, etc.

  int nRows, nCols;
  this->calcSliderRowsCols(this->allSliders.size(), nRows, nCols);
  auto height = this->titleBarAreaHeight;
  height += nRows * this->knobHeight;
  if (this->audioProcessor.plug.floatParametersSmall.size() != 0) {
    height += this->smallKnobHeight;
  }
  if (this->audioProcessor.plug.boolParameters.size() != 0) {
    height += this->toggleHeight;
  }
  this->unscaledWindowHeight = height;
  this->setSize(this->defaultWindowWidth * this->uiScale, height * this->uiScale);
  this->addAndMakeVisible(this->meters);
  int meterRefreshRate_hz = 20;
  this->meters.setDecay(1, meterRefreshRate_hz);
  this->meters.setPeakHold(2, meterRefreshRate_hz);

  std::vector<std::string> scaleOptions = {
    "50%", "75%", "100%", "125%", "150%", "200%"
  };
  this->initDropDownBox(scaleOptions, "UI scale");

  this->startTimerHz(meterRefreshRate_hz);
  this->isInitialized = true;
  this->drawGui();
}
void NtCompressorAudioProcessorEditor::initDropDownBox(
    std::vector<std::string>& vars, std::string title) {
  auto p_box = new juce::ComboBox;
  this->allDropDownBoxes.push_back(p_box);
  p_box->setTitle(title);
  for (size_t i = 0; i < vars.size(); i++) {
    p_box->addItem(vars[i], i + 1);
  }
  p_box->setSelectedItemIndex(2, juce::NotificationType::dontSendNotification);
  p_box->setColour(
      juce::ComboBox::ColourIds::backgroundColourId, juce::Colours::darkgrey);
  this->addAndMakeVisible(*p_box);
  p_box->addListener(this);
}

void NtCompressorAudioProcessorEditor::initSlider(
    NtFx::FloatParameterSpec<float>* p_spec,
    juce::Slider* p_slider,
    juce::Label* p_label) {
  p_slider->setLookAndFeel(&this->knobLookAndFeel);
  p_slider->setTextBoxStyle(juce::Slider::TextBoxBelow,
      false,
      // p_slider->getWidth(),
      80 * this->uiScale,
      this->labelHeight * this->uiScale);
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
  p_button->setColour(
      juce::TextButton::ColourIds::textColourOffId, juce::Colours::grey);
  std::string name(p_spec->name);
  std::replace(name.begin(), name.end(), '_', ' ');
  p_button->setButtonText(name);
  this->allToggleAttachments.emplace_back(
      new juce::AudioProcessorValueTreeState::ButtonAttachment(
          this->audioProcessor.parameters, p_spec->name, *p_button));
}

NtCompressorAudioProcessorEditor::~NtCompressorAudioProcessorEditor() {
  for (auto toggle : this->allToggles) {
    toggle->setLookAndFeel(nullptr);
  }
  for (auto slider : this->allSliders) {
    slider->setLookAndFeel(nullptr);
  }
  for (auto slider : this->allSmallSliders) {
    slider->setLookAndFeel(nullptr);
  }
}

void NtCompressorAudioProcessorEditor::paint(juce::Graphics& g) {
  g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
  g.setColour(juce::Colours::darkgrey);
  for (size_t i = 0; i < grayAreas.size(); i++) {
    g.fillRect(this->grayAreas[i]);
  }

  // g.fillRect(this->titleBarArea);
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

void NtCompressorAudioProcessorEditor::drawGui() {
  if (!this->isInitialized) { return; }
  auto area         = this->getLocalBounds();
  auto titleBarArea = area.removeFromTop(this->titleBarAreaHeight * this->uiScale);
  this->grayAreas.clear();
  this->grayAreas.push_back(titleBarArea);
  auto uiScaleDropDownArea = titleBarArea.removeFromLeft(100 * this->uiScale);
  int uiScaleDropDownPad   = 5 * this->uiScale;
  this->grayAreas.push_back(uiScaleDropDownArea.removeFromTop(uiScaleDropDownPad));
  this->grayAreas.push_back(uiScaleDropDownArea.removeFromLeft(uiScaleDropDownPad));
  this->grayAreas.push_back(uiScaleDropDownArea.removeFromBottom(uiScaleDropDownPad));
  this->grayAreas.push_back(uiScaleDropDownArea.removeFromRight(uiScaleDropDownPad));
  this->allDropDownBoxes[0]->setBounds(uiScaleDropDownArea);
  int pad = 10 * this->uiScale;
  area.removeFromTop(pad);
  area.removeFromLeft(pad);
  area.removeFromBottom(pad);
  area.removeFromRight(pad);

  auto meterWidth = area.getWidth() / 15;
  auto meterArea  = area.removeFromLeft(meterWidth * (this->meters.size() + 0.5));
  this->meters.setFontSize((this->defaultFontSize - 2) * this->uiScale);
  this->meters.setBounds(meterArea);
  this->borderedAreas.clear();
  this->borderedAreas.push_back(meterArea);
  auto nSliders = this->audioProcessor.plug.floatParameters.size();
  int nColumns;
  int nRows;
  this->calcSliderRowsCols(nSliders, nRows, nColumns);
  auto totalHeight   = area.getHeight();
  auto togglesArea   = area.removeFromBottom(this->toggleHeight * this->uiScale);
  auto nSmallSliders = this->audioProcessor.plug.floatParametersSmall.size();
  if (nSmallSliders) {
    auto smallSlidersArea =
        area.removeFromBottom(this->smallKnobHeight * this->uiScale);
    this->borderedAreas.push_back(smallSlidersArea);
    smallSlidersArea.removeFromLeft(pad);
    smallSlidersArea.removeFromRight(pad);
    smallSlidersArea.removeFromTop(pad);
    smallSlidersArea.removeFromBottom(pad);

    for (size_t i = 0; i < nSmallSliders; i++) {
      auto smallSliderArea =
          smallSlidersArea.removeFromLeft(this->smallKnobWidth * this->uiScale);
      auto labelArea = smallSliderArea.removeFromTop(this->labelHeight * this->uiScale);
      this->allSmallSliders[i]->setBounds(smallSliderArea);
      this->allSmallSliderLabels[i]->setFont(this->defaultFontSize * this->uiScale);
    }
  }

  auto knobsArea = area;
  this->borderedAreas.push_back(knobsArea);
  size_t iSlider     = 0;
  size_t columnWidth = knobsArea.getWidth() / nColumns;
  size_t rowHeight   = knobsArea.getHeight() / nRows;
  for (size_t i = 0; i < nRows; i++) {
    auto rowArea = knobsArea.removeFromTop(rowHeight);
    rowArea.removeFromTop(pad);
    rowArea.removeFromBottom(pad);
    for (size_t j = 0; j < nColumns; j++) {
      if (iSlider >= nSliders) { break; }
      auto sliderArea = rowArea.removeFromLeft(columnWidth);
      auto labelArea  = sliderArea.removeFromTop(this->labelHeight * this->uiScale);
      this->allSliders[iSlider]->setBounds(sliderArea);
      this->allSliderLabels[iSlider]->setFont(this->defaultFontSize * this->uiScale);
      iSlider++;
    }
  }

  auto togglePad = 20 * this->uiScale;
  this->borderedAreas.push_back(togglesArea);
  togglesArea.removeFromTop(togglePad);
  togglesArea.removeFromBottom(togglePad);
  auto nToggles = this->audioProcessor.plug.boolParameters.size();
  columnWidth   = togglesArea.getWidth() / nToggles;
  // ButtonLookAndFeel buttonLookAndFeel(this->defaultFontSize * this->uiScale);
  this->buttonLookAndFeel.fontSize = this->defaultFontSize * this->uiScale;
  for (size_t i = 0; i < nToggles; i++) {
    auto toggleArea = togglesArea.removeFromLeft(columnWidth);
    toggleArea.removeFromLeft(togglePad);
    toggleArea.removeFromRight(togglePad);
    this->allToggles[i]->setBounds(toggleArea);
    this->allToggles[i]->setLookAndFeel(&this->buttonLookAndFeel);
  }
  this->repaint();
}

void NtCompressorAudioProcessorEditor::timerCallback() {
  for (size_t i = 0; i < this->meters.size(); i++) {
    this->meters.refresh(i, this->audioProcessor.plug.getAndResetPeakLevel(i));
  }
  int badVarId = this->audioProcessor.plug.getAndResetErrorVal();
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
  if (!NtFx::reportNotFiniteState) { return; }
  juce::NativeMessageBox::showMessageBoxAsync(
      juce::MessageBoxIconType::WarningIcon, "Nonefinite Value", message);
}

void NtCompressorAudioProcessorEditor::sliderValueChanged(juce::Slider* p_slider) {
  auto name   = p_slider->getName().toStdString();
  auto* p_val = this->audioProcessor.plug.getFloatParamByName(name);
  if (!p_val) {
    juce::NativeMessageBox::showMessageBoxAsync(
        juce::MessageBoxIconType::WarningIcon, "Bad Name", "Float " + name);
    return;
  }
  *p_val = p_slider->getValue();
  this->audioProcessor.plug.updateCoeffs();
}

void NtCompressorAudioProcessorEditor::buttonClicked(juce::Button* p_button) {
  auto name   = p_button->getName().toStdString();
  auto* p_val = this->audioProcessor.plug.getBoolParamByName(name);
  if (!p_val) {
    juce::NativeMessageBox::showMessageBoxAsync(
        juce::MessageBoxIconType::WarningIcon, "Bad Name", "Bool " + name);
    return;
  }
  *p_val = p_button->getToggleState();
  this->audioProcessor.plug.updateCoeffs();
}

void NtCompressorAudioProcessorEditor::comboBoxChanged(juce::ComboBox* p_box) {
  if (p_box == this->allDropDownBoxes[0]) {
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
    this->setSize(this->defaultWindowWidth * this->uiScale,
        this->unscaledWindowHeight * this->uiScale);
  }
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
