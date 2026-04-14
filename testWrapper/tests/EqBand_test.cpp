

#include "lib/Biquad.h"
#include "lib/ComponentTest.h"

NTFX_TEST_BEGIN

NTFX_TEST() {
  auto bqBell             = NtFx::Biquad::EqBand<double>();
  bqBell.settings.shape   = NtFx::Biquad::Shape::bell;
  bqBell.settings.gain_db = 12;
  bqBell.settings.fc_hz   = 4e3;
  bqBell.settings.q       = 2;
  NTFX_ADD_TEST(bqBell, "impulse");
  auto bqHfp           = NtFx::Biquad::EqBand<double>();
  bqHfp.settings.shape = NtFx::Biquad::Shape::hpf;
  NTFX_ADD_TEST(bqHfp, "impulse");
  auto bqLpf           = NtFx::Biquad::EqBand<double>();
  bqLpf.settings.shape = NtFx::Biquad::Shape::lpf;
  NTFX_ADD_TEST(bqLpf, "impulse");
  auto bqLoShelf             = NtFx::Biquad::EqBand<double>();
  bqLoShelf.settings.shape   = NtFx::Biquad::Shape::loShelf;
  bqLoShelf.settings.gain_db = 12;
  NTFX_ADD_TEST(bqLoShelf, "impulse");
  auto bqHiShelf             = NtFx::Biquad::EqBand<double>();
  bqHiShelf.settings.shape   = NtFx::Biquad::Shape::hiShelf;
  bqHiShelf.settings.gain_db = 12;
  NTFX_ADD_TEST(bqHiShelf, "impulse");
  return NTFX_RUN_TESTS();
}