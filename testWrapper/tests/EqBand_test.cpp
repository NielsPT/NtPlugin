#include "lib/Audio.h"
#include "lib/Biquad.h"
#include "lib/ComponentTest.h"
#include <memory>

int main() {
  auto set = NtFx::ComponentTestSet(std::string(testFileBaseName(__FILE__)));
  auto bellStorage      = std::make_unique<NtFx::Biquad::EqBand>();
  auto& bell            = *bellStorage;
  bell.settings.shape   = NtFx::Biquad::Shape::bell;
  bell.settings.gain_db = 12;
  bell.settings.fc_hz   = 4e3;
  bell.settings.q       = 2;
  NTFX_ADD_TEST(set, bell, "impulse");
  auto hpfStorage    = std::make_unique<NtFx::Biquad::EqBand>();
  auto& hpf          = *hpfStorage;
  hpf.settings.shape = NtFx::Biquad::Shape::hpf;
  NTFX_ADD_TEST(set, hpf, "impulse");
  auto lpfStorage    = std::make_unique<NtFx::Biquad::EqBand>();
  auto& lpf          = *lpfStorage;
  lpf.settings.shape = NtFx::Biquad::Shape::lpf;
  NTFX_ADD_TEST(set, lpf, "impulse");
  auto loShelfStorage      = std::make_unique<NtFx::Biquad::EqBand>();
  auto& loShelf            = *loShelfStorage;
  loShelf.settings.shape   = NtFx::Biquad::Shape::loShelf;
  loShelf.settings.gain_db = 12;
  NTFX_ADD_TEST(set, loShelf, "impulse");
  auto hiShelfStorage      = std::make_unique<NtFx::Biquad::EqBand>();
  auto& hiShelf            = *hiShelfStorage;
  hiShelf.settings.shape   = NtFx::Biquad::Shape::hiShelf;
  hiShelf.settings.gain_db = 12;
  NTFX_ADD_TEST(set, hiShelf, "impulse");
  auto bandpassStorage    = std::make_unique<NtFx::Biquad::EqBand>();
  auto& bandpass          = *bandpassStorage;
  bandpass.settings.shape = NtFx::Biquad::Shape::bpf;
  NTFX_ADD_TEST(set, bandpass, "impulse");
  auto bandpass_q1Storage    = std::make_unique<NtFx::Biquad::EqBand>();
  auto& bandpass_q1          = *bandpass_q1Storage;
  bandpass_q1.settings.shape = NtFx::Biquad::Shape::bpf;
  bandpass_q1.settings.q     = 1;
  NTFX_ADD_TEST(set, bandpass_q1, "impulse");
  auto bandpass_q2Storage    = std::make_unique<NtFx::Biquad::EqBand>();
  auto& bandpass_q2          = *bandpass_q2Storage;
  bandpass_q2.settings.shape = NtFx::Biquad::Shape::bpf;
  bandpass_q2.settings.q     = 2;
  NTFX_ADD_TEST(set, bandpass_q2, "impulse");
  auto notchStorage    = std::make_unique<NtFx::Biquad::EqBand>();
  auto& notch          = *notchStorage;
  notch.settings.shape = NtFx::Biquad::Shape::notch;
  NTFX_ADD_TEST(set, notch, "impulse");
  auto cascadeSingleStorage = std::make_unique<NtFx::Biquad::Cascade<1>>();
  auto& cascadeSingle       = *cascadeSingleStorage;
  cascadeSingle.settings[0].shape = NtFx::Biquad::Shape::notch;
  NTFX_ADD_TEST(set, cascadeSingle, "impulse");
  auto cascade8notchStorage = std::make_unique<NtFx::Biquad::Cascade<8>>();
  auto& cascade8notch       = *cascade8notchStorage;
  for (size_t i = 0; i < 8; i++) {
    cascade8notch.settings[i].shape = NtFx::Biquad::Shape::notch;
    cascade8notch.settings[i].q     = 10;
    cascade8notch.settings[i].fc_hz = 100 * signal_t(i * i);
  }
  NTFX_ADD_TEST(set, cascade8notch, "impulse");
  return set.runAllTests();
}