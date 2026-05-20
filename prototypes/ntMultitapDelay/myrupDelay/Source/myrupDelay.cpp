//
//  Academic License - for use in teaching, academic research, and meeting
//  course requirements at degree granting institutions only.  Not for
//  government, commercial, or other organizational use.
//
//  myrupDelay.cpp
//
//  Code generation for function 'myrupDelay'
//


// Include files
#include "myrupDelay.h"
#include "myrupDelay_types.h"
#include "rt_nonfinite.h"
#include "coder_array.h"
#include "rt_nonfinite.h"
#include <cmath>

// Function Declarations
static derivedAudioPlugin *getPluginInstance(myrupDelayStackData *SD);
static void getPluginInstance_init(myrupDelayStackData *SD);
static double rt_powd_snf(double u0, double u1);
static double rt_roundd_snf(double u);

// Function Definitions
int derivedAudioPlugin::getLatencyInSamplesInt32() const
{
  return this->PrivateLatency;
}

double derivedAudioPlugin::getSampleRate() const
{
  return this->PrivateSampleRate;
}

derivedAudioPlugin *derivedAudioPlugin::init(myrupDelayStackData *SD)
{
  derivedAudioPlugin *plugin;
  int i;
  plugin = this;

  //  Pass constructor args to plugin.
  plugin->fs = 48000.0;
  plugin->bypass = false;
  plugin->tempo = 120.0;
  plugin->mix = 1.0;
  plugin->mix_gui = 100.0;
  plugin->level4 = -6.0;
  plugin->level8dot = -6.0;
  plugin->level8 = -6.0;
  plugin->level16dot = -6.0;
  plugin->level16 = -6.0;
  for (i = 0; i < 5; i++) {
    plugin->level[i] = 0.0;
  }

  plugin->pan4 = 0.0;
  plugin->pan8dot = -50.0;
  plugin->pan8 = 50.0;
  plugin->pan16dot = 100.0;
  plugin->pan16 = -100.0;
  for (i = 0; i < 5; i++) {
    plugin->pan[i] = 0.0;
  }

  plugin->fb4 = 0.0;
  plugin->fb8 = 0.0;
  plugin->fb16 = 0.0;
  plugin->fb16dot = 0.0;
  plugin->fb8dot = 0.0;
  for (i = 0; i < 5; i++) {
    plugin->fb[i] = 0.0;
  }

  for (i = 0; i < 5; i++) {
    plugin->del[i] = 0.0;
  }

  for (i = 0; i < 307200; i++) {
    plugin->dLine[i] = 0.0;
  }

  plugin->storeLoc = 1.0;
  plugin->PrivateLatency = 0;
  if (!SD->pd->thisPtr_not_empty) {
    SD->pd->thisPtr = 0UL;
    SD->pd->thisPtr_not_empty = true;
  }

  return plugin;
}

void derivedAudioPlugin::process(const coder::array<double, 2U> &x, coder::array<
  double, 2U> &y)
{
  double dv[1];
  double accumFb_idx_0;
  double accumFb_idx_1;
  double accum_idx_0;
  double accum_idx_1;
  int n;
  int u0;
  u0 = x.size(0);
  if (u0 <= 2) {
    u0 = 2;
  }

  if (x.size(0) == 0) {
    n = 0;
  } else {
    n = u0;
  }

  if (this->bypass) {
    y.set_size(x.size(0), 2);
    u0 = x.size(0) * x.size(1);
    for (n = 0; n < u0; n++) {
      y[n] = x[n];
    }
  } else {
    y.set_size(n, 2);
    for (int i = 0; i < n; i++) {
      double b;
      double loc;
      accum_idx_0 = 0.0;
      accumFb_idx_0 = 0.0;
      accum_idx_1 = 0.0;
      accumFb_idx_1 = 0.0;
      for (int j = 0; j < 5; j++) {
        double accum_idx_1_tmp;
        loc = this->storeLoc - this->del[j];
        dv[0] = loc;
        u0 = 0;
        if (loc < 1.0) {
          u0 = 1;
        }

        if (0 <= u0 - 1) {
          dv[0] = loc + 153600.0;
        }

        loc = this->dLine[static_cast<int>(dv[0]) - 1];
        accum_idx_0 += loc * this->level[j] * std::sqrt(1.0 - this->pan[j]);
        accum_idx_1_tmp = this->dLine[static_cast<int>(dv[0]) + 153599];
        accum_idx_1 += accum_idx_1_tmp * this->level[j] * std::sqrt(this->pan[j]);
        b = this->fb[j];
        accumFb_idx_0 += loc * b;
        accumFb_idx_1 += accum_idx_1_tmp * b;
      }

      u0 = static_cast<int>(this->storeLoc);
      this->dLine[u0 - 1] = x[i] + accumFb_idx_0;
      this->dLine[u0 + 153599] = x[i + x.size(0)] + accumFb_idx_1;
      this->storeLoc++;
      dv[0] = this->storeLoc;
      u0 = 0;
      if (this->storeLoc > 153600.0) {
        u0 = 1;
      }

      if (0 <= u0 - 1) {
        dv[0] = 1.0;
      }

      this->storeLoc = dv[0];
      b = this->mix;
      loc = 1.0 - this->mix;
      y[i] = accum_idx_0 * b + x[i] * loc;
      y[i + y.size(0)] = accum_idx_1 * b + x[i + x.size(0)] * loc;
    }
  }
}

