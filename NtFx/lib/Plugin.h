#pragma once
#include <string>
#include <vector>

namespace NtFx {
// TODO: general parameter class.
template <typename signal_t>
struct FloatParameterSpec {
  signal_t* p_val;
  std::string name;
  std::string suffix;
  signal_t minVal { 0.0 };
  signal_t maxVal { 1.0 };
  signal_t defaultVal { 0.5 };
  signal_t skew { 0.0 };
};

struct BoolParameterSpec {
  bool* p_val;
  std::string name;
  bool defaultVal { false };
};

template <typename signal_t>
struct Plugin {
  int fs = 44100;
  std::vector<FloatParameterSpec<signal_t>> floatParameters;
  std::vector<FloatParameterSpec<signal_t>> floatParametersSmall;
  std::vector<BoolParameterSpec> boolParameters;

  bool* getBoolParamByName(std::string name) {
    for (auto param : this->boolParameters) {
      if (param.name == name) { return param.p_val; }
    }
    return nullptr;
  }

  signal_t* getFloatParamByName(std::string name) {
    for (auto param : this->floatParameters) {
      if (param.name == name) { return param.p_val; }
    }
    for (auto param : this->floatParametersSmall) {
      if (param.name == name) { return param.p_val; }
    }
    return nullptr;
  }
};
}