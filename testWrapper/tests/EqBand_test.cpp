#include "lib/Audio.h"
#include "lib/Biquad.h"
#include "lib/ComponentTest.h"

int main() {
  auto set  = NtFx::ComponentTestSet(std::string(testFileBaseName(__FILE__)));
  auto bell = NtFx::Biquad::EqBand();
  bell.settings.shape   = NtFx::Biquad::Shape::bell;
  bell.settings.gain_db = 12;
  bell.settings.fc_hz   = 4e3;
  bell.settings.q       = 2;
  NTFX_ADD_TEST(set, bell, "impulse");
  auto hpf           = NtFx::Biquad::EqBand();
  hpf.settings.shape = NtFx::Biquad::Shape::hpf;
  NTFX_ADD_TEST(set, hpf, "impulse");
  auto lpf           = NtFx::Biquad::EqBand();
  lpf.settings.shape = NtFx::Biquad::Shape::lpf;
  NTFX_ADD_TEST(set, lpf, "impulse");
  auto loShelf             = NtFx::Biquad::EqBand();
  loShelf.settings.shape   = NtFx::Biquad::Shape::loShelf;
  loShelf.settings.gain_db = 12;
  NTFX_ADD_TEST(set, loShelf, "impulse");
  auto hiShelf             = NtFx::Biquad::EqBand();
  hiShelf.settings.shape   = NtFx::Biquad::Shape::hiShelf;
  hiShelf.settings.gain_db = 12;
  NTFX_ADD_TEST(set, hiShelf, "impulse");
  auto bandpass           = NtFx::Biquad::EqBand();
  bandpass.settings.shape = NtFx::Biquad::Shape::bpf;
  NTFX_ADD_TEST(set, bandpass, "impulse");
  auto bandpass_q1           = NtFx::Biquad::EqBand();
  bandpass_q1.settings.shape = NtFx::Biquad::Shape::bpf;
  bandpass_q1.settings.q     = 1;
  NTFX_ADD_TEST(set, bandpass_q1, "impulse");
  auto bandpass_q2           = NtFx::Biquad::EqBand();
  bandpass_q2.settings.shape = NtFx::Biquad::Shape::bpf;
  bandpass_q2.settings.q     = 2;
  NTFX_ADD_TEST(set, bandpass_q2, "impulse");
  auto notch           = NtFx::Biquad::EqBand();
  notch.settings.shape = NtFx::Biquad::Shape::notch;
  NTFX_ADD_TEST(set, notch, "impulse");
  auto cascadeSingle              = NtFx::Biquad::Eq<1>();
  cascadeSingle.settings[0].shape = NtFx::Biquad::Shape::notch;
  NTFX_ADD_TEST(set, cascadeSingle, "impulse");
  auto cascade8notch = NtFx::Biquad::Eq<8>();
  for (size_t i = 0; i < 8; i++) {
    cascade8notch.settings[i].shape = NtFx::Biquad::Shape::notch;
    cascade8notch.settings[i].q     = 10;
    cascade8notch.settings[i].fc_hz = 100 * signal_t(i * i);
  }
  NTFX_ADD_TEST(set, cascade8notch, "impulse");
  return set.runAllTests();
}