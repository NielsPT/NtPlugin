/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include "NtCompressorPlugin.h"
#include <JuceHeader.h>
//==============================================================================
/**
 */
class NtCompressorAudioProcessor : public juce::AudioProcessor {
public:
  NtCompressorAudioProcessor();
  ~NtCompressorAudioProcessor() override;
  void prepareToPlay(double sampleRate, int samplesPerBlock) override;
  void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
  bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
#endif

  void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

  juce::AudioProcessorEditor* createEditor() override;
  bool hasEditor() const override;

  const juce::String getName() const override;

  bool acceptsMidi() const override;
  bool producesMidi() const override;
  bool isMidiEffect() const override;
  double getTailLengthSeconds() const override;

  int getNumPrograms() override;
  int getCurrentProgram() override;
  void setCurrentProgram(int index) override;
  const juce::String getProgramName(int index) override;
  void changeProgramName(int index, const juce::String& newName) override;

  void getStateInformation(juce::MemoryBlock& destData) override;
  void setStateInformation(const void* data, int sizeInBytes) override;
  void setPeakLevel(int channelIndex, float peakLevel);
  float getPeakLevel(int channelIndex);
  float getGainReduction();

  NtFx::CompressorPlugin<float> plug;
  juce::AudioProcessorValueTreeState parameters;

private:
  std::array<std::atomic<float>, 2> m_peakLevels;
  juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NtCompressorAudioProcessor)
};
