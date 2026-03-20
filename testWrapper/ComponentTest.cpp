

#include "lib/ComponentTest.h"
#include "lib/Biquad.h"
#include "lib/PeakSensor.h"
#include "lib/RmsSensor.h"
#include "plugins/ntMultiband3.h"
#include <cassert>
#include <vector>

namespace NtFx {
template <>
int NtFx::ComponentTest<float>::nTests = 0;
template <>
int NtFx::ComponentTest<float>::nSuccessful = 0;
template <>
int NtFx::ComponentTest<float>::nComponents = 0;
template <>
std::vector<NtFx::ComponentTest<float>*>
    NtFx::ComponentTest<float>::allTests { };
template <>
std::vector<std::string> NtFx::ComponentTest<float>::allTestNames { };
}

int main() {
  auto peakSensor = NtFx::PeakSensorStereo<float>();
  peakSensor.setT_ms(20);
  ADD_TEST(peakSensor);
  auto peakHoldSensor = NtFx::PeakHoldSensorStereo<float>();
  peakHoldSensor.setT_ms(20);
  peakHoldSensor.setTHold_ms(1);
  ADD_TEST(peakHoldSensor);
  auto rmsSensor = NtFx::RmsSensorStereo<float>();
  rmsSensor.setT_ms(10);
  ADD_TEST(rmsSensor);
  auto bqBell             = NtFx::Biquad::EqBand<float>();
  bqBell.settings.shape   = NtFx::Biquad::Shape::bell;
  bqBell.settings.gain_db = 12;
  bqBell.settings.fc_hz   = 4e3;
  bqBell.settings.q       = 2;
  ADD_TEST(bqBell);
  auto bqHfp           = NtFx::Biquad::EqBand<float>();
  bqHfp.settings.shape = NtFx::Biquad::Shape::hpf;
  ADD_TEST(bqHfp);
  auto bqLfp           = NtFx::Biquad::EqBand<float>();
  bqLfp.settings.shape = NtFx::Biquad::Shape::lpf;
  ADD_TEST(bqLfp);
  auto bqLoShelf             = NtFx::Biquad::EqBand<float>();
  bqLoShelf.settings.shape   = NtFx::Biquad::Shape::loShelf;
  bqLoShelf.settings.gain_db = 12;
  ADD_TEST(bqLoShelf);
  auto bqHiShelf             = NtFx::Biquad::EqBand<float>();
  bqHiShelf.settings.shape   = NtFx::Biquad::Shape::hiShelf;
  bqHiShelf.settings.gain_db = 12;
  ADD_TEST(bqHiShelf);
  auto firstOrderHpf =
      NtFx::FirstOrder::StereoFilter<float, NtFx::FirstOrder::Shape::hpf>();
  firstOrderHpf.setFc(1e3);
  ADD_TEST(firstOrderHpf);
  auto firstOrderLpf =
      NtFx::FirstOrder::StereoFilter<float, NtFx::FirstOrder::Shape::lpf>();
  firstOrderLpf.setFc(1e3);
  ADD_TEST(firstOrderLpf);
  auto firstOrderLpfWithZero =
      NtFx::FirstOrder::StereoFilter<float, NtFx::FirstOrder::Shape::lpfZero>();
  firstOrderLpfWithZero.setFc(1e3);
  ADD_TEST(firstOrderLpfWithZero);
  NtFx::ComponentTest<float>::runAllTests();
  return !NtFx::ComponentTest<float>::getResults();
}