

#include "lib/ComponentTest.h"
#include "lib/RmsSensor.h"

int main() {
  auto set = NtFx::ComponentTestSet(std::string(testFileBaseName(__FILE__)));
  auto rmsSensor = NtFx::LongRmsSensorStereo();
  rmsSensor.setT_ms(10);
  NTFX_ADD_TEST(set, rmsSensor, "dynamic_alternating");
  return set.runAllTests();
}