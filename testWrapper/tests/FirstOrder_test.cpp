

#include "lib/ComponentTest.h"
#include "lib/FirstOrder.h"

NTFX_TEST_BEGIN

NTFX_TEST() {
  auto hpf  = NtFx::FirstOrder::StereoFilter<NtFx::FirstOrder::Shape::hpf>();
  hpf.fc_hz = 1e3;
  NTFX_ADD_TEST(hpf, "impulse");
  auto lpf  = NtFx::FirstOrder::StereoFilter<NtFx::FirstOrder::Shape::lpf>();
  lpf.fc_hz = 1e3;
  NTFX_ADD_TEST(lpf, "impulse");
  auto lpfWithZero =
      NtFx::FirstOrder::StereoFilter<NtFx::FirstOrder::Shape::lpfZero>();
  lpfWithZero.fc_hz = 1e3;
  NTFX_ADD_TEST(lpfWithZero, { "impulse" });
  return NTFX_RUN_TESTS();
}