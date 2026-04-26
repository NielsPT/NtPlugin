

#include "lib/ComponentTest.h"
#include "lib/SoftClip.h"

NTFX_TEST_BEGIN

NTFX_TEST() {
  auto third = NtFx::SoftClip3<double>();
  NTFX_ADD_TEST(third, "linearSweep");
  auto fifth = NtFx::SoftClip5<double>();
  NTFX_ADD_TEST(fifth, "linearSweep");
  return NTFX_RUN_TESTS();
}