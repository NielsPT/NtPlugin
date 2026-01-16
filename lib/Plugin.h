#pragma once
#include <array>
#include <cstddef>
#include <string>
#include <vector>

#include "lib/Stereo.h"
#include "lib/UiSpec.h"
#include "lib/utils.h"

#ifndef NTFX_PLUGIN
  #error NTFX_PLUGIN is not defined.
#endif

namespace NtFx {

template <typename signal_t>
struct Plugin {
  float tempo = 120;
  int fs      = 44100;
  // TODO: Should be const. Can it be done?
  std::vector<KnobSpec<signal_t>> primaryKnobs;
  std::vector<KnobSpec<signal_t>> secondaryKnobs;
  std::vector<ToggleSpec> toggles;
  std::vector<MeterSpec> meterSpec;
  std::array<NtFx::Stereo<signal_t>, nMetersMax> peakLevels;
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

  /**
   * @brief Updates the peak level of a meter. This is a template so that we can do
   range checking at compile time.
   *
   * @tparam idx Index of meter to update.
   * @tparam invert If true, the smallest value will be selected.
   * @param val New value. If val has a larger magnitude than the current peak value,
   the current peak value will be overridden.
   * @return NTFX_INLINE_MEMBER
   */
  template <size_t idx, bool invert = false>
  NTFX_INLINE_MEMBER void updatePeakLevel(NtFx::Stereo<signal_t> val) noexcept {
    if constexpr (idx >= nMetersMax) {
      static_assert(false, "Meter index is out of bounds.");
    }
    if (invert) {
      if (val < this->peakLevels[idx]) { this->peakLevels[idx] = val; }
    } else {
      if (val > this->peakLevels[idx]) { this->peakLevels[idx] = val; }
    }
  }

  /**
   * @brief Get and reset peak level for specified meter.
   *
   * @param idx
   * @return NtFx::Stereo<signal_t> Highest signal level since last call.
   */
  NTFX_INLINE_MEMBER NtFx::Stereo<signal_t> getAndResetPeakLevel(size_t idx) noexcept {
    signal_t def = NTFX_SIG_T(0);
    if (idx >= this->meterSpec.size()) { return def; }
    if (this->meterSpec[idx].invert) { def = NTFX_SIG_T(1); }
    NtFx::Stereo<signal_t> tmp = this->peakLevels[idx];
    this->peakLevels[idx]      = NTFX_SIG_T(def);
    ensureFinite(tmp, def);
    return tmp;
  }
};
}