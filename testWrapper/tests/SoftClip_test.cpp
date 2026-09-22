

#include "lib/ComponentTest.h"
#include "lib/SoftClip.h"
#include <memory>

int main() {
  auto set = NtFx::ComponentTestSet(std::string(testFileBaseName(__FILE__)));
  auto hardStorage = std::make_unique<NtFx::HardClip>();
  auto& hard       = *hardStorage;
  NTFX_ADD_TEST(set, hard, "linearSweep");
  auto hardPlus6Storage = std::make_unique<NtFx::HardClip>();
  auto& hardPlus6       = *hardPlus6Storage;
  hardPlus6.gain_db     = 6;
  NTFX_ADD_TEST(set, hardPlus6, "linearSweep");
  auto thirdStorage = std::make_unique<NtFx::SoftClip3>();
  auto& third       = *thirdStorage;
  NTFX_ADD_TEST(set, third, "linearSweep");
  auto fifthStorage = std::make_unique<NtFx::SoftClip5>();
  auto& fifth       = *fifthStorage;
  NTFX_ADD_TEST(set, fifth, "linearSweep");
  auto thirdAntiliasStorage  = std::make_unique<NtFx::SoftClip3AntiAlias>();
  auto& thirdAntiliasDefault = *thirdAntiliasStorage;
  NTFX_ADD_TEST(set, thirdAntiliasDefault, "linearSweep");
  NTFX_ADD_TEST(set, thirdAntiliasDefault, "impulse");
  auto thirdAntiliasOneBypassStorage =
      std::make_unique<NtFx::SoftClip3AntiAlias>();
  auto& thirdAntiliasOneBypass         = *thirdAntiliasOneBypassStorage;
  thirdAntiliasOneBypass.bypass2Enable = false;
  NTFX_ADD_TEST(set, thirdAntiliasOneBypass, "impulse");
  auto thirdAntiliasNoBypassStorage =
      std::make_unique<NtFx::SoftClip3AntiAlias>();
  auto& thirdAntiliasNoBypass         = *thirdAntiliasNoBypassStorage;
  thirdAntiliasNoBypass.bypass1Enable = false;
  thirdAntiliasNoBypass.bypass2Enable = false;
  NTFX_ADD_TEST(set, thirdAntiliasNoBypass, "impulse");
  return set.runAllTests();
}