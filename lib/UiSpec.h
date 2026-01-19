#pragma once

#include "lib/utils.h"
#include <string>
#include <vector>

namespace NtFx {
// TODO: Document all of this.
// TODO: general parameter class.
// TODO: IDEA: Knob groups.
// - Each group takes up an area in the primary knob grid.
// - Each group can contain a number of primary and secondary knobs. Primary are
// placed on a row in the top of the group, secondary in a grid below.
/**
 * @brief Specification for a knob.
 *
 * @tparam signal_t
 */
template <typename signal_t>
struct KnobSpec {
  /** Pointer to value the knob represents. Used to bind to UI.*/
  signal_t* p_val;
  /** Name of knob, used for IDs and label in UI. Can't contain ' '. '_' is
   * replaced with ' ' in UI lables. */
  std::string name;
  /** Added to the end of value label for knob. Eg. " dB" or " ms". */
  std::string suffix { "" };
  /** Starting level of knob. */
  signal_t minVal { 0.0 };
  /** End level of knob, */
  signal_t maxVal { 1.0 };
  /** Sets the value at the middel of the knob radius.*/
  signal_t midPoint { 0.0 };
  /** Sets midPoint for logarithmic scale. */
  NTFX_INLINE_MEMBER void setLogScale() {
    this->midPoint = std::sqrt(this->minVal * this->maxVal);
  }
  signal_t defaultVal { 0.0 };
};

/**
 * @brief Specification for toggles in UI.
 *
 */
struct ToggleSpec {
  /** Pointer to value the knob represents. Used to bind to UI.*/
  bool* p_val;
  /** Name of knob, used for IDs and label in UI. Can't contain ' '. '_' is
   * replaced with ' ' in UI lables. */
  std::string name;
  bool defaultVal { false };
};

/** Size of meter peak level array and thus max number of meters available. */
constexpr int nMetersMax = 8;
struct StereoMeterSpec {
  /** Name of meter, used for label and id. */
  std::string name { "" };
  /** Top value of meter. */
  float maxVal_db { 0.0 };
  /** Bottom value of meter. */
  float minVal_db { -36.0 };
  /** When true, the meter starts from maxVal and lights up downwords. */
  bool invert { false };
  /** Adds a text scale to the right of the meter. */
  bool hasScale { false };
  // /** Colour of meter foreground. */
  // uint32_t foregroundColour { 0xFF000000 };
};

/**
 * @brief General settings for to UI.
 *
 */
struct GuiSpec {
  /** Add meters to the UI. */
  bool includeMeters { true };
  /** Add titlebar to the top of the UI. This includes oversampling and display
   * scaling.
   */
  bool includeTitleBar { true };
  /** Enable row of smaller knobs below the main grid. */
  bool includeSecondaryKnobs { true };
  /** UI foreground colour in HEX format 0x[opacity][red][green][blue]. */
  uint32_t foregroundColour { 0xFFFFFFFF };
  /** UI background colour in HEX format 0x[opacity][red][green][blue]. */
  uint32_t backgroundColour { 0xFF000000 };
  /** Window width in pixels before scaling. */
  int defaultWindowWidth { 1000 };
  /** The maximum number of rows in main knob grid. */
  int maxRows { 3 };
  /** The maximum number of columns in main knob grid. */
  int maxColumns { 6 };
  /** Font size before scaling. */
  float defaultFontSize { 16 };
  /** Height of all text labels in the UI. */
  float labelHeight { 20 };
  /** Height of toggle row at the bottom of the UI. */
  float toggleHeight { 50 };
  /** Height or row of knobs in grid in UI. */
  float knobHeight { 200 };
  /** Width of secondary knobs in UI. */
  float secondaryKnobWidth { 75 };
  /** Height of secondary knobs in UI including text labels name and value. */
  float secondaryKnobHeight { 115 };
  /** Height of title bar in pixels before UI scaling. */
  float titleBarHeight { 22 };
  /** Number of dots in the meteres. */
  int meterHeight_dots { 12 };
  /** Width of each meter in pixels. */
  int meterWidth { 35 };
  /** Refresh rate of the timer that updates the meters in the UI. */
  float meterRefreshRate_hz { 30 };
  /** List of all meters to be displayed in the UI. */
  std::vector<StereoMeterSpec> meters;
};

/**
 * @brief Specification for drop down in UI.
 *
 */
struct DropDownSpec {
  /** Name of drop down, used for label and id. */
  std::string name;
  /** Vector of options in the drop down. */
  std::vector<std::string> options;
  /** Index of default option. */
  int defaultIdx;
};

/**
 * @brief Specifies the layout of the title bar.
 *
 */
struct TitleBarSpec {
  std::vector<DropDownSpec> dropDowns {
    {
        "UI Scale",
        {
            "50%",
            "75%",
            "100%",
            "125%",
            "150%",
            "175%",
            "200%",
        },
        2,
    },
    // TODO: Oversampling
    // {
    //     "Oversampling",
    //     {
    //         "disable",
    //         "iir_2x",
    //         "iir_4x",
    //         "iir_8x",
    //         "fir_2x_lq",
    //         "fir_4x_lq",
    //         "fir_8x_lq",
    //         "fir_2x_hq",
    //         "fir_4x_hq",
    //         "fir_8x_hq",
    //     },
    //     0,
    // },
  };
};
} // namespace NtFx