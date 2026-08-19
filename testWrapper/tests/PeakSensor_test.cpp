

#include "lib/ComponentTest.h"
#include "lib/PeakSensor.h"
#include <memory>

int main() {
  auto set = NtFx::ComponentTestSet(std::string(testFileBaseName(__FILE__)));
  auto peakSensorStorage = std::make_unique<NtFx::PeakSensorStereo>();
  auto& peakSensor       = *peakSensorStorage;
  peakSensor.setT_ms(20);
  NTFX_ADD_TEST(set, peakSensor, "dynamic_alternating");
  auto peakHoldSensorStorage = std::make_unique<NtFx::PeakHoldSensorStereo<>>();
  auto& peakHoldSensor       = *peakHoldSensorStorage;
  peakHoldSensor.setT_ms(20);
  peakHoldSensor.setTHold_ms(1);
  NTFX_ADD_TEST(set, peakHoldSensor, "dynamic_alternating");
  return set.runAllTests();
}