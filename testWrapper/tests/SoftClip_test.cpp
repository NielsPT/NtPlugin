

#include "lib/ComponentTest.h"
#include "lib/SoftClip.h"

int main() {
  auto set   = NtFx::ComponentTestSet(std::string(testFileBaseName(__FILE__)));
  auto third = NtFx::SoftClip3();
  NTFX_ADD_TEST(set, third, "linearSweep");
  auto fifth = NtFx::SoftClip5();
  NTFX_ADD_TEST(set, fifth, "linearSweep");
  return set.runAllTests();
}