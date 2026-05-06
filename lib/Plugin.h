#pragma once

/**
 * @file Plugin.h
 * @author Niels Thøgersen (niels.thoegersen@gmail.com)
 * @brief Base class for NTplugin framework.
 *
 * @copyright Copyright (c) 2026
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Affero General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option) any
 * later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "lib/Component.h"
#include "lib/RmsSensor.h"
#include "lib/Stereo.h"
#include "lib/UiSpec.h"
#include "lib/utils.h"

#include <array>
#include <cstddef>
#include <string>
#include <type_traits>
#include <vector>

namespace NtFx {

/**
 * @brief True virtual base class for NtPlugin. Inherit from this in order to
 * make a stereo audio plugin with auto-genrated UI.
 *
 * @tparam signal_t Basic datatype for audio signal.
 */
template <typename signal_t>
  requires std::is_floating_point_v<signal_t>
struct NtPlugin : public Component<Stereo<signal_t>> {
  /**
   * @brief Array of rms sensors for RMS meter.
   *
   */
  std::array<LongRmsSensorStereo<signal_t, 250, 192>, 2> xRms;

  /**
   * @brief Peak level to be displayed in the meters.
   */
  std::array<Stereo<signal_t>, nMetersMax> peakLevels;

  /**
   * @brief Specification for UI. Modify this to change the look of your
   * plugin.
   */
  UiSpec uiSpec;

  /**
   * @brief vector of primary knobs. Add your number paramters to this to
   * display them in the UI.
   */
  std::vector<KnobSpec<signal_t>> primaryKnobs;

  /**
   * @brief vector of secondary knobs.
   */
  std::vector<KnobSpec<signal_t>> secondaryKnobs;

  /**
   * @brief vector of DropDowns to be displayed
   */
  std::vector<DropDownSpec> dropdowns;

  /**
   * @brief vector of toggles to be displayed at the bottom of the UI.
   */
  std::vector<ToggleSpec> toggles;

  /**
   * @brief vector of groups of toggles to be placed in the right side area.
   *
   */
  std::vector<ToggleSetSpec> toggleSets;

  /**
   * @brief vector of radioButtons to be displayed at the right of the UI.
   */
  std::vector<RadioButtonSetSpec> radioButtons;

  /**
   * @brief List of all meters to be displayed in the UI.
   */
  std::vector<MeterSpec> meters = {
    { .name = "IN", .addRms = true },
    { .name = "OUT", .hasScale = true, .addRms = true },
  };

  /**
   * @brief Session tempo. Automatically updated by wrapper.
   */
  signal_t tempo = 0;

  /**
   * @brief Updated at base sample rate and can be used in process() for exenal
   * side chain input.
   */
  signal_t xSc;

  /**
   * @brief Set this to true if you want to force an update of the UI
   */
  bool uiNeedsUpdate = false;

  /**
   * @brief Called by the wrapper whenever the tempo changes.
   *
   */
  virtual void onTempoChanged() noexcept { }

  /**
   * @brief Used by the wrapper to get a pointer to a value based on the
   * corresponding knob's name.
   *
   * @param name Name of knob to get value for.
   */
  signal_t* getKnobValuePtr(std::string name) const noexcept {
    for (auto param : this->primaryKnobs) {
      if (param.name == name) { return param.p_val; }
    }
    for (auto param : this->secondaryKnobs) {
      if (param.name == name) { return param.p_val; }
    }
    return nullptr;
  }

  /**
   * @brief Used by the wrapper to get a pointer to a value based on the
   * corresponding toggle's name.
   *
   * @param name Name of toggle to get value for.
   */
  bool* getToggleValuePtr(std::string name) const noexcept {
    for (auto param : this->toggles) {
      if (param.name == name) { return param.p_val; }
    }
    return nullptr;
  }

  /**
   * @brief Used by the wrapper to get a pointer to a value based on the
   * corresponding drop down's name.
   *
   * @param name Name of drop down to get value for.
   */
  int* getDropDownValuePtr(std::string name) const noexcept {
    for (auto dropdown : this->dropdowns) {
      if (dropdown.name == name) { return dropdown.p_val; }
    }
    return nullptr;
  }

