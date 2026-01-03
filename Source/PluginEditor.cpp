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
  // getLookAndFeel().setColour(
  //     juce::Slider::ColourIds::thumbColourId, juce::Colours::white);
  getLookAndFeel().setColour(
      juce::ResizableWindow::ColourIds::backgroundColourId, juce::Colours::black);

  int meterRefreshRate_hz = 20;
  setSize(1000, 500);

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

  // addAndMakeVisible(this->gainReductionMeter);
  this->ratioSlider.setSkewFactorFromMidPoint(2.0);
  this->allSliders = {
    &this->threshSlider,
    &this->ratioSlider,
    &this->kneeSlider,
    &this->tAttSlider,
    &this->tRelSlider,
    &this->tRmsSlider,
    &this->makeupSlider,
    &this->mixSlider,
  };
  this->allSliderLabels = {
    &this->threshLabel,
    &this->ratioLabel,
    &this->kneeLabel,
    &this->tAttLabel,
    &this->tRelLabel,
    &this->tRmsLabel,
    &this->makeupLabel,
    &this->mixLabel,
  };
  this->allSliderAttachments = {
    &this->p_threshSliderAttachment,
    &this->p_ratioSliderAttachment,
    &this->p_kneeSliderAttachment,
    &this->p_tAttSliderAttachment,
    &this->p_tRelSliderAttachment,
    &this->p_tRmsSliderAttachment,
    &this->p_makeupSliderAttachment,
    &this->p_mixSliderAttachment,
  };
  this->allToggles = {
    &this->rmsToggle,
    &this->feedbackToggle,
    &this->linToggle,
    &this->bypassToggle,
  };
  this->allToggleAttachments = {
    &this->p_rmsToggleAttachment,
    &this->p_feedbackToggleAttachment,
    &this->p_linToggleAttachment,
    &this->p_bypassToggleAttachment,
  };
  this->allMeterLabels = {
    &this->inMeterLabel,
    &this->grMeterLabel,
    &this->outMeterLabel,
  };

  constexpr bool placeAbove = false;
  for (size_t i = 0; i < this->allSliders.size(); i++) {
    this->allSliders[i]->setLookAndFeel(&this->knobLookAndFeel);
    this->allSliders[i]->setTextBoxStyle(juce::Slider::TextBoxBelow,
        false,
        this->sliderWidth * 2 - 2 * this->entryPad,
        this->entryHeight);
    this->allSliderLabels[i]->attachToComponent(this->allSliders[i], placeAbove);
    this->allSliderLabels[i]->setJustificationType(juce::Justification::centred);
    this->allSliderLabels[i]->setText(this->audioProcessor.plug.floatParameters[i].name,
        juce::NotificationType::dontSendNotification);
    this->allSliders[i]->setTextValueSuffix(
        this->audioProcessor.plug.floatParameters[i].suffix);
    this->allSliders[i]->setSliderStyle(juce::Slider::SliderStyle::Rotary);
    this->allSliders[i]->addListener(this);
    addAndMakeVisible(this->allSliders[i]);
    addAndMakeVisible(*this->allSliderLabels[i]);
    this->allSliderAttachments[i]->reset(new SA(this->audioProcessor.parameters,
        this->audioProcessor.plug.floatParameters[i].name,
        *this->allSliders[i]));
  }

  for (size_t i = 0; i < this->allToggles.size(); i++) {
    addAndMakeVisible(this->allToggles[i]);
    this->allToggles[i]->setClickingTogglesState(true);
    this->allToggles[i]->setToggleable(true);
    this->allToggles[i]->addListener(this);
    // this->allToggles[i]->onClick = [this]() { this->audioProcessor.plug.reset(); };
    this->allToggles[i]->setButtonText(
        this->audioProcessor.plug.boolParameters[i].name);
    this->allToggleAttachments[i]->reset(new BA(this->audioProcessor.parameters,
        this->audioProcessor.plug.boolParameters[i].name,
        *this->allToggles[i]));
  }
  std::vector<std::string> meterLabelStrings = { "IN", "OUT", "GR" };
  for (size_t i = 0; i < this->allMeterLabels.size(); i++) {
    this->addAndMakeVisible(*this->allMeterLabels[i]);
    this->allMeterLabels[i]->setText(
        meterLabelStrings[i], juce::NotificationType::dontSendNotification);
  }
  this->addAndMakeVisible(this->meterScale);
  // this->addAndMakeVisible(this->frameCounterLabel);
  this->drawGui();
}

NtCompressorAudioProcessorEditor::~NtCompressorAudioProcessorEditor() { }

