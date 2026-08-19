#include "lib/Comp.h"
#include "lib/ComponentTest.h"
#include <memory>

int main() {
  auto set = NtFx::ComponentTestSet(std::string(testFileBaseName(__FILE__)));
  auto sharedSettings      = NtFx::Comp::ScSettings();
  sharedSettings.ratio     = 3;
  sharedSettings.knee_db   = 0;
  sharedSettings.thresh_db = -6;
  sharedSettings.tAtt_ms   = 1;
  sharedSettings.tRel_ms   = 20;
  auto peakDbScStorage     = std::make_unique<NtFx::Comp::PeakSideChainDb>();
  auto& peakDbSc           = *peakDbScStorage;
  peakDbSc.settings        = sharedSettings;
  NTFX_ADD_TEST(set, peakDbSc, "dynamic_alternating");
  auto peakSettings1       = sharedSettings;
  peakSettings1.linkEnable = true;
  auto peakDbLinStorage    = std::make_unique<NtFx::Comp::PeakSideChainLin>();
  auto& peakDbLin          = *peakDbLinStorage;
  peakDbLin.settings       = peakSettings1;
  NTFX_ADD_TEST(set, peakDbLin, "dynamic_alternating");
  auto peakSettings2       = sharedSettings;
  peakSettings2.linkEnable = true;
  auto peakDbScLinkStorage = std::make_unique<NtFx::Comp::PeakSideChainDb>();
  auto& peakDbScLink       = *peakDbScLinkStorage;
  peakDbScLink.settings    = peakSettings2;
  NTFX_ADD_TEST(set, peakDbScLink, "dynamic_alternating");
  auto rmsSettings    = sharedSettings;
  rmsSettings.tRms_ms = 10;
  rmsSettings.tAtt_ms = 10;
  auto rmsScDbStorage = std::make_unique<NtFx::Comp::RmsSideChainDb>();
  auto& rmsScDb       = *rmsScDbStorage;
  rmsScDb.settings    = rmsSettings;
  NTFX_ADD_TEST(set, rmsScDb, "dynamic_alternating");
  auto rmsScLinStorage = std::make_unique<NtFx::Comp::RmsSideChainLinear>();
  auto& rmsScLin       = *rmsScLinStorage;
  rmsScLin.settings    = rmsSettings;
  NTFX_ADD_TEST(set, rmsScLin, "dynamic_alternating");
  return set.runAllTests();
}