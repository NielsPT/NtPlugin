#include "lib/ComponentTest.h"
#include "lib/Generator.h"

int main() {
  auto set = NtFx::ComponentTestSet(std::string(testFileBaseName(__FILE__)));
  NtFx::Generator::Noise noise;
  NTFX_ADD_TEST(set, noise, "impulse");
  return set.runAllTests();
}