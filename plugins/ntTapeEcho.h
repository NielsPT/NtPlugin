#pragma once

#include "lib/Biquad.h"
#include "lib/Plugin.h"
#include "lib/SoftClip.h"
#include "lib/Stereo.h"
#include "lib/utils.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>

enum SubDev : int {
  half,
  fourth,
  eighth_dot,
  eighth,
  sixteenth_dot,
  sixteenth,
};
// 192 kHz * 2 seconds * 8 x oversampling * 2 stores pr sample * 2 for mod (the
// last doesn't need to be that big).
// TODO: Calculate space for mod.
// TODO: Dynamic alocation of delayline.
constexpr int delayLineLength = 192e3 * 2 * 8 * 2 * 2;

template <typename signal_t>
struct ntTapeEcho : public NtFx::NtPlugin<signal_t> {
  float fs;
  signal_t tGui             = 0.5;
  signal_t fb_percent       = 20;
  signal_t modFreq          = 1.0;
  signal_t modPhase         = 0.0;
  signal_t clipG_db         = 0.0;
  signal_t mix_percent      = 100.0;
  signal_t tOffset          = 0.0;
  signal_t modDepth_percent = 0.1;
  signal_t noise_db         = -100;
  bool sync                 = false;
  bool mod                  = true;
  bool clip                 = true;
  bool bypass               = false;
  SubDev subDev             = SubDev::fourth;
  signal_t tempoScale       = 1;
  size_t nGlide             = 4;
  NtFx::Biquad::Settings<signal_t> hpfSettings;
  NtFx::Biquad::Settings<signal_t> lpfSettings;

  NtFx::Biquad::Coeffs5<signal_t> hpfCoeffs;
  NtFx::Biquad::Coeffs5<signal_t> lpfCoeffs;
  NtFx::Biquad::StereoState<signal_t> hpfState;
  NtFx::Biquad::StereoState<signal_t> lpfState;
  NtFx::Stereo<signal_t> fbState;
  std::array<NtFx::Stereo<signal_t>, delayLineLength> delayLine;
  signal_t fb_lin         = 0.2;
  signal_t noise_lin      = 0;
  size_t nDelayGui        = 24000;
  size_t nDelayGlided     = 24000;
  size_t iStore           = 0;
  signal_t aClip_lin      = 1;
  size_t timeCounter      = 0;
  size_t nOffset          = 0;
  signal_t mix_lin        = 1;
  signal_t modDepth       = 0.1;
  signal_t thetaMod       = 0;
  signal_t thetaModOffset = 0;
  int glideCount          = 0;
  std::array<signal_t, 3> softClipCoeffs;

