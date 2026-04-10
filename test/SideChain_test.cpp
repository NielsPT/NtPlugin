#include "lib/ComponentTest.h"
#include "lib/SideChain.h"

NTFX_TEST_BEGIN

NTFX_TEST() {
  auto peakSettings0      = NtFx::ScSettings<float>();
  peakSettings0.ratio_db  = 3;
  peakSettings0.knee_db   = 0;
  peakSettings0.thresh_db = -6;
  peakSettings0.tRel_ms   = 20;
  auto peakDbSc           = NtFx::PeakSideChainDb<float>(peakSettings0);
  NTFX_ADD_TEST(peakDbSc, "dynamic");
  auto peakSettings1       = peakSettings0;
  peakSettings1.linkEnable = true;
  auto peakDbScLink        = NtFx::PeakSideChainDb<float>(peakSettings1);
  NTFX_ADD_TEST(peakDbScLink, "dynamic");
  return NTFX_RUN_TESTS();
}