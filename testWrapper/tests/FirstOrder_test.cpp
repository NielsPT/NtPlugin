

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