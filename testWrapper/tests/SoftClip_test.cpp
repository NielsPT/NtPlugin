

#include "lib/ComponentTest.h"
#include "lib/SoftClip.h"
#include <memory>

int main() {
  auto set = NtFx::ComponentTestSet(std::string(testFileBaseName(__FILE__)));
  auto hardDefaultStorage = std::make_unique<NtFx::Clip::Hard>();
  auto& hardDefault       = *hardDefaultStorage;
  NTFX_ADD_TEST(set, hardDefault, "linearSweep");
  NTFX_ADD_TEST(set, hardDefault, "sin100Hz");
  NTFX_ADD_TEST(set, hardDefault, "sin1kHz");
  NTFX_ADD_TEST(set, hardDefault, "sin10kHz");
  NTFX_ADD_TEST(set, hardDefault, "dualSin100Hz10kHz");
  auto hardPlus6Storage = std::make_unique<NtFx::Clip::Hard>();
  auto& hardPlus6       = *hardPlus6Storage;
  hardPlus6.gain_db     = 6;
  NTFX_ADD_TEST(set, hardPlus6, "linearSweep");
  NTFX_ADD_TEST(set, hardPlus6, "sin1kHz");
  auto thirdStorage = std::make_unique<NtFx::Clip::Soft3>();
  auto& third       = *thirdStorage;
  NTFX_ADD_TEST(set, third, "linearSweep");
  NTFX_ADD_TEST(set, third, "sin1kHz");
  auto fifthStorage = std::make_unique<NtFx::Clip::Soft5>();
  auto& fifth       = *fifthStorage;
  NTFX_ADD_TEST(set, fifth, "linearSweep");
  auto thirdAntialiasStorage  = std::make_unique<NtFx::Clip::SoftAntialias3>();
  auto& thirdAntialiasDefault = *thirdAntialiasStorage;
  NTFX_ADD_TEST(set, thirdAntialiasDefault, "linearSweep");
  NTFX_ADD_TEST(set, thirdAntialiasDefault, "impulse");
  auto thirdAntialiasOneBypassStorage =
      std::make_unique<NtFx::Clip::SoftAntialias3>();
  auto& thirdAntialiasOneBypass         = *thirdAntialiasOneBypassStorage;
  thirdAntialiasOneBypass.bypass2Enable = false;
  NTFX_ADD_TEST(set, thirdAntialiasOneBypass, "impulse");
  auto thirdAntialiasNoBypassStorage =
      std::make_unique<NtFx::Clip::SoftAntialias3>();
  auto& thirdAntialiasNoBypass         = *thirdAntialiasNoBypassStorage;
  thirdAntialiasNoBypass.bypass1Enable = false;
  thirdAntialiasNoBypass.bypass2Enable = false;
  NTFX_ADD_TEST(set, thirdAntialiasNoBypass, "impulse");
  auto secondDefaultStorage = std::make_unique<NtFx::Clip::Second>();
  auto secondDefault        = *secondDefaultStorage;
  NTFX_ADD_TEST(set, secondDefault, "linearSweep");
  NTFX_ADD_TEST(set, secondDefault, "impulse");
  return set.runAllTests();
}