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
#include "juce_core/system/juce_PlatformDefs.h"
#include "lib/SampleRateConverter.h"
#include "lib/Stereo.h"
#include "lib/UiSpec.h"

#include <cstddef>
#include <string>
#include <type_traits>
#include <vector>

NtPluginAudioProcessor::NtPluginAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(BusesProperties()
  #if !JucePlugin_IsMidiEffect
    #if !JucePlugin_IsSynth
              .withInput("Input", juce::AudioChannelSet::stereo(), true)
    #endif
              .withOutput("Output", juce::AudioChannelSet::stereo(), true)
  #endif
              )
#endif
      ,
      paramLayout(*this,
          nullptr,
          juce::Identifier(JucePlugin_Name),
          createParameterLayout()),
      src(plug) {
}

NtPluginAudioProcessor::~NtPluginAudioProcessor() { }

const juce::String NtPluginAudioProcessor::getName() const {
  return JucePlugin_Name;
}

bool NtPluginAudioProcessor::acceptsMidi() const {
#if JucePlugin_WantsMidiInput
  return true;
#else
  return false;
#endif
}

bool NtPluginAudioProcessor::producesMidi() const {
#if JucePlugin_ProducesMidiOutput
  return true;
#else
  return false;
#endif
}

bool NtPluginAudioProcessor::isMidiEffect() const {
#if JucePlugin_IsMidiEffect
  return true;
#else
  return false;
#endif
}

double NtPluginAudioProcessor::getTailLengthSeconds() const { return 0.0; }
int NtPluginAudioProcessor::getNumPrograms() { return 1; }
int NtPluginAudioProcessor::getCurrentProgram() { return 0; }
void NtPluginAudioProcessor::setCurrentProgram(int index) { }
const juce::String NtPluginAudioProcessor::getProgramName(int index) {
  return {};
}
void NtPluginAudioProcessor::changeProgramName(
    int index, const juce::String& newName) { }

void NtPluginAudioProcessor::prepareToPlay(
    double sampleRate, int samplesPerBlock) {
  this->fsBase = sampleRate;
  this->updateOversampling(1);
  this->plug.xRms[0].reset(sampleRate);
  this->plug.xRms[1].reset(sampleRate);
}

juce::AudioChannelSet m_outputFormat;
void NtPluginAudioProcessor::releaseResources() { }

#ifndef JucePlugin_PreferredChannelConfigurations
bool NtPluginAudioProcessor::isBusesLayoutSupported(
    const BusesLayout& layouts) const {
  #if JucePlugin_IsMidiEffect
  juce::ignoreUnused(layouts);
  return true;
  #else
  if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
      && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
    return false;
    #if !JucePlugin_IsSynth
  if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
    return false;
    #endif

  return true;
  #endif
}
#endif

void NtPluginAudioProcessor::processBlock(
    juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) {
  juce::ScopedNoDenormals noDenormals;
  auto totalNumInputChannels  = getTotalNumInputChannels();
  auto totalNumOutputChannels = getTotalNumOutputChannels();
  for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i) {
    buffer.clear(i, 0, buffer.getNumSamples());
  }
  auto p_playHead = this->getPlayHead();
  auto p_posInfo  = p_playHead->getPosition();

  if (p_posInfo) {
    auto tempo = p_posInfo->getBpm();
    if (tempo && tempo != this->plug.tempo) {
      this->plug.tempo = *tempo;
      this->plug.onTempoChanged();
    }
  }
  auto leftBuffer  = buffer.getWritePointer(0);
  auto rightBuffer = buffer.getWritePointer(1);
  for (size_t i = 0; i < buffer.getNumSamples(); i++) {
    NtFx::Stereo<float> x { leftBuffer[i], rightBuffer[i] };
    auto y = this->src.process(x);
    if (this->plug.meters[0].addRms) { this->plug.xRms[0].process(x); }
    if (this->plug.meters[1].addRms) { this->plug.xRms[1].process(y); }
    leftBuffer[i]  = y.l;
    rightBuffer[i] = y.r;
  }
}

bool NtPluginAudioProcessor::hasEditor() const { return true; }

juce::AudioProcessorEditor* NtPluginAudioProcessor::createEditor() {
  return new NtPluginAudioProcessorEditor(*this);
}

void NtPluginAudioProcessor::getStateInformation(juce::MemoryBlock& destData) {
  auto state = this->paramLayout.copyState();
  std::unique_ptr<juce::XmlElement> xml(state.createXml());
  copyXmlToBinary(*xml, destData);
}