void derivedAudioPlugin::reset()
{
  this->fs = this->getSampleRate();
  this->update();
}

void derivedAudioPlugin::setSampleRateForReset(double rate)
{
  this->PrivateSampleRate = rate;
}

void derivedAudioPlugin::set_fb16(double val)
{
  this->fb16 = val;
  this->update();
}

void derivedAudioPlugin::set_fb16dot(double val)
{
  this->fb16dot = val;
  this->update();
}

void derivedAudioPlugin::set_fb4(double val)
{
  this->fb4 = val;
  this->update();
}

void derivedAudioPlugin::set_fb8(double val)
{
  this->fb8 = val;
  this->update();
}

void derivedAudioPlugin::set_fb8dot(double val)
{
  this->fb8dot = val;
  this->update();
}

void derivedAudioPlugin::set_level16(double val)
{
  this->level16 = val;
  this->update();
}

void derivedAudioPlugin::set_level16dot(double val)
{
  this->level16dot = val;
  this->update();
}

void derivedAudioPlugin::set_level4(double val)
{
  this->level4 = val;
  this->update();
}

void derivedAudioPlugin::set_level8(double val)
{
  this->level8 = val;
  this->update();
}

void derivedAudioPlugin::set_level8dot(double val)
{
  this->level8dot = val;
  this->update();
}

void derivedAudioPlugin::set_mix_gui(double val)
{
  this->mix_gui = val;
  this->update();
}

void derivedAudioPlugin::set_pan16(double val)
{
  this->pan16 = val;
  this->update();
}

void derivedAudioPlugin::set_pan16dot(double val)
{
  this->pan16dot = val;
  this->update();
}

void derivedAudioPlugin::set_pan4(double val)
{
  this->pan4 = val;
  this->update();
}

void derivedAudioPlugin::set_pan8(double val)
{
  this->pan8 = val;
  this->update();
}

void derivedAudioPlugin::set_pan8dot(double val)
{
  this->pan8dot = val;
  this->update();
}

void derivedAudioPlugin::set_tempo(double val)
{
  this->tempo = val;
  this->update();
}

void derivedAudioPlugin::update()
{
  this->mix = this->mix_gui / 100.0;
  this->del[0] = rt_roundd_snf(60.0 * this->fs / this->tempo);
  this->del[1] = rt_roundd_snf(60.0 * this->fs / this->tempo / 2.0);
  this->del[2] = rt_roundd_snf(60.0 * this->fs / this->tempo / 4.0);
  this->del[3] = rt_roundd_snf(60.0 * this->fs / this->tempo / 4.0 * 1.5);
  this->del[4] = rt_roundd_snf(60.0 * this->fs / this->tempo / 2.0 * 1.5);
  this->level[0] = rt_powd_snf(10.0, this->level4 / 20.0);
  this->level[4] = rt_powd_snf(10.0, this->level8dot / 20.0);
  this->level[1] = rt_powd_snf(10.0, this->level8 / 20.0);
  this->level[3] = rt_powd_snf(10.0, this->level16dot / 20.0);
  this->level[2] = rt_powd_snf(10.0, this->level16 / 20.0);
  this->pan[0] = this->pan4 / 200.0 + 0.5;
  this->pan[4] = this->pan8dot / 200.0 + 0.5;
  this->pan[1] = this->pan8 / 200.0 + 0.5;
  this->pan[3] = this->pan16dot / 200.0 + 0.5;
  this->pan[2] = this->pan16 / 200.0 + 0.5;
  this->fb[0] = this->fb4 / 100.0;
  this->fb[4] = this->fb8dot / 100.0;
  this->fb[1] = this->fb8 / 100.0;
  this->fb[3] = this->fb16dot / 100.0;
  this->fb[2] = this->fb16 / 100.0;
}

static derivedAudioPlugin *getPluginInstance(myrupDelayStackData *SD)
{
  if (!SD->pd->plugin_not_empty) {
    SD->pd->plugin.init(SD);
    SD->pd->plugin_not_empty = true;
  }

  return &SD->pd->plugin;
}

static void getPluginInstance_init(myrupDelayStackData *SD)
{
  SD->pd->plugin_not_empty = false;
}

