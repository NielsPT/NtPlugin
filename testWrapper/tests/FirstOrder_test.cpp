

#include "lib/ComponentTest.h"
#include "plugins/ntMultiband3.h"

NTFX_TEST_BEGIN

NTFX_TEST() {
  auto hpf =
      NtFx::FirstOrder::StereoFilter<double, NtFx::FirstOrder::Shape::hpf>();
  hpf.setFc(1e3);
  NTFX_ADD_TEST(hpf, "impulse");
  auto lpf =
      NtFx::FirstOrder::StereoFilter<double, NtFx::FirstOrder::Shape::lpf>();
  lpf.setFc(1e3);
  NTFX_ADD_TEST(lpf, "impulse");
  auto lpfWithZero = NtFx::FirstOrder::StereoFilter<double,
      NtFx::FirstOrder::Shape::lpfZero>();
  lpfWithZero.setFc(1e3);
  NTFX_ADD_TEST(lpfWithZero, { "impulse" });
  return NTFX_RUN_TESTS();
}