void NtPluginAudioProcessor::setStateInformation(
    const void* data, int sizeInBytes) {
  std::unique_ptr<juce::XmlElement> xmlState(
      getXmlFromBinary(data, sizeInBytes));
  if (xmlState.get() == nullptr) { return; }
  if (!xmlState->hasTagName(this->paramLayout.state.getType())) { return; }
  this->paramLayout.replaceState(juce::ValueTree::fromXml(*xmlState));
  this->loadParameter(this->plug.primaryKnobs);
  this->loadParameter(this->plug.secondaryKnobs);
  this->loadParameter(this->plug.toggles);
  this->loadParameter(this->plug.dropdowns);
  this->loadRadioButtons(this->plug.radioButtons);
  // TODO: Oversampling is not loaded.
}

template <typename T>
void NtPluginAudioProcessor::loadParameter(std::vector<T>& v) {
  for (auto& p : v) {
    if (!p.p_val) { continue; }
    auto par = this->paramLayout.getParameterAsValue(p.name);
    *p.p_val = par.getValue();
    DBG("Loaded '" << p.name << "': " << float(*p.p_val));
  }
}

void NtPluginAudioProcessor::loadRadioButtons(
    std::vector<NtFx::RadioButtonSetSpec>& v) {
  for (auto& p : v) {
    int val;
    for (size_t i = 0; i < p.options.size(); i++) {
      auto par = this->paramLayout.getParameterAsValue(
          NtFx::makeTmpToggle(p.name, p.options[i]).name);
      if (par.getValue()) { val = i; }
    }
    *p.p_val = val + 1;
    DBG("Loaded '" << p.name << "': " << float(*p.p_val));
  }
}

void NtPluginAudioProcessor::updateOversampling(int mode) {
  this->src.update(
      static_cast<NtFx::Src::oversamplingMode>(mode), this->fsBase);
  this->src.reset();
  this->plug.reset(this->src.coeffs.fsHi);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
  DBG("Creating new NtFx plugin editor.");
  return new NtPluginAudioProcessor();
}

// This is all fine, and stores to XML and reads from XML when the UI is
// initialized, but not when the plugin editor has not been opened in the
// session. How do we get the values on load?
juce::AudioProcessorValueTreeState::ParameterLayout
NtPluginAudioProcessor::createParameterLayout() {
  int i = 1;
  juce::AudioProcessorValueTreeState::ParameterLayout parameters;
  this->createParameters<float>(this->plug.primaryKnobs, parameters, i);
  this->createParameters<float>(this->plug.secondaryKnobs, parameters, i);
  this->createParameters<bool>(this->plug.toggles, parameters, i);
  this->createParameters<int>(this->plug.dropdowns, parameters, i);
  this->createParameters<int>(this->titleBarSpec.dropDowns, parameters, i);
  std::vector<NtFx::ToggleSpec> vRadioButtonParams;
  for (auto& r : this->plug.radioButtons) {
    vRadioButtonParams.clear();
    for (auto& option : r.options) {
      vRadioButtonParams.push_back(NtFx::makeTmpToggle(r.name, option));
    }
    this->createParameters<bool>(vRadioButtonParams, parameters, i);
  }
  DBG("Created " + std::to_string(i - 1) + " paramters.");
  return parameters;
}

template <typename t_val, typename t_spec>
void NtPluginAudioProcessor::createParameters(std::vector<t_spec>& vParams,
    juce::AudioProcessorValueTreeState::ParameterLayout& paramLayout,
    int& i) {
  for (auto p : vParams) {
    juce::ParameterID id(p.name, i++);
    if constexpr (std::is_same_v<t_val, int>) {
      juce::StringArray options;
      for (const auto& option : p.options) {
        options.add(juce::String(option));
      }
      paramLayout.add(std::make_unique<juce::AudioParameterChoice>(
          id, p.name, options, p._defaultVal));
    }
    if constexpr (std::is_same_v<t_val, bool>) {
      paramLayout.add(std::make_unique<juce::AudioParameterBool>(
          id, p.name, p._defaultVal));
    }
    if constexpr (std::is_floating_point_v<t_val>) {
      paramLayout.add(std::make_unique<juce::AudioParameterFloat>(
          id, p.name, p.minVal, p.maxVal, p._defaultVal));
    }
    DBG("Created parameter '" << p.name << "'.");
  }
}