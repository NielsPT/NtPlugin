#include "lib/ComponentTest.h"
#include "lib/GateSc.h"

NTFX_TEST_BEGIN

NTFX_TEST() {
  auto noHold               = NtFx::Gate::Sc();
  noHold.settings.thresh_db = -6;
  noHold.settings.range_db  = -12;
  noHold.settings.tHold_ms  = 1;
  noHold.settings.tRel_ms   = 10;
  NTFX_ADD_TEST(noHold, "dynamic_matched");
  auto slowAttAndHold               = NtFx::Gate::Sc();
  slowAttAndHold.settings.thresh_db = -6;
  slowAttAndHold.settings.range_db  = -12;
  slowAttAndHold.settings.tHold_ms  = 1;
  slowAttAndHold.settings.tRel_ms   = 10;
  slowAttAndHold.settings.tHold_ms  = 10;
  slowAttAndHold.settings.tAtt_ms   = 5;
  NTFX_ADD_TEST(slowAttAndHold, "dynamic_matched");
  return NTFX_RUN_TESTS();
}
