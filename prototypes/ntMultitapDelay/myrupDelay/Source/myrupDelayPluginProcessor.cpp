#include "../JuceLibraryCode/JuceHeader.h"

#include "myrupDelay.h"
#include "myrupDelay_types.h"

struct onParamChangeListener : AudioProcessorValueTreeState::Listener {
   onParamChangeListener(myrupDelayStackData* sd)
       : SD(sd) {
   }

   void parameterChanged(const String& parameterID, float newValue) override {
      (void)parameterID;
      int idx = -1;
      if (parameterID == "tempo") {
         idx = 0;
      } else if (parameterID == "mix_gui") {
         idx = 1;
      } else if (parameterID == "level4") {
         idx = 2;
      } else if (parameterID == "level8dot") {
         idx = 3;
      } else if (parameterID == "level8") {
         idx = 4;
      } else if (parameterID == "level16dot") {
         idx = 5;
      } else if (parameterID == "level16") {
         idx = 6;
      } else if (parameterID == "pan4") {
         idx = 7;
      } else if (parameterID == "pan8dot") {
         idx = 8;
      } else if (parameterID == "pan8") {
         idx = 9;
      } else if (parameterID == "pan16dot") {
         idx = 10;
      } else if (parameterID == "pan16") {
         idx = 11;
      } else if (parameterID == "fb4") {
         idx = 12;
      } else if (parameterID == "fb8dot") {
         idx = 13;
      } else if (parameterID == "fb8") {
         idx = 14;
      } else if (parameterID == "fb16dot") {
         idx = 15;
      } else if (parameterID == "fb16") {
         idx = 16;
      } else if (parameterID == "bypass") {
         idx = 17;
      }
      onParamChangeCImpl(SD, idx, static_cast<double>(newValue));
   }

   myrupDelayStackData* SD;
};

