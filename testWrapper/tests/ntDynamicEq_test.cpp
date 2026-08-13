#include "lib/Biquad.h"
#include "lib/ComponentTest.h"
#include "plugins/ntDynamicEq.h"

int main() {
  auto set    = NtFx::ComponentTestSet(std::string(testFileBaseName(__FILE__)));
  auto bypass = ntDynamicEq();
  bypass.bypassEnable = true;
  NTFX_ADD_TEST(set, bypass, "impulse");
  auto defaults = ntDynamicEq();
  NTFX_ADD_TEST(set, defaults, "impulse");
  auto staticReduction = ntDynamicEq();
  for (size_t i = 0; i < Bands::n; i++) {
    staticReduction.bands[i].settings.shape   = NtFx::Biquad::Shape::bell;
    staticReduction.bands[i].settings.gain_db = -12;
    staticReduction.bands[i].settings.q       = 2;
  }
  NTFX_ADD_TEST(set, staticReduction, "impulse");
  auto staticBoost = ntDynamicEq();
  for (size_t i = 0; i < Bands::n; i++) {
    staticBoost.bands[i].settings.gain_db = 12;
    staticBoost.bands[i].settings.q       = 1;
  }
  NTFX_ADD_TEST(set, staticBoost, "impulse");
  return set.runAllTests();
}
