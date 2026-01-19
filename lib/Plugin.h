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

/**
 * @brief True virtual base class for NtPlugin. Inherit from this in order to
 * make a stereo audio plugin with auto-genrated UI.
 *
 * @tparam signal_t Basic datatype for audio signal.
 */
template <typename signal_t>
struct NtPlugin {
  /** Session tempo. Automatically updated by wrapper. */
  float tempo = 0;
  /** vector of primary knobs. Add your number paramters to this to display them
   * in the UI. */
  std::vector<KnobSpec<signal_t>> primaryKnobs;
  /** vector of secondary knobs. */
  std::vector<KnobSpec<signal_t>> secondaryKnobs;
  /** vector of toggles to be displayed at the bottom of the UI. */
  std::vector<ToggleSpec> toggles;
  /** Peak level to be displayed in the meters. */
  std::array<NtFx::Stereo<signal_t>, nMetersMax> peakLevels;
  /** Specification for UI. Modify this to change the look of your plugin. */
  GuiSpec guiSpec;

  /**
   * @brief Called for every sample as audio is processed.
   *
   * @param x Stereo<signal_t> Input
   * @return Stereo<signal_t> output.
   */
  virtual NTFX_INLINE_MEMBER Stereo<signal_t> processSample(
      Stereo<signal_t> x) noexcept = 0;

  /**
   * @brief Called when ever a parameter (knob or toggle) changes. Update your
   * coefficients here.
   *
   */
  virtual NTFX_INLINE_MEMBER void updateCoeffs() noexcept = 0;

  /**
   * @brief Called when the plugin loads and when ever the samplerate or buffer
   * size changes.
   *
   * @param fs Sample rate. If you need it for calculateCoeffs, store it.
   */
  virtual NTFX_INLINE_MEMBER void reset(int fs) noexcept = 0;

  /**
   * @brief Used by the wrapper to get a pointer to a value based on the
   * corresponding knob's name.
   *
   * @param name Name of knob to get value for.
   * @return signal_t* Pointer to value.
   */
  template <typename T>
  T* getValuePtr(std::string name) const noexcept {
    for (auto param : this->primaryKnobs) {
      if (param.name == name) { return param.p_val; }
    }
    for (auto param : this->secondaryKnobs) {
      if (param.name == name) { return param.p_val; }
    }
    for (auto param : this->toggles) {
      if (param.name == name) { return param.p_val; }
    }
    return nullptr;
  }

  /**
   * @brief Copies the values of all members in the derived plugin class to
   * their respective knobs and toggels. Should always be called as the last
   * thing in the constructor of the derived plugin class.
   *
   */
  constexpr void updateDefaults() noexcept {
    for (auto& k : this->primaryKnobs) { k.defaultVal = *k.p_val; }
    for (auto& k : this->secondaryKnobs) { k.defaultVal = *k.p_val; }
    for (auto& t : this->toggles) { t.defaultVal = *t.p_val; }
  }

  /**
   * @brief Updates the peak level of a meter. This is a template so that we can
   * do range checking at compile time.
   *
   * @tparam idx Index of meter to update.
   * @tparam invert If true, the smallest value will be selected.
   * @param val New value. If val has a larger magnitude than the current peak
   value, the current peak value will be overridden.
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
  NTFX_INLINE_MEMBER NtFx::Stereo<signal_t> getAndResetPeakLevel(
      size_t idx) noexcept {
    signal_t def = NTFX_SIG_T(0);
    if (idx >= this->guiSpec.meters.size()) { return def; }
    if (this->guiSpec.meters[idx].invert) { def = NTFX_SIG_T(1); }
    NtFx::Stereo<signal_t> tmp = this->peakLevels[idx];
    this->peakLevels[idx]      = NTFX_SIG_T(def);
    ensureFinite(tmp, def);
    return tmp;
  }
};
}