//==============================================================================
class myrupDelayAudioProcessor : public AudioProcessor {
public:
   //==============================================================================
   myrupDelayAudioProcessor()
       : paramListener(&mStackData)
       , parameters(*this, nullptr) {
      mStackData.pd = &mPersistentData;

      myrupDelay_initialize(&mStackData);

      createPluginInstance(&mStackData, reinterpret_cast<unsigned long long>(this));

      //
      // Parameter property tempo
      //
      parameters.createAndAddParameter(
          "tempo", "Tempo", "bpm",
          NormalisableRange<float>(60.f, 300.f), 120.f,
          [](float val) { return String(val, 3); },
          nullptr);
      parameters.addParameterListener("tempo", &paramListener);

      //
      // Parameter property mix_gui
      //
      parameters.createAndAddParameter(
          "mix_gui", "Mix", "%",
          NormalisableRange<float>(0.f, 100.f), 100.f,
          [](float val) { return String(val, 3); },
          nullptr);
      parameters.addParameterListener("mix_gui", &paramListener);

      //
      // Parameter property level4
      //
      parameters.createAndAddParameter(
          "level4", "1/4", "dB",
          NormalisableRange<float>(-100.f, 0.f), -6.f,
          [](float val) { return String(val, 3); },
          nullptr);
      parameters.addParameterListener("level4", &paramListener);

      //
      // Parameter property level8dot
      //
      parameters.createAndAddParameter(
          "level8dot", "1/8.", "dB",
          NormalisableRange<float>(-100.f, 0.f), -6.f,
          [](float val) { return String(val, 3); },
          nullptr);
      parameters.addParameterListener("level8dot", &paramListener);

      //
      // Parameter property level8
      //
      parameters.createAndAddParameter(
          "level8", "1/8", "dB",
          NormalisableRange<float>(-100.f, 0.f), -6.f,
          [](float val) { return String(val, 3); },
          nullptr);
      parameters.addParameterListener("level8", &paramListener);

      //
      // Parameter property level16dot
      //
      parameters.createAndAddParameter(
          "level16dot", " 1/16.", "dB",
          NormalisableRange<float>(-100.f, 0.f), -6.f,
          [](float val) { return String(val, 3); },
          nullptr);
      parameters.addParameterListener("level16dot", &paramListener);

      //
      // Parameter property level16
      //
      parameters.createAndAddParameter(
          "level16", "1/16", "dB",
          NormalisableRange<float>(-100.f, 0.f), -6.f,
          [](float val) { return String(val, 3); },
          nullptr);
      parameters.addParameterListener("level16", &paramListener);

      //
      // Parameter property pan4
      //
      parameters.createAndAddParameter(
          "pan4", "Pan", "",
          NormalisableRange<float>(-100.f, 100.f), 0.f,
          [](float val) { return String(val, 3); },
          nullptr);
      parameters.addParameterListener("pan4", &paramListener);

      //
      // Parameter property pan8dot
      //
      parameters.createAndAddParameter(
          "pan8dot", "Pan", "",
          NormalisableRange<float>(-100.f, 100.f), -50.f,
          [](float val) { return String(val, 3); },
          nullptr);
      parameters.addParameterListener("pan8dot", &paramListener);

      //
      // Parameter property pan8
      //
      parameters.createAndAddParameter(
          "pan8", "Pan", "",
          NormalisableRange<float>(-100.f, 100.f), 50.f,
          [](float val) { return String(val, 3); },
          nullptr);
      parameters.addParameterListener("pan8", &paramListener);

      //
      // Parameter property pan16dot
      //
      parameters.createAndAddParameter(
          "pan16dot", "Pan", "",
          NormalisableRange<float>(-100.f, 100.f), 100.f,
          [](float val) { return String(val, 3); },
          nullptr);
      parameters.addParameterListener("pan16dot", &paramListener);

      //
      // Parameter property pan16
      //
      parameters.createAndAddParameter(
          "pan16", "Pan", "",
          NormalisableRange<float>(-100.f, 100.f), -100.f,
          [](float val) { return String(val, 3); },
          nullptr);
      parameters.addParameterListener("pan16", &paramListener);

      //
      // Parameter property fb4
      //
      parameters.createAndAddParameter(
          "fb4", "Feedback", "%",
          NormalisableRange<float>(0.f, 100.f), 0.f,
          [](float val) { return String(val, 3); },
          nullptr);
      parameters.addParameterListener("fb4", &paramListener);

      //
      // Parameter property fb8dot
      //
      parameters.createAndAddParameter(
          "fb8dot", "Feedback", "%",
          NormalisableRange<float>(0.f, 100.f), 0.f,
          [](float val) { return String(val, 3); },
          nullptr);
      parameters.addParameterListener("fb8dot", &paramListener);

      //
      // Parameter property fb8
      //
      parameters.createAndAddParameter(
          "fb8", "Feedback", "%",
          NormalisableRange<float>(0.f, 100.f), 0.f,
          [](float val) { return String(val, 3); },
          nullptr);
      parameters.addParameterListener("fb8", &paramListener);

      //
      // Parameter property fb16dot
      //
      parameters.createAndAddParameter(
          "fb16dot", "Feedback", "%",
          NormalisableRange<float>(0.f, 100.f), 0.f,
          [](float val) { return String(val, 3); },
          nullptr);
      parameters.addParameterListener("fb16dot", &paramListener);

      //
      // Parameter property fb16
      //
      parameters.createAndAddParameter(
          "fb16", "Feedback", "%",
          NormalisableRange<float>(0.f, 100.f), 0.f,
          [](float val) { return String(val, 3); },
          nullptr);
      parameters.addParameterListener("fb16", &paramListener);

      //
      // Parameter property bypass
      //
      const StringArray choices18({ "In", "Out" });
      parameters.createAndAddParameter(
          "bypass", "Master Bypass", "",
          NormalisableRange<float>(0.f, choices18.size() - 1.f, 1.f), 0.f,
          [=](float value) { return choices18[(int)(value + 0.5)]; },
          [=](const String& text) { return (float)choices18.indexOf(text); },
          false, true, true);
      parameters.addParameterListener("bypass", &paramListener);

      parameters.state = ValueTree(Identifier("myrupDelay"));
   }

   ~myrupDelayAudioProcessor() {
      myrupDelay_terminate();
   }

   //==============================================================================
   void prepareToPlay(double sampleRate, int samplesPerBlock) override {
      (void)samplesPerBlock;
      resetCImpl(&mStackData, sampleRate);
      setLatencySamples(getLatencyInSamplesCImpl(&mStackData));
   }

