

#include "lib/ComponentTest.h"
#include "lib/RmsSensor.h"

NTFX_TEST_BEGIN

NTFX_TEST() {
  auto rmsSensor = NtFx::RmsSensorStereo<double>();
  rmsSensor.setT_ms(10);
  NTFX_ADD_TEST(rmsSensor, "dynamic");
  return NTFX_RUN_TESTS();
}