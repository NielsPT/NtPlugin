#include "lib/ComponentTest.h"
#include "lib/Generator.h"

NTFX_TEST_BEGIN

NTFX_TEST() {
  NtFx::Generator::Noise noise;
  NTFX_ADD_TEST(noise, "impulse");
  return NTFX_RUN_TESTS();
}