   void releaseResources() override { }

   void processBlock(AudioBuffer<double>& buffer, MidiBuffer& midiMessages) override {
      (void)midiMessages;
      ScopedNoDenormals noDenormals;
      const double** inputs = buffer.getArrayOfReadPointers();
      double** outputs = buffer.getArrayOfWritePointers();
      int nSamples = buffer.getNumSamples();
      myrupDelayStackData* SD = &mStackData;

      int osz0_;
      int osz1_;
      if (nSamples <= MAX_SAMPLES_PER_FRAME) {
         /* Fast path for common frame sizes. */
         int isz0_ = nSamples;
         int isz1_ = nSamples;
         processEntryPoint(SD, (double)nSamples,
             inputs[0], &isz0_,
             inputs[1], &isz1_,
             outputs[0], &osz0_,
             outputs[1], &osz1_);
      } else {
         /* Fallback for unusually large frames. */
         int isz0_ = MAX_SAMPLES_PER_FRAME;
         int isz1_ = MAX_SAMPLES_PER_FRAME;
         int n = MAX_SAMPLES_PER_FRAME;
         for (int i_ = 0; i_ < nSamples; i_ += MAX_SAMPLES_PER_FRAME) {
            if (i_ + MAX_SAMPLES_PER_FRAME > nSamples) {
               n = nSamples - i_;
               isz0_ = nSamples - i_;
               isz1_ = nSamples - i_;
            }
            processEntryPoint(SD, (double)n,
                inputs[0] + i_, &isz0_,
                inputs[1] + i_, &isz1_,
                outputs[0] + i_, &osz0_,
                outputs[1] + i_, &osz1_);
         }
      }
   }

   void processBlock(AudioBuffer<float>& buffer, MidiBuffer& midiMessages) override {
      (void)midiMessages;
      AudioBuffer<double> doubleBuffer;
      doubleBuffer.makeCopyOf(buffer);
      processBlock(doubleBuffer, midiMessages);
      buffer.makeCopyOf(doubleBuffer);
   }

   //==============================================================================
   bool hasEditor() const override { return true; }
   AudioProcessorEditor* createEditor() override;

   //==============================================================================
   const String getName() const override { return JucePlugin_Name; }

   bool acceptsMidi() const override { return false; }
   bool producesMidi() const override { return false; }
   bool isMidiEffect() const override { return false; }
   double getTailLengthSeconds() const override { return 0.0; }

   //==============================================================================
   // NB: some hosts don't cope very well if you tell them there are 0 programs,
   // so this should be at least 1, even if you're not really implementing programs.
   int getNumPrograms() override { return 1; }
   int getCurrentProgram() override { return 0; }
   void setCurrentProgram(int index) override { (void)index; }
   const String getProgramName(int index) override {
      (void)index;
      return {};
   }
   void changeProgramName(int index, const String& newName) override {
      (void)index;
      (void)newName;
   }

   //==============================================================================
   void getStateInformation(MemoryBlock& destData) override {
      ScopedPointer<XmlElement> xml(parameters.state.createXml());
      copyXmlToBinary(*xml, destData);
   }

   void setStateInformation(const void* data, int sizeInBytes) override {
      ScopedPointer<XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
      if (xmlState != nullptr)
         if (xmlState->hasTagName(parameters.state.getType()))
            parameters.state = ValueTree::fromXml(*xmlState);
   }

   bool supportsDoublePrecisionProcessing() const override { return true; }

private:
   //==============================================================================
   static const int MAX_SAMPLES_PER_FRAME = 4096;

   myrupDelayStackData mStackData;
   myrupDelayPersistentData mPersistentData;
   onParamChangeListener paramListener;

   //==============================================================================
   AudioProcessorValueTreeState parameters;

   JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(myrupDelayAudioProcessor)
};

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
   return new myrupDelayAudioProcessor();
}

#include "myrupDelayPluginEditor.h"

AudioProcessorEditor* myrupDelayAudioProcessor::createEditor() {
   return new myrupDelayAudioProcessorEditor(*this, parameters);
}
