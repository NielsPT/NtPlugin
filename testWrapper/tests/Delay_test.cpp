#include "lib/Audio.h"
#include "lib/ComponentTest.h"
#include "lib/Delay.h"
#include "lib/utils.h"
#include <memory>

int main() {
  auto set = NtFx::ComponentTestSet(std::string(testFileBaseName(__FILE__)));

  auto shortDelayStorage = std::make_unique<NtFx::Delay::Short<50.0>>();
  auto& shortDelay       = *shortDelayStorage;
  shortDelay.t_ms        = 10;
  NTFX_ADD_TEST(set, shortDelay, "impulse");

  auto longDelayStorage = std::make_unique<NtFx::Delay::Long<80.0>>();
  auto& longDelay       = *longDelayStorage;
  longDelay.t_ms        = 30;
  NTFX_ADD_TEST(set, longDelay, "impulse");

  auto shortGlidedDelayStorage =
      std::make_unique<NtFx::Delay::ShortGlided<50.0>>();
  auto& shortGlidedDelay = *shortGlidedDelayStorage;
  shortGlidedDelay.reset(NTFX_FS);
  shortGlidedDelay.t_ms.ui = 15;
  for (int i = 0; i < 9600; ++i) { shortGlidedDelay.process(0); }
  NTFX_ADD_TEST(set, shortGlidedDelay, "impulse");
  NTFX_ADD_TEST(set, shortGlidedDelay, "linearSweep");

  auto longGlidedDelayStorage =
      std::make_unique<NtFx::Delay::LongGlided<80.0>>();
  auto& longGlidedDelay = *longGlidedDelayStorage;
  longGlidedDelay.reset(NTFX_FS);
  longGlidedDelay.t_ms.ui = 40;
  for (int i = 0; i < 9600; ++i) { longGlidedDelay.process(0); }
  NTFX_ADD_TEST(set, longGlidedDelay, "impulse");
  NTFX_ADD_TEST(set, longGlidedDelay, "linearSweep");

  auto fractDelayStorage = std::make_unique<NtFx::Delay::ShortFract<2400>>();
  auto& fractDelay       = *fractDelayStorage;
  fractDelay.t_ms        = 20.5;
  NTFX_ADD_TEST(set, fractDelay, "impulse");

  auto modDelayStorage  = std::make_unique<NtFx::Delay::Mod>();
  auto& modDelay        = *modDelayStorage;
  modDelay.fMod_hz.ui   = 0.5;
  modDelay.depth_p      = 20;
  modDelay.phaseMod_deg = 90;
  NTFX_ADD_TEST(set, modDelay, "linearSweep");

  auto t_ms = NtFx::primespace<signal_t>(5, 97, 8);

  auto _multiTap = std::make_unique<NtFx::Delay::LongMultiTap<100.0, 8>>();
  auto& multiTap = *_multiTap;
  for (size_t i = 0; i < 8; i++) {
    multiTap.t_ms[i] = t_ms[i];
    std::cout << "LongMultiTap " << i << ": " << t_ms[i] << " ms.\n";
  }
  NTFX_ADD_TEST(set, multiTap, "impulse");

  auto _multiTapDecay = std::make_unique<NtFx::Delay::LongMultiTap<100.0, 8>>();
  auto& multiTapDecay = *_multiTapDecay;
  for (size_t i = 0; i < 8; i++) { multiTapDecay.t_ms[i] = t_ms[i]; }
  multiTapDecay.setGainToDecay();
  NTFX_ADD_TEST(set, multiTapDecay, "impulse");

  return set.runAllTests();
}
