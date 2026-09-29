#include "lib/Biquad.h"
#include "lib/ComponentTest.h"
#include <memory>

int main() {
  auto set   = NtFx::ComponentTestSet(std::string(testFileBaseName(__FILE__)));
  auto bell_ = std::make_unique<NtFx::Biquad::EqBand>();
  auto& bell = *bell_;
  bell.settings.shape   = NtFx::Biquad::Shape::bell;
  bell.settings.gain_db = 12;
  bell.settings.fc_hz   = 4e3;
  bell.settings.q       = 2;
  NTFX_ADD_TEST(set, bell, "impulse");
  auto hpf_          = std::make_unique<NtFx::Biquad::EqBand>();
  auto& hpf          = *hpf_;
  hpf.settings.shape = NtFx::Biquad::Shape::hpf;
  NTFX_ADD_TEST(set, hpf, "impulse");
  auto lpf_          = std::make_unique<NtFx::Biquad::EqBand>();
  auto& lpf          = *lpf_;
  lpf.settings.shape = NtFx::Biquad::Shape::lpf;
  NTFX_ADD_TEST(set, lpf, "impulse");
  auto loShelf_            = std::make_unique<NtFx::Biquad::EqBand>();
  auto& loShelf            = *loShelf_;
  loShelf.settings.shape   = NtFx::Biquad::Shape::loShelf;
  loShelf.settings.gain_db = 12;
  NTFX_ADD_TEST(set, loShelf, "impulse");
  auto hiShelf_            = std::make_unique<NtFx::Biquad::EqBand>();
  auto& hiShelf            = *hiShelf_;
  hiShelf.settings.shape   = NtFx::Biquad::Shape::hiShelf;
  hiShelf.settings.gain_db = 12;
  NTFX_ADD_TEST(set, hiShelf, "impulse");
  auto bandpass_          = std::make_unique<NtFx::Biquad::EqBand>();
  auto& bandpass          = *bandpass_;
  bandpass.settings.shape = NtFx::Biquad::Shape::bpf;
  NTFX_ADD_TEST(set, bandpass, "impulse");
  auto bandpass_q1_          = std::make_unique<NtFx::Biquad::EqBand>();
  auto& bandpass_q1          = *bandpass_q1_;
  bandpass_q1.settings.shape = NtFx::Biquad::Shape::bpf;
  bandpass_q1.settings.q     = 1;
  NTFX_ADD_TEST(set, bandpass_q1, "impulse");
  auto bandpass_q2_          = std::make_unique<NtFx::Biquad::EqBand>();
  auto& bandpass_q2          = *bandpass_q2_;
  bandpass_q2.settings.shape = NtFx::Biquad::Shape::bpf;
  bandpass_q2.settings.q     = 2;
  NTFX_ADD_TEST(set, bandpass_q2, "impulse");
  auto notch_          = std::make_unique<NtFx::Biquad::EqBand>();
  auto& notch          = *notch_;
  notch.settings.shape = NtFx::Biquad::Shape::notch;
  NTFX_ADD_TEST(set, notch, "impulse");
  return set.runAllTests();
}