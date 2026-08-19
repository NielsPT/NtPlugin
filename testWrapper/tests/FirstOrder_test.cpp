

#include "lib/ComponentTest.h"
#include "lib/FirstOrder.h"
#include <memory>

int main() {
  auto set = NtFx::ComponentTestSet(std::string(testFileBaseName(__FILE__)));
  auto hpfStorage = std::make_unique<
      NtFx::FirstOrder::StereoFilter<NtFx::FirstOrder::Shape::hpf>>();
  auto& hpf = *hpfStorage;
  hpf.fc_hz = 1e3;
  NTFX_ADD_TEST(set, hpf, "impulse");
  auto lpfStorage = std::make_unique<
      NtFx::FirstOrder::StereoFilter<NtFx::FirstOrder::Shape::lpf>>();
  auto& lpf = *lpfStorage;
  lpf.fc_hz = 1e3;
  NTFX_ADD_TEST(set, lpf, "impulse");
  auto lpfWithZeroStorage = std::make_unique<
      NtFx::FirstOrder::StereoFilter<NtFx::FirstOrder::Shape::lpfZero>>();
  auto& lpfWithZero = *lpfWithZeroStorage;
  lpfWithZero.fc_hz = 1e3;
  NTFX_ADD_TEST(set, lpfWithZero, "impulse");
  return set.runAllTests();
}