  void setParameterActive(std::string name, bool val) {
    for (auto& param : this->primaryKnobs) {
      if (param.name == name) {
        param.isActive      = val;
        this->uiNeedsUpdate = true;
        return;
      }
    }
    for (auto& param : this->secondaryKnobs) {
      if (param.name == name) {
        param.isActive      = val;
        this->uiNeedsUpdate = true;
        return;
      }
    }
    // TODO: isActive field for all param types. INCLUDES fixing editor.
    // for (auto& param : this->toggles) {
    //   if (param.name == name) {
    //     param.isActive      = false;
    //     this->uiNeedsUpdate = true;
    //     return;
    //   }
    // }
    // for (auto& param : this->dropdowns) {
    //   if (param.name == name) {
    //     param.isActive      = false;
    //     this->uiNeedsUpdate = true;
    //     return;
    //   }
    // }
    // for (auto& param : this->radioButtons) {
    //   if (param.name == name) {
    //     param.isActive      = false;
    //     this->uiNeedsUpdate = true;
    //     return;
    //   }
    // }
    // for (auto& param : this->toggleSets) {
    //   if (param.name == name) {
    //     param.isActive      = false;
    //     this->uiNeedsUpdate = true;
    //     return;
    //   }
    // }
  }

  void deactivateParameter(std::string name) {
    this->setParameterActive(name, false);
  }

  void activateParameter(std::string name) {
    this->setParameterActive(name, true);
  }

  /**
   * @brief Copies the values of all members in the derived plugin class to
   * their respective knobs and toggels. Should always be called as the last
   * thing in the constructor of the derived plugin class.
   *
   */
  constexpr void updateDefaults() noexcept {
    for (auto& k : this->primaryKnobs) {
      if (k.p_val) { k._defaultVal = *k.p_val; }
      if (k.logScale) { k.setLogScale(); }
    }
    for (auto& k : this->secondaryKnobs) {
      if (k.p_val) { k._defaultVal = *k.p_val; }
      if (k.logScale) { k.setLogScale(); }
    }
    for (auto& t : this->toggles) {
      if (t.p_val) { t._defaultVal = *t.p_val; }
    }
    for (auto& t : this->dropdowns) {
      if (t.p_val) { t._defaultVal = *t.p_val; }
    }
    for (auto& t : this->radioButtons) {
      if (t.p_val) { t._defaultVal = *t.p_val; }
    }
    for (auto& s : this->toggleSets) {
      for (auto& t : s.toggles) {
        if (t.p_val) { t._defaultVal = *t.p_val; }
      }
    }
  }

  /**
   * @brief Updates the peak level of a meter. This is a template so that we can
   * do range checking at compile time.
   *
   * @tparam idx Index of meter to update.
   * @tparam invert If true, the smallest value will be selected.
   * @param val New value. If val has a larger magnitude than the current peak
   value, the current peak value will be overridden.
   * @return
   */
  template <size_t idx, bool invert = false>
  NtFx::Stereo<signal_t> updatePeakLevel(NtFx::Stereo<signal_t> val) noexcept {
    if constexpr (idx >= nMetersMax) {
      static_assert(false, "Meter index is out of bounds.");
    }
    if (invert) {
      if (val < this->peakLevels[idx]) { this->peakLevels[idx] = val; }
    } else {
      if (val > this->peakLevels[idx]) { this->peakLevels[idx] = val; }
    }
    return val;
  }

  /**
   * @brief Get and reset peak level for specified meter.
   *
   * @param idx
   * @return NtFx::Stereo<signal_t> Highest signal level since last call.
   */
  NtFx::Stereo<signal_t> getAndResetPeakLevel(size_t idx) noexcept {
    signal_t def = signal_t(0);
    if (idx >= this->meters.size()) { return def; }
    if (this->meters[idx].invert) { def = signal_t(1); }
    ensureFinite(this->peakLevels[idx]);
    NtFx::Stereo<signal_t> tmp = this->peakLevels[idx];
    this->peakLevels[idx]      = signal_t(def);
    return tmp;
  }

  /**
   * @brief Get the RMS level of meter with index idx. RMS level is calculated
   * from the peak level.
   *
   * @param idx Index of meter to get RMS level for.
   * @return NtFx::Stereo<signal_t> RMS level.
   */
  NtFx::Stereo<signal_t> getRms(size_t idx) noexcept {
    if (idx >= 2) { return signal_t(0); }
    return this->xRms[idx].getRms();
  }
};
}