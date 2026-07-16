#include "lib/ComponentTest.h"
#include "plugins/ntDynamicEq.h"

NTFX_TEST_BEGIN

NTFX_TEST() {
  auto bypass         = ntDynamicEq();
  bypass.bypassEnable = true;
  NTFX_ADD_TEST(bypass, "impulse");
  auto defaults = ntDynamicEq();
  NTFX_ADD_TEST(defaults, "impulse");
  auto staticReduction = ntDynamicEq();
  for (size_t i = 0; i < Bands::n; i++) {
    staticReduction.bands[i].settings.gain_db = -12;
    staticReduction.bands[i].settings.q       = 2;
  }
  NTFX_ADD_TEST(staticReduction, "impulse");
  auto staticBoost = ntDynamicEq();
  for (size_t i = 0; i < Bands::n; i++) {
    staticBoost.bands[i].settings.gain_db = 12;
    staticBoost.bands[i].settings.q       = 1;
  }
  NTFX_ADD_TEST(staticBoost, "impulse");
  return NTFX_RUN_TESTS();
}
