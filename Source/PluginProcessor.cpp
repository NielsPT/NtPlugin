/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include <algorithm>
#include <string>

#include "PluginEditor.h"
#include "PluginProcessor.h"

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
    , parameters(*this,
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

int NtCompressorAudioProcessor::getNumPrograms() {
  return 1; // NB: some hosts don't cope very well if you tell them there are 0
            // programs, so this should be at least 1, even if you're not really
            // implementing programs.
}

int NtCompressorAudioProcessor::getCurrentProgram() { return 0; }

void NtCompressorAudioProcessor::setCurrentProgram(int index) { }

const juce::String NtCompressorAudioProcessor::getProgramName(int index) { return {}; }

void NtCompressorAudioProcessor::changeProgramName(
    int index, const juce::String& newName) { }

//==============================================================================
void NtCompressorAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) {
  // Use this method as the place to do any pre-playback
  // initialisation that you need..
  plug.fs = sampleRate;
  plug.update();
  plug.reset();
  // juce::NativeMessageBox::showMessageBoxAsync(
  //     juce::MessageBoxIconType::InfoIcon, "Function called", "prepareToPlay");
}

juce::AudioChannelSet m_outputFormat;
void NtCompressorAudioProcessor::releaseResources() {
  // When playback stops, you can use this as an opportunity to free up any
  // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool NtCompressorAudioProcessor::isBusesLayoutSupported(
    const BusesLayout& layouts) const {
  #if JucePlugin_IsMidiEffect
  juce::ignoreUnused(layouts);
  return true;
  #else
  // This is the place where you check if the layout is supported.
  // In this template code we only support mono or stereo.
  // Some plugin hosts, such as certain GarageBand versions, will only
  // load plugins that support stereo bus layouts.
  if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
      && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
    return false;

    // This checks if the input layout matches the output layout
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

  // In case we have more outputs than inputs, this code clears any output
  // channels that didn't contain input data, (because these aren't
  // guaranteed to be empty - they may contain garbage).
  // This is here to avoid people getting screaming feedback
  // when they first compile a plugin, but obviously you don't need to keep
  // this code if your algorithm always overwrites all the output channels.
  for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i) {
    buffer.clear(i, 0, buffer.getNumSamples());
  }

  auto leftBuffer  = buffer.getWritePointer(0);
  auto rightBuffer = buffer.getWritePointer(1);
  for (size_t i = 0; i < buffer.getNumSamples(); i++) {
    Stereo<float> x = { leftBuffer[i], rightBuffer[i] };
    auto y          = plug.processSample(x);
    leftBuffer[i]   = y.l;
    rightBuffer[i]  = y.r;
  }

  for (int channel = 0; channel < totalNumInputChannels; ++channel) {
    setPeakLevel(channel, buffer.getMagnitude(channel, 0, buffer.getNumSamples()));
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
void NtCompressorAudioProcessor::getStateInformation(juce::MemoryBlock& destData) {
  auto state = this->parameters.copyState();
  std::unique_ptr<juce::XmlElement> xml(state.createXml());
  copyXmlToBinary(*xml, destData);
}

void NtCompressorAudioProcessor::setStateInformation(
    const void* data, int sizeInBytes) {
  std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
  if (xmlState.get() != nullptr)
    if (xmlState->hasTagName(parameters.state.getType()))
      parameters.replaceState(juce::ValueTree::fromXml(*xmlState));
}

void NtCompressorAudioProcessor::setPeakLevel(int channelIndex, float peakLevel) {
  if (!juce::isPositiveAndBelow(channelIndex, m_peakLevels.size())) return;

  m_peakLevels[channelIndex].store(
      std::max(peakLevel, m_peakLevels[channelIndex].load()));
}

float NtCompressorAudioProcessor::getPeakLevel(int channelIndex) {
  if (!juce::isPositiveAndBelow(channelIndex, m_peakLevels.size())) return 0.0f;

  return m_peakLevels[channelIndex].exchange(0.0f);
}

float NtCompressorAudioProcessor::getGainReduction() { return this->plug.grState; }

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
  return new NtCompressorAudioProcessor();
}

juce::AudioProcessorValueTreeState::ParameterLayout
NtCompressorAudioProcessor::createParameterLayout() {
  juce::AudioProcessorValueTreeState::ParameterLayout parameters;
  for (auto p : this->plug.floatParameters) {
    std::string name(p.name);
    std::replace(name.begin(), name.end(), '_', ' ');
    parameters.add(std::make_unique<juce::AudioParameterFloat>(
        p.name, name, p.minVal, p.maxVal, p.defaultVal));
  }
  for (auto p : this->plug.boolParameters) {
    std::string name(p.name);
    std::replace(name.begin(), name.end(), '_', ' ');
    parameters.add(
        std::make_unique<juce::AudioParameterBool>(p.name, name, p.defaultVal));
  }
  return parameters;
}