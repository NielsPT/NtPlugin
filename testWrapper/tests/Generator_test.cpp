#include "lib/ComponentTest.h"
#include "lib/Generator.h"
#include <memory>

int main() {
  auto set = NtFx::ComponentTestSet(std::string(testFileBaseName(__FILE__)));
  auto noiseStorage = std::make_unique<NtFx::Generator::Noise>();
  auto& noise       = *noiseStorage;
  NTFX_ADD_TEST(set, noise, "impulse");
  return set.runAllTests();
}