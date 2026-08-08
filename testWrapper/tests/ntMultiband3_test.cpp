#include "lib/ComponentTest.h"
#include "plugins/ntMultiband3.h"

NTFX_TEST_BEGIN

NTFX_TEST() {
  auto bypass         = ntMultiband3();
  bypass.bypassEnable = true;
  NTFX_ADD_TEST(bypass, "impulse");
  auto defaults = ntMultiband3();
  NTFX_ADD_TEST(defaults, "impulse");
  auto soloLow              = ntMultiband3();
  soloLow.solos[Bands::hi]  = false;
  soloLow.solos[Bands::mid] = false;
  soloLow.solos[Bands::lo]  = true;
  NTFX_ADD_TEST(soloLow, "impulse");
  auto soloMid              = ntMultiband3();
  soloMid.solos[Bands::hi]  = false;
  soloMid.solos[Bands::mid] = true;
  soloMid.solos[Bands::lo]  = false;
  NTFX_ADD_TEST(soloMid, "impulse");
  auto soloHi              = ntMultiband3();
  soloHi.solos[Bands::hi]  = true;
  soloHi.solos[Bands::mid] = false;
  soloHi.solos[Bands::lo]  = false;
  NTFX_ADD_TEST(soloHi, "impulse");
  return NTFX_RUN_TESTS();
}
