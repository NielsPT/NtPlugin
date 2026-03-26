

#include "lib/ComponentTest.h"
#include "lib/PeakSensor.h"

NTFX_TEST_BEGIN

int main() {
  auto peakSensor = NtFx::PeakSensorStereo<float>();
  peakSensor.setT_ms(20);
  NTFX_ADD_SINGLE_TEST(peakSensor, "dynamic");
  auto peakHoldSensor = NtFx::PeakHoldSensorStereo<float>();
  peakHoldSensor.setT_ms(20);
  peakHoldSensor.setTHold_ms(1);
  NTFX_ADD_SINGLE_TEST(peakHoldSensor, "dynamic");
  return NtFx::ComponentTest<float>::runAllTests();
}