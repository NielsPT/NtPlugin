

#include "lib/ComponentTest.h"
#include "lib/SoftClip.h"
#include <memory>

int main() {
  auto set = NtFx::ComponentTestSet(std::string(testFileBaseName(__FILE__)));
  auto hardStorage = std::make_unique<NtFx::Clip::Hard>();
  auto& hard       = *hardStorage;
  NTFX_ADD_TEST(set, hard, "linearSweep");
  auto hardPlus6Storage = std::make_unique<NtFx::Clip::Hard>();
  auto& hardPlus6       = *hardPlus6Storage;
  hardPlus6.gain_db     = 6;
  NTFX_ADD_TEST(set, hardPlus6, "linearSweep");
  auto thirdStorage = std::make_unique<NtFx::Clip::Soft3>();
  auto& third       = *thirdStorage;
  NTFX_ADD_TEST(set, third, "linearSweep");
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
  return set.runAllTests();
}