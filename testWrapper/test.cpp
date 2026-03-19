

#include "lib/Biquad.h"
#include "lib/ComponentTest.h"
#include "lib/PeakSensor.h"
#include "lib/RmsSensor.h"
#include "plugins/ntMultiband3.h"
#include <cassert>
#include <vector>

using testBench = NtFx::ComponentTest<float>;

namespace NtFx {
template <>
int testBench::nTests = 0;
template <>
int testBench::nSuccessful = 0;
template <>
int testBench::nComponents = 0;
template <>
std::vector<testBench*> testBench::allTests { };
template <>
std::vector<std::string> testBench::allTestNames { };
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
  auto biquad             = NtFx::Biquad::EqBand<float>();
  biquad.settings.gain_db = 12;
  biquad.settings.fc_hz   = 4e3;
  biquad.settings.shape   = NtFx::Biquad::Shape::bell;
  biquad.settings.q       = 2;
  ADD_TEST(biquad);
  auto hpf =
      NtFx::FirstOrder::StereoFilter<float, NtFx::FirstOrder::Shape::hpf>();
  hpf.setFc(1e3);
  ADD_TEST(hpf);
  auto lpf =
      NtFx::FirstOrder::StereoFilter<float, NtFx::FirstOrder::Shape::lpf>();
  lpf.setFc(1e3);
  ADD_TEST(lpf);
  auto lpfZero =
      NtFx::FirstOrder::StereoFilter<float, NtFx::FirstOrder::Shape::lpfZero>();
  lpfZero.setFc(1e3);
  ADD_TEST(lpfZero);
  testBench::runAllTests();
  return !testBench::getResults();
}