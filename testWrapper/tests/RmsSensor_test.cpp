

#include "lib/ComponentTest.h"
#include "lib/RmsSensor.h"

NTFX_TEST_BEGIN

NTFX_TEST() {
  auto rmsSensor = NtFx::LongRmsSensorStereo<double>();
  rmsSensor.setT_ms(10);
  NTFX_ADD_TEST(rmsSensor, "dynamic_alternating");
  return NTFX_RUN_TESTS();
}