

#include "lib/ComponentTest.h"
#include "lib/FirstOrder.h"

int main() {
  auto set  = NtFx::ComponentTestSet(std::string(testFileBaseName(__FILE__)));
  auto hpf  = NtFx::FirstOrder::StereoFilter<NtFx::FirstOrder::Shape::hpf>();
  hpf.fc_hz = 1e3;
  NTFX_ADD_TEST(set, hpf, "impulse");
  auto lpf  = NtFx::FirstOrder::StereoFilter<NtFx::FirstOrder::Shape::lpf>();
  lpf.fc_hz = 1e3;
  NTFX_ADD_TEST(set, lpf, "impulse");
  auto lpfWithZero =
      NtFx::FirstOrder::StereoFilter<NtFx::FirstOrder::Shape::lpfZero>();
  lpfWithZero.fc_hz = 1e3;
  NTFX_ADD_TEST(set, lpfWithZero, "impulse");
  return set.runAllTests();
}