static double rt_powd_snf(double u0, double u1)
{
  double y;
  if (rtIsNaN(u0) || rtIsNaN(u1)) {
    y = rtNaN;
  } else {
    double d;
    double d1;
    d = std::abs(u0);
    d1 = std::abs(u1);
    if (rtIsInf(u1)) {
      if (d == 1.0) {
        y = 1.0;
      } else if (d > 1.0) {
        if (u1 > 0.0) {
          y = rtInf;
        } else {
          y = 0.0;
        }
      } else if (u1 > 0.0) {
        y = 0.0;
      } else {
        y = rtInf;
      }
    } else if (d1 == 0.0) {
      y = 1.0;
    } else if (d1 == 1.0) {
      if (u1 > 0.0) {
        y = u0;
      } else {
        y = 1.0 / u0;
      }
    } else if (u1 == 2.0) {
      y = u0 * u0;
    } else if ((u1 == 0.5) && (u0 >= 0.0)) {
      y = std::sqrt(u0);
    } else if ((u0 < 0.0) && (u1 > std::floor(u1))) {
      y = rtNaN;
    } else {
      y = std::pow(u0, u1);
    }
  }

  return y;
}

static double rt_roundd_snf(double u)
{
  double y;
  if (std::abs(u) < 4.503599627370496E+15) {
    if (u >= 0.5) {
      y = std::floor(u + 0.5);
    } else if (u > -0.5) {
      y = u * 0.0;
    } else {
      y = std::ceil(u - 0.5);
    }
  } else {
    y = u;
  }

  return y;
}

derivedAudioPlugin::~derivedAudioPlugin()
{
}

derivedAudioPlugin::derivedAudioPlugin()
{
}

void createPluginInstance(myrupDelayStackData *SD, unsigned long thisPtr)
{
  if (!SD->pd->thisPtr_not_empty) {
    SD->pd->thisPtr = thisPtr;
    SD->pd->thisPtr_not_empty = true;
  }

  getPluginInstance(SD);
}

int getLatencyInSamplesCImpl(myrupDelayStackData *SD)
{
  derivedAudioPlugin *plugin;
  plugin = getPluginInstance(SD);
  return plugin->getLatencyInSamplesInt32();
}

void myrupDelay_initialize(myrupDelayStackData *SD)
{
  SD->pd->thisPtr_not_empty = false;
  getPluginInstance_init(SD);
}

void myrupDelay_terminate()
{
  // (no terminate code required)
}

void onParamChangeCImpl(myrupDelayStackData *SD, int paramIdx, double value)
{
  derivedAudioPlugin *plugin;
  plugin = getPluginInstance(SD);
  switch (paramIdx) {
   case 0:
    plugin->set_tempo(value);
    break;

   case 1:
    plugin->set_mix_gui(value);
    break;

   case 2:
    plugin->set_level4(value);
    break;

   case 3:
    plugin->set_level8dot(value);
    break;

   case 4:
    plugin->set_level8(value);
    break;

   case 5:
    plugin->set_level16dot(value);
    break;

   case 6:
    plugin->set_level16(value);
    break;

   case 7:
    plugin->set_pan4(value);
    break;

   case 8:
    plugin->set_pan8dot(value);
    break;

   case 9:
    plugin->set_pan8(value);
    break;

   case 10:
    plugin->set_pan16dot(value);
    break;

   case 11:
    plugin->set_pan16(value);
    break;

   case 12:
    plugin->set_fb4(value);
    break;

   case 13:
    plugin->set_fb8dot(value);
    break;

   case 14:
    plugin->set_fb8(value);
    break;

   case 15:
    plugin->set_fb16dot(value);
    break;

   case 16:
    plugin->set_fb16(value);
    break;

   case 17:
    plugin->bypass = (value != 0.0);
    break;
  }
}

void processEntryPoint(myrupDelayStackData *SD, double samplesPerFrame, const
  double i1_data[], const int i1_size[1], const double i2_data[], const int
  i2_size[1], double o1_data[], int o1_size[1], double o2_data[], int o2_size[1])
{
  derivedAudioPlugin *plugin;
  coder::array<double, 2U> i1;
  coder::array<double, 2U> t1;
  int i;
  int loop_ub;
  plugin = getPluginInstance(SD);
  i1.set_size(i1_size[0], 2);
  loop_ub = i1_size[0];
  for (i = 0; i < loop_ub; i++) {
    i1[i] = i1_data[i];
  }

  loop_ub = i2_size[0];
  for (i = 0; i < loop_ub; i++) {
    i1[i + i1.size(0)] = i2_data[i];
  }

  plugin->process(i1, t1);
  if (1.0 > samplesPerFrame) {
    loop_ub = 0;
  } else {
    loop_ub = static_cast<int>(samplesPerFrame);
  }

  o1_size[0] = loop_ub;
  for (i = 0; i < loop_ub; i++) {
    o1_data[i] = t1[i];
  }

  if (1.0 > samplesPerFrame) {
    loop_ub = 0;
  } else {
    loop_ub = static_cast<int>(samplesPerFrame);
  }

  o2_size[0] = loop_ub;
  for (i = 0; i < loop_ub; i++) {
    o2_data[i] = t1[i + t1.size(0)];
  }
}

void resetCImpl(myrupDelayStackData *SD, double rate)
{
  derivedAudioPlugin *plugin;
  plugin = getPluginInstance(SD);
  plugin->setSampleRateForReset(rate);
  plugin->reset();
}

// End of code generation (myrupDelay.cpp)
