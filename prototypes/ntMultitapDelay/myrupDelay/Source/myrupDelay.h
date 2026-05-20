//
//  Academic License - for use in teaching, academic research, and meeting
//  course requirements at degree granting institutions only.  Not for
//  government, commercial, or other organizational use.
//
//  myrupDelay.h
//
//  Code generation for function 'myrupDelay'
//


#ifndef MYRUPDELAY_H
#define MYRUPDELAY_H

// Include files
#include "rtwtypes.h"
#include "coder_array.h"
#include <cstddef>
#include <cstdlib>

// Type Declarations
struct myrupDelayStackData;

// Type Definitions
class derivedAudioPlugin
{
 public:
  derivedAudioPlugin *init(myrupDelayStackData *SD);
  void set_tempo(double val);
  void update();
  void set_mix_gui(double val);
  void set_level4(double val);
  void set_level8dot(double val);
  void set_level8(double val);
  void set_level16dot(double val);
  void set_level16(double val);
  void set_pan4(double val);
  void set_pan8dot(double val);
  void set_pan8(double val);
  void set_pan16dot(double val);
  void set_pan16(double val);
  void set_fb4(double val);
  void set_fb8dot(double val);
  void set_fb8(double val);
  void set_fb16dot(double val);
  void set_fb16(double val);
  void setSampleRateForReset(double rate);
  void reset();
  double getSampleRate() const;
  void process(const coder::array<double, 2U> &x, coder::array<double, 2U> &y);
  int getLatencyInSamplesInt32() const;
  derivedAudioPlugin();
  ~derivedAudioPlugin();
  double fs;
  boolean_T bypass;
  double tempo;
  double mix;
  double mix_gui;
  double level4;
  double level8dot;
  double level8;
  double level16dot;
  double level16;
  double level[5];
  double pan4;
  double pan8dot;
  double pan8;
  double pan16dot;
  double pan16;
  double pan[5];
  double fb4;
  double fb8;
  double fb16;
  double fb16dot;
  double fb8dot;
  double fb[5];
  double del[5];
  double dLine[307200];
  double storeLoc;
 protected:
  int PrivateLatency;
 private:
  double PrivateSampleRate;
};

// Function Declarations
extern void createPluginInstance(myrupDelayStackData *SD, unsigned long thisPtr);
extern int getLatencyInSamplesCImpl(myrupDelayStackData *SD);
extern void myrupDelay_initialize(myrupDelayStackData *SD);
extern void myrupDelay_terminate();
extern void onParamChangeCImpl(myrupDelayStackData *SD, int paramIdx, double
  value);
extern void processEntryPoint(myrupDelayStackData *SD, double samplesPerFrame,
  const double i1_data[], const int i1_size[1], const double i2_data[], const
  int i2_size[1], double o1_data[], int o1_size[1], double o2_data[], int
  o2_size[1]);
extern void resetCImpl(myrupDelayStackData *SD, double rate);

#endif

// End of code generation (myrupDelay.h)