  ntTapeEcho() {
    this->primaryKnobs = {
      { &this->tGui, "Time", " s", 0.02, 2 },
      { &this->fb_percent, "Feedback", " %", 0, 200 },
      { &this->clipG_db, "Drive", " dB", -20, 20 },
      { &this->hpfSettings.fc_hz, "HPF", " Hz", 20, 2000 },
      { &this->lpfSettings.fc_hz, "LPF", " Hz", 200, 20000 },
    };
    this->primaryKnobs[3].setLogScale();
    this->primaryKnobs[4].setLogScale();
    this->secondaryKnobs = {
      { &this->hpfSettings.q, "Q_HP", "", 0.5, 2, 1 },
      { &this->lpfSettings.q, "Q_LP", "", 0.5, 2, 1 },
      { &this->modFreq, "Mod_Freq", " Hz", 0.1, 10 },
      { &this->modDepth_percent, "Mod_Depth", " %", 0.1, 10 },
      { &this->modPhase, "Mod_Phase", "deg", 0, 180 },
      { &this->tOffset, "Offset", " ms", 0, 50 },
      // TODO: proper glide parameter.
      // { &this->nGlide, "Glide_Speed", "", 0, 10 },
      { &this->noise_db, "Noise", " dB", -100, 0 },
      { &this->mix_percent, "Dry_Mix", " %", 0, 100 },
    };

    this->secondaryKnobs[0].setLogScale();
    this->secondaryKnobs[1].setLogScale();
    this->secondaryKnobs[2].setLogScale();
    this->secondaryKnobs[3].setLogScale();
    this->secondaryKnobs[6].setLogScale();
    this->dropdowns = {
      {
          .p_val = (int*)&this->subDev,
          .name = "Subdevision",
          .options = {
              "half",
              "fourth",
              "eighth_dot",
              "eighth",
              "sixteenth_dot",
              "sixteenth",
          }, 
          .defaultIdx = 0,
      },
    };
    this->toggles = {
      { &this->sync, "Sync" },
      { &this->mod, "Mod" },
      { &this->clip, "Output drive" },
      { &this->bypass, "Bypass" },
    };
    this->guiSpec.meters    = { { "IN" }, { "OUT", .hasScale = true } };
    this->lpfSettings.shape = NtFx::Biquad::Shape::lpf;
    this->hpfSettings.shape = NtFx::Biquad::Shape::hpf;
    this->lpfSettings.fc_hz = 20e3;
    this->hpfSettings.fc_hz = 20;
    this->softClipCoeffs    = NtFx::calculateSoftClipCoeffs<signal_t, 2>();
    this->updateDefaults();
  }
  virtual NtFx::Stereo<signal_t> processSample(
      NtFx::Stereo<signal_t> x) noexcept override {
    auto xNoisy = x + NtFx::rand<signal_t>() * this->noise_lin;
    NtFx::ensureFinite<NtFx::Stereo<signal_t>>(xNoisy);
    NtFx::ensureFinite<NtFx::Stereo<signal_t>>(this->fbState);
    this->iStore++;
    if (this->iStore >= delayLineLength) { this->iStore = 0; }
    this->delayLine[this->iStore] = xNoisy + this->fb_lin * this->fbState;

    int nModL = this->nDelayGlided;
    int nModR = this->nDelayGlided + this->nOffset;
    if (this->mod) {
      auto modSawL = NtFx::saw(this->thetaMod * this->timeCounter);
      auto modSawR =
          NtFx::saw(this->thetaMod * this->timeCounter + this->thetaModOffset);
      nModL = std::round(modSawL * this->nDelayGlided * this->modDepth)
          + this->nDelayGlided;
      nModR = std::round(modSawR * this->nDelayGlided * this->modDepth)
          + this->nDelayGlided + this->nOffset;
      this->timeCounter++;
    }

    int iLoadL = this->iStore - nModL;
    if (iLoadL < 0) { iLoadL += delayLineLength; }
    int iLoadR = this->iStore - nModR;
    if (iLoadR < 0) { iLoadR += delayLineLength; }
    NtFx::Stereo<signal_t> yDelay = {
      this->delayLine[iLoadL].l,
      this->delayLine[iLoadR].r,
    };
    auto yFbClip =
        NtFx::softClip3rdStereo(yDelay * this->aClip_lin) / aClip_lin;
    NtFx::ensureFinite<NtFx::Stereo<signal_t>>(yFbClip);
    auto yHp =
        NtFx::Biquad::biQuad5Stereo(&this->hpfCoeffs, &this->hpfState, yFbClip);
    auto yLp =
        NtFx::Biquad::biQuad5Stereo(&this->lpfCoeffs, &this->lpfState, yHp);

    // TODO: This sounds like shit. Make a proper glider.
    if (this->nGlide == 0) {
      this->nDelayGlided = this->nDelayGui;
    } else {
      this->glideCount--;
      if (this->glideCount >= 0) {
        if (this->nDelayGui > this->nDelayGlided) {
          this->nDelayGlided++;
        } else if (this->nDelayGui < this->nDelayGlided) {
          this->nDelayGlided--;
        }
        this->glideCount = this->nGlide;
      }
    }
    this->fbState = yLp;
    auto yOutClip = yLp;
    if (this->clip) {
      yOutClip = NtFx::softClip5thStereo(this->softClipCoeffs, yLp);
    }
    auto y = (1 - this->mix_lin) * x + this->mix_lin * yOutClip;
    this->template updatePeakLevel<0>(x);
    if (this->bypass) {
      this->template updatePeakLevel<1>(x);
      return x;
    }
    this->template updatePeakLevel<1>(y);
    return y;
  }
  virtual void updateCoeffs() noexcept override {
    this->hpfCoeffs = NtFx::Biquad::calcCoeffs5(this->hpfSettings, this->fs);
    this->lpfCoeffs = NtFx::Biquad::calcCoeffs5(this->lpfSettings, this->fs);
    this->nOffset   = std::round(this->tOffset / 1000 * this->fs);
    this->aClip_lin = NtFx::invDb(this->clipG_db);
    this->mix_lin   = this->mix_percent / 100;
    this->fb_lin    = this->fb_percent / 100;
    this->noise_lin = NtFx::invDb(this->noise_db);
    this->modDepth  = this->modDepth_percent / 100;
    switch (this->subDev) {
    case SubDev::half:
      this->tempoScale = 2;
      break;
    case SubDev::fourth:
      this->tempoScale = 1;
      break;
    case SubDev::eighth_dot:
      this->tempoScale = 0.5 * 1.5;
      break;
    case SubDev::eighth:
      this->tempoScale = 0.5;
      break;
    case SubDev::sixteenth_dot:
      this->tempoScale = 0.25 * 1.5;
      break;
    case SubDev::sixteenth:
      this->tempoScale = 0.25;
      break;
    }
    this->thetaMod       = 2.0 * M_PI * this->modFreq / this->fs;
    this->thetaModOffset = this->modPhase * M_PI / 180;
    this->glideCount     = this->nGlide;
    this->onTempoChanged();
  }
  virtual void reset(int fs) noexcept override {
    this->fs = fs;
    std::fill(this->delayLine.begin(), this->delayLine.end(), 0);
    this->updateCoeffs();
  }
  virtual void onTempoChanged() noexcept override {
    if (this->sync && this->tempo) {
      this->tGui                     = 60 / this->tempo * this->tempoScale;
      this->primaryKnobs[0].isActive = false;
    } else {
      this->primaryKnobs[0].isActive = true;
    }
    this->uiNeedsUpdate = true;
    this->nDelayGui     = std::round(this->tGui * this->fs);
  }
};
