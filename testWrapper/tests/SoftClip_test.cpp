

#include "lib/ComponentTest.h"
#include "lib/SoftClip.h"

NTFX_TEST_BEGIN

NTFX_TEST() {
  auto softClip3 = NtFx::SoftClip3<float>();
  NTFX_ADD_TEST(softClip3, "linearSweep");
  auto softClip5 = NtFx::SoftClip5<float>();
  NTFX_ADD_TEST(softClip5, "linearSweep");
  return NTFX_RUN_TESTS();
}