/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#define Q(x) #x
#define QUOTE(x) Q(x)

#pragma once
#if !JUCE_DONT_DECLARE_PROJECTINFO
namespace ProjectInfo {
const char* const projectName   = QUOTE(NTFX_PLUGIN);
const char* const companyName   = "NtFx";
const char* const versionString = "0.1.0";
const int versionNumber         = 0x100;
}
#endif

#include NTFX_PLUGIN_FILE
#include "lib/SampleRateConverter.h"
#include "lib/UiSpec.h"

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_audio_plugin_client/juce_audio_plugin_client.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_processors_headless/juce_audio_processors_headless.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_core/juce_core.h>
#include <juce_data_structures/juce_data_structures.h>
#include <juce_events/juce_events.h>
#include <juce_graphics/juce_graphics.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_gui_extra/juce_gui_extra.h>

//==============================================================================
/**
 */
struct NtCompressorAudioProcessor : public juce::AudioProcessor {
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

  void updateOversampling(int mode);
  juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

  NtFx::TitleBarSpec titleBarSpec;
  // TODO: this should not be here.
  float fsBase = 48000;
  NtFx::Src::Coeffs<float> srcCoeffs;
  NtFx::Src::State<float> srcState;
  NTFX_PLUGIN<float> plug;
  juce::AudioProcessorValueTreeState parameters;
  // bool isInitialized = false;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NtCompressorAudioProcessor)
};
