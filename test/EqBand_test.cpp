

#include "lib/Biquad.h"
#include "lib/ComponentTest.h"

NTFX_TEST_BEGIN

int main() {
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
  return NtFx::ComponentTest<float>::runAllTests();
}