#include "lib/ComponentTest.h"
#include "plugins/ntGate.h"
#include <memory>

int main() {
  auto set = NtFx::ComponentTestSet(std::string(testFileBaseName(__FILE__)));

  // Gate with constructor defaults
  auto gateDefaultsStorage = std::make_unique<ntGate>();
  auto& gateDefaults       = *gateDefaultsStorage;
  NTFX_ADD_TEST(set, gateDefaults, "impulse");

  // Gate bypass
  auto gateBypassStorage  = std::make_unique<ntGate>();
  auto& gateBypass        = *gateBypassStorage;
  gateBypass.bypassEnable = true;
  NTFX_ADD_TEST(set, gateBypass, "impulse");

  // Gate with internal sidechain
  auto gateInternalStorage           = std::make_unique<ntGate>();
  auto& gateInternal                 = *gateInternalStorage;
  gateInternal.scMode                = ScMode::internal;
  gateInternal.sc.settings.thresh_db = -24;
  gateInternal.sc.settings.range_db  = -60;
  NTFX_ADD_TEST(set, gateInternal, "dynamic_alternating");

  // Gate with aggressive threshold
  auto gateAggressiveStorage           = std::make_unique<ntGate>();
  auto& gateAggressive                 = *gateAggressiveStorage;
  gateAggressive.scMode                = ScMode::internal;
  gateAggressive.sc.settings.thresh_db = -12;
  gateAggressive.sc.settings.range_db  = -40;
  NTFX_ADD_TEST(set, gateAggressive, "dynamic_alternating");

  // Gate with gentle threshold
  auto gateGentleStorage           = std::make_unique<ntGate>();
  auto& gateGentle                 = *gateGentleStorage;
  gateGentle.scMode                = ScMode::internal;
  gateGentle.sc.settings.thresh_db = -36;
  gateGentle.sc.settings.range_db  = -20;
  NTFX_ADD_TEST(set, gateGentle, "dynamic_alternating");

  // Gate with lookahead enabled
  auto gateLookaheadStorage           = std::make_unique<ntGate>();
  auto& gateLookahead                 = *gateLookaheadStorage;
  gateLookahead.scMode                = ScMode::internal;
  gateLookahead.lookaheadEnable       = true;
  gateLookahead.sc.settings.thresh_db = -24;
  NTFX_ADD_TEST(set, gateLookahead, "dynamic_alternating");

  return set.runAllTests();
}
