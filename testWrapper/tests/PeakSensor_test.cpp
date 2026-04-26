

#include "lib/ComponentTest.h"
#include "lib/PeakSensor.h"

NTFX_TEST_BEGIN

NTFX_TEST() {
  auto peakSensor = NtFx::PeakSensorStereo<double>();
  peakSensor.setT_ms(20);
  NTFX_ADD_TEST(peakSensor, "dynamic_alternating");
  auto peakHoldSensor = NtFx::PeakHoldSensorStereo<double>();
  peakHoldSensor.setT_ms(20);
  peakHoldSensor.setTHold_ms(1);
  NTFX_ADD_TEST(peakHoldSensor, "dynamic_alternating");
  return NTFX_RUN_TESTS();
}