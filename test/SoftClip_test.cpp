

#include "lib/ComponentTest.h"
#include "lib/SoftClip.h"

NTFX_TEST_BEGIN

int main() {
  auto softClip3 = NtFx::SoftClip3<float>();
  ADD_TEST(softClip3, "linearSweep");
  auto softClip5 = NtFx::SoftClip5<float>();
  ADD_TEST(softClip5, "linearSweep");
  return NtFx::ComponentTest<float>::runAllTests();
}