void NtCompressorAudioProcessorEditor::paint(juce::Graphics& g) {
  g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void NtCompressorAudioProcessorEditor::resized() { this->drawGui(); }

void NtCompressorAudioProcessorEditor::drawGui() {
  auto area = getLocalBounds();
  area.removeFromTop(10);
  area.removeFromLeft(10);
  area.removeFromBottom(10);
  area.removeFromRight(10);
  auto meterWidth = this->meters[0].getWidth();
  auto meterArea  = area.removeFromLeft((this->meters.size() + 1) * meterWidth);
  auto labelArea  = meterArea.removeFromTop(this->entryHeight);
  for (size_t i = 0; i < this->allMeterLabels.size(); i++) {
    auto oneLabelArea = labelArea.removeFromLeft(2 * meterWidth);
    this->allMeterLabels[i]->setBounds(oneLabelArea);
    this->allMeterLabels[i]->setJustificationType(juce::Justification::centred);
  }
  for (size_t i = 0; i < this->meters.size(); i++) {
    auto oneMeterArea = meterArea.removeFromLeft(meterWidth);
    this->meters[i].setBounds(oneMeterArea);
  }
  this->meterScale.setBounds(meterArea);

  // GRID STUFF.
  using Track = juce::Grid::TrackInfo;
  using Fr    = juce::Grid::Fr;
  juce::Grid grid;
  grid.alignContent   = juce::Grid::AlignContent::center;
  grid.justifyItems   = juce::Grid::JustifyItems::center;
  grid.justifyContent = juce::Grid::JustifyContent::spaceBetween;
  grid.templateRows   = {
    Track(Fr(1)),
    Track(Fr(2)),
    Track(Fr(1)),
    Track(Fr(2)),
    Track(Fr(1)),
    Track(Fr(1)),
  };
  grid.templateColumns = {
    Track(Fr(1)),
    Track(Fr(1)),
    Track(Fr(1)),
    Track(Fr(1)),
    // Track(Fr(1)),
    // Track(Fr(1)),
    // Track(Fr(1)),
  };
  // for (int i = 0; i < this->allSliders.size(); i++) {
  //   auto t = Track(Fr(1));
  //   grid.templateColumns.add(t);
  // }
  grid.items = {
    juce::GridItem(this->threshLabel),
    juce::GridItem(this->ratioLabel),
    juce::GridItem(this->tAttLabel),
    juce::GridItem(this->tRelLabel),

    juce::GridItem(this->threshSlider),
    juce::GridItem(this->ratioSlider),
    juce::GridItem(this->tAttSlider),
    juce::GridItem(this->tRelSlider),

    juce::GridItem(this->kneeLabel),
    juce::GridItem(this->tRmsLabel),
    juce::GridItem(this->makeupLabel),
    juce::GridItem(this->mixLabel),

    // juce::GridItem(this->frameCounterLabel),
    juce::GridItem(this->kneeSlider),
    juce::GridItem(this->tRmsSlider),
    juce::GridItem(this->makeupSlider),
    juce::GridItem(this->mixSlider),

    juce::GridItem(),
    juce::GridItem(),
    juce::GridItem(),
    juce::GridItem(),

    juce::GridItem(this->rmsToggle)
        .withHeight(this->buttonHeight)
        .withWidth(this->buttonWidth),
    juce::GridItem(this->feedbackToggle)
        .withHeight(this->buttonHeight)
        .withWidth(this->buttonWidth),
    juce::GridItem(this->linToggle)
        .withHeight(this->buttonHeight)
        .withWidth(this->buttonWidth),
    juce::GridItem(this->bypassToggle)
        .withHeight(this->buttonHeight)
        .withWidth(this->buttonWidth),
  };

  // for (auto label : this->allSliderLabels) {
  //   grid.items.add(juce::GridItem(label));
  // }
  // for (auto slider : this->allSliders) {
  //   grid.items.add(juce::GridItem(slider));
  // }
  // for (int i = 0; i < 3; i++) {
  //   grid.items.add(juce::GridItem());
  // }
  // for (auto toggle : this->allToggles) {
  //   grid.items.add(juce::GridItem(toggle));
  // }
  grid.performLayout(area);
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

void NtCompressorAudioProcessorEditor::sliderValueChanged(juce::Slider* slider) {
  if (slider == &this->threshSlider) {
    this->audioProcessor.plug.thresh_db = slider->getValue();
  } else if (slider == &this->ratioSlider) {
    this->audioProcessor.plug.ratio = slider->getValue();
  } else if (slider == &this->kneeSlider) {
    this->audioProcessor.plug.knee_db = slider->getValue();
  } else if (slider == &this->tAttSlider) {
    this->audioProcessor.plug.tAtt_ms = slider->getValue();
  } else if (slider == &this->tRelSlider) {
    this->audioProcessor.plug.tRel_ms = slider->getValue();
  } else if (slider == &this->tRmsSlider) {
    this->audioProcessor.plug.tRms_ms = slider->getValue();
  } else if (slider == &this->makeupSlider) {
    this->audioProcessor.plug.makeup_db = slider->getValue();
  } else if (slider == &this->mixSlider) {
    this->audioProcessor.plug.mix_percent = slider->getValue();
  }
  this->audioProcessor.plug.update();
}

void NtCompressorAudioProcessorEditor::buttonClicked(juce::Button* button) {
  bool val = button->getToggleState();
  if (button == &this->bypassToggle) {
    this->audioProcessor.plug.bypassEnable = val;
  } else if (button == &this->feedbackToggle) {
    this->audioProcessor.plug.feedbackEnable = val;
  } else if (button == &this->linToggle) {
    this->audioProcessor.plug.linEnable = val;
  } else if (button == &this->rmsToggle) {
    this->audioProcessor.plug.rmsEnable = val;
  }
  this->audioProcessor.plug.update();
}