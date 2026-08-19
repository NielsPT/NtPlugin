#include "lib/ComponentTest.h"
#include "plugins/ntChorus.h"
#include <memory>

int main() {
  auto set = NtFx::ComponentTestSet(std::string(testFileBaseName(__FILE__)));

  // Chorus with constructor defaults
  auto chorusDefaultsStorage = std::make_unique<ntChorus>();
  auto* chorusDefaults       = chorusDefaultsStorage.get();
  NTFX_ADD_TEST_PTR(set, chorusDefaults, "impulse");

  // Chorus bypass
  auto chorusBypassStorage   = std::make_unique<ntChorus>();
  auto* chorusBypass         = chorusBypassStorage.get();
  chorusBypass->bypassEnable = true;
  NTFX_ADD_TEST_PTR(set, chorusBypass, "impulse");

  // Chorus with fast modulation
  auto chorusFastStorage       = std::make_unique<ntChorus>();
  auto* chorusFast             = chorusFastStorage.get();
  chorusFast->dlMod.fMod_hz.ui = 2;
  chorusFast->dlWet.t_ms.ui    = 15;
  chorusFast->dlMod.depth_p    = 30;
  NTFX_ADD_TEST_PTR(set, chorusFast, "linearSweep");

  // Chorus with slow modulation
  auto chorusSlowStorage       = std::make_unique<ntChorus>();
  auto* chorusSlow             = chorusSlowStorage.get();
  chorusSlow->dlMod.fMod_hz.ui = 0.1;
  chorusSlow->dlWet.t_ms.ui    = 20;
  chorusSlow->dlMod.depth_p    = 10;
  NTFX_ADD_TEST_PTR(set, chorusSlow, "linearSweep");

  // Chorus with varied phase offset
  auto chorusPhaseStorage         = std::make_unique<ntChorus>();
  auto* chorusPhase               = chorusPhaseStorage.get();
  chorusPhase->dlMod.fMod_hz.ui   = 1;
  chorusPhase->dlWet.t_ms.ui      = 10;
  chorusPhase->dlMod.depth_p      = 20;
  chorusPhase->dlMod.phaseMod_deg = 45;
  NTFX_ADD_TEST_PTR(set, chorusPhase, "linearSweep");

  return set.runAllTests();
}
