#include "lib/ComponentTest.h"
#include "lib/GateSc.h"
#include <memory>

int main() {
  auto set = NtFx::ComponentTestSet(std::string(testFileBaseName(__FILE__)));
  auto noHoldStorage        = std::make_unique<NtFx::Gate::Sc>();
  auto& noHold              = *noHoldStorage;
  noHold.settings.thresh_db = -6;
  noHold.settings.range_db  = -12;
  noHold.settings.tHold_ms  = 1;
  noHold.settings.tRel_ms   = 10;
  NTFX_ADD_TEST(set, noHold, "dynamic_matched");
  auto slowAttAndHoldStorage        = std::make_unique<NtFx::Gate::Sc>();
  auto& slowAttAndHold              = *slowAttAndHoldStorage;
  slowAttAndHold.settings.thresh_db = -6;
  slowAttAndHold.settings.range_db  = -12;
  slowAttAndHold.settings.tHold_ms  = 1;
  slowAttAndHold.settings.tRel_ms   = 10;
  slowAttAndHold.settings.tHold_ms  = 10;
  slowAttAndHold.settings.tAtt_ms   = 5;
  NTFX_ADD_TEST(set, slowAttAndHold, "dynamic_matched");
  return set.runAllTests();
}
