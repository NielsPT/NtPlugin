

#include "lib/ComponentTest.h"
#include "plugins/ntMultiband3.h"

NTFX_TEST_BEGIN

NTFX_TEST() {
  auto firstOrderHpf =
      NtFx::FirstOrder::StereoFilter<float, NtFx::FirstOrder::Shape::hpf>();
  firstOrderHpf.setFc(1e3);
  NTFX_ADD_TEST(firstOrderHpf, "impulse");
  auto firstOrderLpf =
      NtFx::FirstOrder::StereoFilter<float, NtFx::FirstOrder::Shape::lpf>();
  firstOrderLpf.setFc(1e3);
  NTFX_ADD_TEST(firstOrderLpf, "impulse");
  auto firstOrderLpfWithZero =
      NtFx::FirstOrder::StereoFilter<float, NtFx::FirstOrder::Shape::lpfZero>();
  firstOrderLpfWithZero.setFc(1e3);
  NTFX_ADD_TEST(firstOrderLpfWithZero, { "impulse" });
  return NTFX_RUN_TESTS();
}