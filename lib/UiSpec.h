#pragma once

/**
 * @file UiSpec.h
 * @author Niels Thøgersen (niels.thoegersen@gmail.com)
 * @brief Specification for plugin UI.
 *
 * @copyright Copyright (c) 2026
 *
 * Copyright (C) 2026 Niels Thøgersen, NTlyd
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

#include "gcem.hpp"
#include "lib/Audio.h"
#include "lib/utils.h"
#include <cstdint>
#include <string>
#include <vector>

namespace NtFx {
/**
 * @brief Specification for a knob.
 *
 * @tparam signal_t
 */
struct KnobSpec {
  signal_t* p_val { nullptr }; ///< Pointer to value the knob represents.
  std::string name { "" };     ///< Display name of knob.
  std::string suffix { "" };   ///< Added to the end of value label for knob.
  signal_t minVal { 0.0 };     ///< Starting level of knob.
  signal_t maxVal { 1.0 };     ///< End level of knob,
  signal_t midPoint { 0.0 }; ///< Sets the middel of the knob. 0 for don't care.
  bool logScale { false };   ///< Call setLogScale at construction.
  bool isActive { true };    ///< Gray out knob and make it unresponsive.
  signal_t _defaultVal;      ///< Default value. Set by updateDefaults().
  void setLogScale() {       ///< Sets midPoint for logarithmic scale.
    this->midPoint = gcem::sqrt(this->minVal * this->maxVal);
  }
};

/**
 * @brief Spec for a group of knobs.
 *
 */
struct KnobGroupSpec {
  std::string name;
  std::vector<KnobSpec> primaryKnobs;
  std::vector<KnobSpec> secondaryKnobs;
};

/**
 * @brief Specification for toggles in UI.
 *
 */
struct ToggleSpec {
  bool* p_val; ///< Pointer to value the knob represents. Used to bind to UI.
  /** Name of knob, used for IDs and label in UI. */
  std::string name;
  bool _defaultVal { false };
};

static inline ToggleSpec makeTmpToggle(
    std::string paramType, std::string groupName, std::string paramName) {
  return { nullptr, mangleName(paramType, groupName, paramName) };
}

struct ToggleSetSpec {
  std::string name;
  std::vector<ToggleSpec> toggles;
};

constexpr int nMetersMax = 8; ///< Size of meter peak level array and thus max
                              ///< number of meters available.
/**
 * @brief Specification for meters.
 *
 */
struct MeterSpec {
  std::string name { "" };   ///< Name of meter, used for label and id.
  float maxVal_db { 0.0 };   ///< Top value of meter.
  float minVal_db { -36.0 }; ///< Bottom value of meter.
  bool invert { false };     ///< When true, the meter lights up top down.
  bool hasScale { false };   ///< Adds a text scale to the right of the meter.
  float decay_s { 0.25 };    ///< Decay of meter fall off in seconds.
  float hold_s { 2 };        ///< Meter Hold time in seconds.
  /** Adds RMS to meter. Currently works for the first two meters. RMS of input
   * will be added to meter 0 and RMS of output will be added to meter 1.*/
  bool addRms = false;
};

/**
 * @brief Specification for drop down in UI.
 *
 */
struct OptionsSpec {
  int* p_val { nullptr };
  std::string name; ///< Name of drop down, used for label and id.
  std::vector<std::string> options; ///< Vector of options in the drop down.
  int _defaultVal { 0 };            ///< Index of default option.
  int _id { 0 };
};

typedef OptionsSpec DropDownSpec;
typedef OptionsSpec RadioButtonSetSpec;

/**
 * @brief Specifies the layout of the title bar.
 *
 */
struct TitleBarSpec {
  std::vector<DropDownSpec> dropdowns {
    {
        nullptr,
        "UI Scale",
        { "50%", "75%", "100%", "125%", "150%", "175%", "200%" },
        2,
    },
    { nullptr, "Theme", { "Light", "Dark" }, 1 },
    {
        nullptr,
        "Oversampling",
        { "disable",
            // TODO: IIR oversampling
            // "iir_2x",
            // "iir_4x",
            // "iir_8x",
            "FIR 2X LQ",
            "FIR 4X LQ",
            "FIR 8X LQ",
            "FIR 2X HQ",
            "FIR 4X HQ",
            "FIR 8X HQ" },
        0,
    },
  };
};

/**
 * @brief General settings for to UI.
 *
 */
struct UiSpec {
  bool includeMeters { true };              ///< Add meters to the UI.
  bool includeTitleBar { true };            ///< Add title bar to UI.
  bool includeSecondaryKnobs { true };      ///< Enable row below the main grid.
  uint32_t foregroundColour { 0xFFFFFFFF }; ///< UI foreground colour in HEX.
  uint32_t backgroundColour { 0xFF000000 }; ///< UI background colour in HEX.
  int maxRows { 3 };    ///< The maximum number of rows in main knob grid.
  int maxColumns { 6 }; ///< The maximum number of columns in main knob grid.
  float defaultFontSize { 16 }; ///< Font size before scaling.
  float labelHeight { 20 };     ///< Height of all text labels in the UI.
  float toggleHeight { 45 }; ///< Height of toggle row at the bottom of the UI.
  float radioButtonHeight { 25 };     ///< Height of each separate radio button.
  float radioButtonAreaWidth { 125 }; ///< Width of the radio buttons ares.
  float knobWidth { 150 };            ///< Width of primary knobs.
  float knobHeight { 200 };           ///< Height or row of knobs in grid in UI.
  float secondaryKnobWidth { 75 };    ///< Width of secondary knobs in UI.
  float secondaryKnobHeight { 115 };  ///< Height of secondary knobs in UI.
  float titleBarHeight { 22 };        ///< Height of title bar in pixels.
  int meterHeight_dots { 12 };        ///< Number of dots in the meteres.
  int meterWidth { 35 };              ///< Width of each meter in pixels.
  float meterRefreshRate_hz { 50 };   ///< Refresh rate of meters in the UI.
  int groupWidth { 200 };
  int groupEvenColOffset { 40 };
  int groupKnobWidth { groupWidth / 2 };
  int groupKnobHeight { 120 };
  int groupPad { 20 };
};
} // namespace NtFx