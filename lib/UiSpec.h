#pragma once

#include <string>
#include <vector>

namespace NtFx {
// TODO: general parameter class.
// TODO: IDEA: Knob groups.
// - Each group takes up an area in the primary knob grid.
// - Each group can contain a number of primary and secondary knobs. Primary are placed
// on a row in the top of the group, secondary in a grid below.
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

constexpr int nMetersMax = 8;
struct MeterSpec {
  std::string name { "" };
  float minVal_db { -45.0 };
  bool invert { false };
  bool hasScale { false };
};

struct GuiSpec {
  bool includeMeters { true };
  bool includeTitleBar { true };
  bool includeSecondaryKnobs { true };
  uint32_t backgroundColour { 0xFF001100 };
  int defaultWindowWidth { 1000 };
  int maxRows { 3 };
  int maxColumns { 6 };
  int defaultFontSize { 16 };
  float labelHeight { 20 };
  float toggleHeight { 75 };
  float knobHeight { 200 };
  float secondaryKnobWidth { 75 };
  float secondaryKnobHeight { 115 };
  float titleBarAreaHeight { 22 };
  int meterHeight_dots { 14 };
  float meterRefreshRate_hz { 30 };
  std::vector<MeterSpec> meters;
};

struct DropDownSpec {
  std::string name;
  std::vector<std::string> options;
  int defaultIdx;
};

struct TitleBarSpec {
  std::string name;
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