#include "lib/ComponentTest.h"
#include "lib/GateSc.h"

int main() {
  auto set    = NtFx::ComponentTestSet(std::string(testFileBaseName(__FILE__)));
  auto noHold = NtFx::Gate::Sc();
  noHold.settings.thresh_db = -6;
  noHold.settings.range_db  = -12;
  noHold.settings.tHold_ms  = 1;
  noHold.settings.tRel_ms   = 10;
  NTFX_ADD_TEST(set, noHold, "dynamic_matched");
  auto slowAttAndHold               = NtFx::Gate::Sc();
  slowAttAndHold.settings.thresh_db = -6;
  slowAttAndHold.settings.range_db  = -12;
  slowAttAndHold.settings.tHold_ms  = 1;
  slowAttAndHold.settings.tRel_ms   = 10;
  slowAttAndHold.settings.tHold_ms  = 10;
  slowAttAndHold.settings.tAtt_ms   = 5;
  NTFX_ADD_TEST(set, slowAttAndHold, "dynamic_matched");
  return set.runAllTests();
}
