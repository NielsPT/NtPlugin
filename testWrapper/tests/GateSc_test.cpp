#include "lib/ComponentTest.h"
#include "lib/GateSc.h"

NTFX_TEST_BEGIN

NTFX_TEST() {
  auto set      = NtFx::Gate::ScSettings<double>();
  set.thresh_db = -6;
  set.range_db  = -12;
  set.tHold_ms  = 1;
  set.tRel_ms   = 10;
  auto noHold   = NtFx::Gate::Sc<double>(set);
  NTFX_ADD_TEST(noHold, "dynamic_matched");
  auto holdSet        = set;
  holdSet.tHold_ms    = 10;
  holdSet.tAtt_ms     = 5;
  auto slowAttAndHold = NtFx::Gate::Sc<double>(holdSet);
  NTFX_ADD_TEST(slowAttAndHold, "dynamic_matched");
  return NTFX_RUN_TESTS();
}
