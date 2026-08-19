#include "lib/Biquad.h"
#include "lib/ComponentTest.h"
#include "plugins/ntDynamicEq.h"
#include <memory>

int main() {
  auto set = NtFx::ComponentTestSet(std::string(testFileBaseName(__FILE__)));
  auto bypassStorage  = std::make_unique<ntDynamicEq>();
  auto& bypass        = *bypassStorage;
  bypass.bypassEnable = true;
  NTFX_ADD_TEST(set, bypass, "impulse");
  auto defaultsStorage = std::make_unique<ntDynamicEq>();
  auto& defaults       = *defaultsStorage;
  NTFX_ADD_TEST(set, defaults, "impulse");
  auto staticReductionStorage = std::make_unique<ntDynamicEq>();
  auto& staticReduction       = *staticReductionStorage;
  for (size_t i = 0; i < Bands::n; i++) {
    staticReduction.bands[i].settings.shape   = NtFx::Biquad::Shape::bell;
    staticReduction.bands[i].settings.gain_db = -12;
    staticReduction.bands[i].settings.q       = 2;
  }
  NTFX_ADD_TEST(set, staticReduction, "impulse");
  auto staticBoostStorage = std::make_unique<ntDynamicEq>();
  auto& staticBoost       = *staticBoostStorage;
  for (size_t i = 0; i < Bands::n; i++) {
    staticBoost.bands[i].settings.gain_db = 12;
    staticBoost.bands[i].settings.q       = 1;
  }
  NTFX_ADD_TEST(set, staticBoost, "impulse");
  return set.runAllTests();
}
