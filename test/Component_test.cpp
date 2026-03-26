

#include "lib/Biquad.h"
#include "lib/ComponentTest.h"
#include "lib/PeakSensor.h"
#include "lib/RmsSensor.h"
#include "lib/SoftClip.h"
#include "plugins/ntMultiband3.h"

NTFX_TEST_BEGIN

int main() {
  auto peakSensor = NtFx::PeakSensorStereo<float>();
  peakSensor.setT_ms(20);
  ADD_TEST(peakSensor, "dynamic");
  auto peakHoldSensor = NtFx::PeakHoldSensorStereo<float>();
  peakHoldSensor.setT_ms(20);
  peakHoldSensor.setTHold_ms(1);
  ADD_TEST(peakHoldSensor, "dynamic");
  auto rmsSensor = NtFx::RmsSensorStereo<float>();
  rmsSensor.setT_ms(10);
  ADD_TEST(rmsSensor, "dynamic");
  auto bqBell             = NtFx::Biquad::EqBand<float>();
  bqBell.settings.shape   = NtFx::Biquad::Shape::bell;
  bqBell.settings.gain_db = 12;
  bqBell.settings.fc_hz   = 4e3;
  bqBell.settings.q       = 2;
  ADD_TEST(bqBell, "impulse");
  auto bqHfp           = NtFx::Biquad::EqBand<float>();
  bqHfp.settings.shape = NtFx::Biquad::Shape::hpf;
  ADD_TEST(bqHfp, "impulse");
  auto bqLpf           = NtFx::Biquad::EqBand<float>();
  bqLpf.settings.shape = NtFx::Biquad::Shape::lpf;
  ADD_TEST(bqLpf, "impulse");
  auto bqLoShelf             = NtFx::Biquad::EqBand<float>();
  bqLoShelf.settings.shape   = NtFx::Biquad::Shape::loShelf;
  bqLoShelf.settings.gain_db = 12;
  ADD_TEST(bqLoShelf, "impulse");
  auto bqHiShelf             = NtFx::Biquad::EqBand<float>();
  bqHiShelf.settings.shape   = NtFx::Biquad::Shape::hiShelf;
  bqHiShelf.settings.gain_db = 12;
  ADD_TEST(bqHiShelf, "impulse");
  auto firstOrderHpf =
      NtFx::FirstOrder::StereoFilter<float, NtFx::FirstOrder::Shape::hpf>();
  firstOrderHpf.setFc(1e3);
  ADD_TEST(firstOrderHpf, "impulse");
  auto firstOrderLpf =
      NtFx::FirstOrder::StereoFilter<float, NtFx::FirstOrder::Shape::lpf>();
  firstOrderLpf.setFc(1e3);
  ADD_TEST(firstOrderLpf, "impulse");
  auto firstOrderLpfWithZero =
      NtFx::FirstOrder::StereoFilter<float, NtFx::FirstOrder::Shape::lpfZero>();
  firstOrderLpfWithZero.setFc(1e3);
  ADD_TEST(firstOrderLpfWithZero, "impulse");
  auto softClip3 = NtFx::SoftClip3<float>();
  ADD_TEST(softClip3, "linearSweep");
  auto softClip5 = NtFx::SoftClip5<float>();
  ADD_TEST(softClip5, "linearSweep");
  return NtFx::ComponentTest<float>::runAllTests();
}