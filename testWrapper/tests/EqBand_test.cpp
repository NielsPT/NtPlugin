

#include "lib/Biquad.h"
#include "lib/ComponentTest.h"

NTFX_TEST_BEGIN

NTFX_TEST() {
  auto bell             = NtFx::Biquad::EqBand<double>();
  bell.settings.shape   = NtFx::Biquad::Shape::bell;
  bell.settings.gain_db = 12;
  bell.settings.fc_hz   = 4e3;
  bell.settings.q       = 2;
  NTFX_ADD_TEST(bell, "impulse");
  auto hpf           = NtFx::Biquad::EqBand<double>();
  hpf.settings.shape = NtFx::Biquad::Shape::hpf;
  NTFX_ADD_TEST(hpf, "impulse");
  auto lpf           = NtFx::Biquad::EqBand<double>();
  lpf.settings.shape = NtFx::Biquad::Shape::lpf;
  NTFX_ADD_TEST(lpf, "impulse");
  auto loShelf             = NtFx::Biquad::EqBand<double>();
  loShelf.settings.shape   = NtFx::Biquad::Shape::loShelf;
  loShelf.settings.gain_db = 12;
  NTFX_ADD_TEST(loShelf, "impulse");
  auto hiShelf             = NtFx::Biquad::EqBand<double>();
  hiShelf.settings.shape   = NtFx::Biquad::Shape::hiShelf;
  hiShelf.settings.gain_db = 12;
  NTFX_ADD_TEST(hiShelf, "impulse");
  return NTFX_RUN_TESTS();
}