

#include "lib/ComponentTest.h"
#include "lib/PeakSensor.h"

int main() {
  auto set = NtFx::ComponentTestSet(std::string(testFileBaseName(__FILE__)));
  auto peakSensor = NtFx::PeakSensorStereo();
  peakSensor.setT_ms(20);
  NTFX_ADD_TEST(set, peakSensor, "dynamic_alternating");
  auto peakHoldSensor = NtFx::PeakHoldSensorStereo();
  peakHoldSensor.setT_ms(20);
  peakHoldSensor.setTHold_ms(1);
  NTFX_ADD_TEST(set, peakHoldSensor, "dynamic_alternating");
  return set.runAllTests();
}