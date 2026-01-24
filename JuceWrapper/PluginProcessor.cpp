/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include <algorithm>
#include <string>

#include "PluginEditor.h"
#include "PluginProcessor.h"
#include "lib/SampleRateConverter.h"
#include "lib/UiSpec.h"

//==============================================================================
NtCompressorAudioProcessor::NtCompressorAudioProcessor()
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
      parameters(*this,
          nullptr,
          juce::Identifier("NtCompressor_01"),
          createParameterLayout()) {
}

NtCompressorAudioProcessor::~NtCompressorAudioProcessor() { }

//==============================================================================
const juce::String NtCompressorAudioProcessor::getName() const {
  return JucePlugin_Name;
}

bool NtCompressorAudioProcessor::acceptsMidi() const {
#if JucePlugin_WantsMidiInput
  return true;
#else
  return false;
#endif
}

bool NtCompressorAudioProcessor::producesMidi() const {
#if JucePlugin_ProducesMidiOutput
  return true;
#else
  return false;
#endif
}

bool NtCompressorAudioProcessor::isMidiEffect() const {
#if JucePlugin_IsMidiEffect
  return true;
#else
  return false;
#endif
}

double NtCompressorAudioProcessor::getTailLengthSeconds() const { return 0.0; }
int NtCompressorAudioProcessor::getNumPrograms() { return 1; }
int NtCompressorAudioProcessor::getCurrentProgram() { return 0; }
void NtCompressorAudioProcessor::setCurrentProgram(int index) { }
const juce::String NtCompressorAudioProcessor::getProgramName(int index) {
  return {};
}
void NtCompressorAudioProcessor::changeProgramName(
    int index, const juce::String& newName) { }

//==============================================================================
void NtCompressorAudioProcessor::prepareToPlay(
    double sampleRate, int samplesPerBlock) {
  // if (!this->isInitialized) { return; }
  this->fsBase = sampleRate;
  this->updateOversampling(1);
}

juce::AudioChannelSet m_outputFormat;
void NtCompressorAudioProcessor::releaseResources() { }

#ifndef JucePlugin_PreferredChannelConfigurations
bool NtCompressorAudioProcessor::isBusesLayoutSupported(
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

void NtCompressorAudioProcessor::processBlock(
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
    if (tempo) { this->plug.tempo = *tempo; }
  }
  auto leftBuffer  = buffer.getWritePointer(0);
  auto rightBuffer = buffer.getWritePointer(1);
  for (size_t i = 0; i < buffer.getNumSamples(); i++) {
    NtFx::Stereo<float> x = { leftBuffer[i], rightBuffer[i] };
    // auto y                = plug.processSample(x);
    auto y = NtFx::Src::processSample<float>(
        this->plug, this->srcState, this->srcCoeffs, x);
    leftBuffer[i]  = y.l;
    rightBuffer[i] = y.r;
  }
}

//==============================================================================
bool NtCompressorAudioProcessor::hasEditor() const {
  return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* NtCompressorAudioProcessor::createEditor() {
  return new NtCompressorAudioProcessorEditor(*this);
}

//==============================================================================
void NtCompressorAudioProcessor::getStateInformation(
    juce::MemoryBlock& destData) {
  auto state = this->parameters.copyState();
  std::unique_ptr<juce::XmlElement> xml(state.createXml());
  copyXmlToBinary(*xml, destData);
}

void NtCompressorAudioProcessor::setStateInformation(
    const void* data, int sizeInBytes) {
  std::unique_ptr<juce::XmlElement> xmlState(
      getXmlFromBinary(data, sizeInBytes));
  if (xmlState.get() != nullptr)
    if (xmlState->hasTagName(parameters.state.getType()))
      parameters.replaceState(juce::ValueTree::fromXml(*xmlState));
}

void NtCompressorAudioProcessor::updateOversampling(int mode) {
  NtFx::Src::update(static_cast<NtFx::Src::oversamplingMode>(mode),
      this->fsBase,
      this->srcCoeffs);
  NtFx::Src::reset(this->srcState);
  this->plug.reset(this->srcCoeffs.fsHi);
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
  DBG("Creating new NtFx plugin editor.");
  return new NtCompressorAudioProcessor();
}

juce::AudioProcessorValueTreeState::ParameterLayout
NtCompressorAudioProcessor::createParameterLayout() {
  int i = 1;
  juce::AudioProcessorValueTreeState::ParameterLayout parameters;
  for (auto p : this->plug.primaryKnobs) {
    juce::ParameterID id(p.name, i++);
    std::string name(p.name);
    std::replace(name.begin(), name.end(), '_', ' ');
    parameters.add(std::make_unique<juce::AudioParameterFloat>(
        id, name, p.minVal, p.maxVal, p.defaultVal));
  }
  for (auto p : this->plug.secondaryKnobs) {
    juce::ParameterID id(p.name, i++);
    std::string name(p.name);
    std::replace(name.begin(), name.end(), '_', ' ');
    parameters.add(std::make_unique<juce::AudioParameterFloat>(
        id, name, p.minVal, p.maxVal, p.defaultVal));
  }
  for (auto p : this->plug.toggles) {
    juce::ParameterID id(p.name, i++);
    std::string name(p.name);
    std::replace(name.begin(), name.end(), '_', ' ');
    parameters.add(
        std::make_unique<juce::AudioParameterBool>(id, name, p.defaultVal));
  }
  for (auto d : this->titleBarSpec.dropDowns) {
    juce::ParameterID id(d.name, i++);
    std::string name(d.name);
    std::replace(name.begin(), name.end(), '_', ' ');
    juce::StringArray options;
    for (const auto& str : d.options) { options.add(juce::String(str)); }
    parameters.add(std::make_unique<juce::AudioParameterChoice>(
        id, name, options, d.defaultIdx));
  }
  DBG("Created " + std::to_string(i) + " paramters.");
  return parameters;
}