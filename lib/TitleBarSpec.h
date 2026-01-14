#pragma once

#include <string>
#include <vector>

#include "utils.h"

namespace NtFx {
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
            "200%",
        },
        2,
    },
    {
        "Oversampling",
        {
            "disable",
            "iir_2x",
            "iir_4x",
            "iir_8x",
            "fir_2x_lq",
            "fir_4x_lq",
            "fir_8x_lq",
            "fir_2x_hq",
            "fir_4x_hq",
            "fir_8x_hq",
        },
        0,
    },
  };
};
}
