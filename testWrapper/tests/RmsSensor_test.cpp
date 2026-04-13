

#include "lib/ComponentTest.h"
#include "lib/RmsSensor.h"

NTFX_TEST_BEGIN

NTFX_TEST() {
  auto rmsSensor = NtFx::RmsSensorStereo<float>();
  rmsSensor.setT_ms(10);
  NTFX_ADD_TEST(rmsSensor, "dynamic");
  return NTFX_RUN_TESTS();
}