#pragma once
#include <string>
#include <vector>

#include "Stereo.h"

#ifndef NTFX_PLUGIN
  #error NTFX_PLUGIN is not defined.
#endif

namespace NtFx {
// TODO: general parameter class.
template <typename signal_t>
struct KnobSpec {
  signal_t* p_val;
  std::string name;
  std::string suffix { "" };
  signal_t minVal { 0.0 };
  signal_t maxVal { 1.0 };
  signal_t defaultVal { 0.5 };
  signal_t skew { 0.0 };
};

struct ToggleSpec {
  bool* p_val;
  std::string name;
  bool defaultVal { false };
};

struct GuiSpec {
  int maxRows            = 3;
  int maxColumns         = 6;
  int defaultFontSize    = 16;
  int defaultWindowWidth = 1000;
};

enum MeterIdx {
  in  = 0,
  out = 1,
  gr  = 2,
  end = gr,
};

template <typename signal_t>
struct Plugin {
  float tempo = 120;
  int fs      = 44100;
  // TODO: Should be const. Can it be done?
  std::vector<KnobSpec<signal_t>> primaryKnobs;
  std::vector<KnobSpec<signal_t>> secondaryKnobs;
  std::vector<ToggleSpec> toggles;
  std::array<NtFx::Stereo<signal_t>, MeterIdx::end + 1> peakLevels;
  GuiSpec guiSpec;

  virtual NTFX_INLINE_MEMBER Stereo<signal_t> processSample(
      Stereo<signal_t> x) noexcept                        = 0;
  virtual NTFX_INLINE_MEMBER void updateCoeffs() noexcept = 0;
  virtual NTFX_INLINE_MEMBER void reset(int fs) noexcept  = 0;

  bool* getToggleValuePtr(std::string name) const noexcept {
    for (auto param : this->toggles) {
      if (param.name == name) { return param.p_val; }
    }
    return nullptr;
  }

  signal_t* getKnobValuePtr(std::string name) const noexcept {
    for (auto param : this->primaryKnobs) {
      if (param.name == name) { return param.p_val; }
    }
    for (auto param : this->secondaryKnobs) {
      if (param.name == name) { return param.p_val; }
    }
    return nullptr;
  }

  constexpr void updateDefaults() noexcept {
    for (auto& k : this->primaryKnobs) { k.defaultVal = *k.p_val; }
    for (auto& k : this->secondaryKnobs) { k.defaultVal = *k.p_val; }
    for (auto& t : this->toggles) { t.defaultVal = *t.p_val; }
  }

  NTFX_INLINE_MEMBER void updatePeakLevel(
      NtFx::Stereo<signal_t> val, size_t idx, bool invert = false) noexcept {
    auto tmp = val < this->peakLevels[idx];
    if ((!invert && !tmp) || (invert && tmp)) { this->peakLevels[idx] = val; }
  }

  NTFX_INLINE_MEMBER NtFx::Stereo<signal_t> getAndResetPeakLevel(size_t idx) noexcept {
    NtFx::Stereo<signal_t> tmp = this->peakLevels[idx];
    signal_t def               = NTFX_SIG_T(0);
    if (idx >= MeterIdx::end && idx == MeterIdx::gr) { def = NTFX_SIG_T(1); }
    this->peakLevels[idx] = NTFX_SIG_T(def);
    ensureFinite(tmp, def);
    return tmp;
  }
};
}