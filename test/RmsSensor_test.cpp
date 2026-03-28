

#include "lib/ComponentTest.h"
#include "lib/RmsSensor.h"

NTFX_TEST_BEGIN

int main() {
  auto rmsSensor = NtFx::RmsSensorStereo<float>();
  rmsSensor.setT_ms(10);
  NTFX_ADD_TEST(rmsSensor, "dynamic");
  return NtFx::ComponentTest<float>::runAllTests();
}