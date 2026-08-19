

#include "lib/ComponentTest.h"
#include "lib/RmsSensor.h"
#include <memory>

int main() {
  auto set = NtFx::ComponentTestSet(std::string(testFileBaseName(__FILE__)));
  auto rmsSensorStorage = std::make_unique<NtFx::LongRmsSensorStereo<>>();
  auto& rmsSensor       = *rmsSensorStorage;
  rmsSensor.setT_ms(10);
  NTFX_ADD_TEST(set, rmsSensor, "dynamic_alternating");
  return set.runAllTests();
}