#include "lib/ComponentTest.h"
#include "lib/SideChain.h"

NTFX_TEST_BEGIN

NTFX_TEST() {
  auto sharedSettings      = NtFx::ScSettings<double>();
  sharedSettings.ratio_db  = 3;
  sharedSettings.knee_db   = 0;
  sharedSettings.thresh_db = -6;
  sharedSettings.tRel_ms   = 20;
  auto peakDbSc            = NtFx::PeakSideChainDb<double>(sharedSettings);
  NTFX_ADD_TEST(peakDbSc, "dynamic");
  auto peakSettings1       = sharedSettings;
  peakSettings1.linkEnable = true;
  auto peakDbScLink        = NtFx::PeakSideChainDb<double>(peakSettings1);
  NTFX_ADD_TEST(peakDbScLink, "dynamic");
  auto rmsSettings    = sharedSettings;
  rmsSettings.tRms_ms = 10;
  rmsSettings.tAtt_ms = 10;
  auto rmsScDb        = NtFx::RmsSideChainDb<double>(rmsSettings);
  NTFX_ADD_TEST(rmsScDb, "dynamic");
  auto rmsScLin = NtFx::RmsSideChainLinear<double>(rmsSettings);
  NTFX_ADD_TEST(rmsScLin, "dynamic");
  return NTFX_RUN_TESTS();
}