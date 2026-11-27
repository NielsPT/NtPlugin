

#include "lib/Audio.h"
#include "lib/ComponentTest.h"
#include "lib/FirstOrder.h"
#include <memory>

int main() {
  auto set = NtFx::ComponentTestSet(std::string(testFileBaseName(__FILE__)));
  auto hpfStorage = std::make_unique<
      NtFx::FirstOrder::Filter<NtFx::FirstOrder::Shape::hpf>>();
  auto& hpf = *hpfStorage;
  hpf.fc_hz = 1e3;
  NTFX_ADD_TEST(set, hpf, "impulse");
  auto lpfStorage = std::make_unique<
      NtFx::FirstOrder::Filter<NtFx::FirstOrder::Shape::lpf>>();
  auto& lpf = *lpfStorage;
  lpf.fc_hz = 1e3;
  NTFX_ADD_TEST(set, lpf, "impulse");
  auto lpfWithZeroStorage = std::make_unique<
      NtFx::FirstOrder::Filter<NtFx::FirstOrder::Shape::lpfZero>>();
  auto& lpfWithZero = *lpfWithZeroStorage;
  lpfWithZero.fc_hz = 1e3;
  NTFX_ADD_TEST(set, lpfWithZero, "impulse");

  auto _loShelfPlus6   = std::make_unique<NtFx::FirstOrder::LoShelf<Audio>>();
  auto loShelfPlus6    = *_loShelfPlus6;
  loShelfPlus6.gain_db = 6;
  NTFX_ADD_TEST(set, loShelfPlus6, "impulse");

  auto _loShelfMinus6   = std::make_unique<NtFx::FirstOrder::LoShelf<Audio>>();
  auto loShelfMinus6    = *_loShelfMinus6;
  loShelfMinus6.gain_db = -6;
  NTFX_ADD_TEST(set, loShelfMinus6, "impulse");

  auto _hiShelfPlus6   = std::make_unique<NtFx::FirstOrder::HiShelf<Audio>>();
  auto hiShelfPlus6    = *_hiShelfPlus6;
  hiShelfPlus6.gain_db = 6;
  NTFX_ADD_TEST(set, hiShelfPlus6, "impulse");

  auto _hiShelfMinus6   = std::make_unique<NtFx::FirstOrder::HiShelf<Audio>>();
  auto hiShelfMinus6    = *_hiShelfMinus6;
  hiShelfMinus6.gain_db = -6;
  NTFX_ADD_TEST(set, hiShelfMinus6, "impulse");

  auto _toneControlDefault =
      std::make_unique<NtFx::FirstOrder::ToneControl<Audio>>();
  auto& toneControlDefault = *_toneControlDefault;
  NTFX_ADD_TEST(set, toneControlDefault, "impulse");
  auto _toneControlPlus20 =
      std::make_unique<NtFx::FirstOrder::ToneControl<Audio>>();
  auto& toneControlPlus20   = *_toneControlPlus20;
  toneControlPlus20.gain_db = 20;
  NTFX_ADD_TEST(set, toneControlPlus20, "impulse");
  auto _toneControlMinus20 =
      std::make_unique<NtFx::FirstOrder::ToneControl<Audio>>();
  auto& toneControlMinus20   = *_toneControlMinus20;
  toneControlMinus20.gain_db = -20;
  NTFX_ADD_TEST(set, toneControlMinus20, "impulse");
  return set.runAllTests();
}