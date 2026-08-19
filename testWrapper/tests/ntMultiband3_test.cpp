#include "lib/ComponentTest.h"
#include "plugins/ntMultiband3.h"
#include <memory>

int main() {
  auto set = NtFx::ComponentTestSet(std::string(testFileBaseName(__FILE__)));
  auto bypassStorage  = std::make_unique<ntMultiband3>();
  auto& bypass        = *bypassStorage;
  bypass.bypassEnable = true;
  NTFX_ADD_TEST(set, bypass, "impulse");
  auto defaultsStorage = std::make_unique<ntMultiband3>();
  auto& defaults       = *defaultsStorage;
  NTFX_ADD_TEST(set, defaults, "impulse");
  auto soloLowStorage       = std::make_unique<ntMultiband3>();
  auto& soloLow             = *soloLowStorage;
  soloLow.solos[Bands::hi]  = false;
  soloLow.solos[Bands::mid] = false;
  soloLow.solos[Bands::lo]  = true;
  NTFX_ADD_TEST(set, soloLow, "impulse");
  auto soloMidStorage       = std::make_unique<ntMultiband3>();
  auto& soloMid             = *soloMidStorage;
  soloMid.solos[Bands::hi]  = false;
  soloMid.solos[Bands::mid] = true;
  soloMid.solos[Bands::lo]  = false;
  NTFX_ADD_TEST(set, soloMid, "impulse");
  auto soloHiStorage       = std::make_unique<ntMultiband3>();
  auto& soloHi             = *soloHiStorage;
  soloHi.solos[Bands::hi]  = true;
  soloHi.solos[Bands::mid] = false;
  soloHi.solos[Bands::lo]  = false;
  NTFX_ADD_TEST(set, soloHi, "impulse");
  return set.runAllTests();
}
