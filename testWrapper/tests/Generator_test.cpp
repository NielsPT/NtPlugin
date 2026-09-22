#include "lib/Audio.h"
#include "lib/ComponentTest.h"
#include "lib/Generator.h"
#include <memory>

int main() {
  auto set = NtFx::ComponentTestSet(std::string(testFileBaseName(__FILE__)));
  auto noiseStorage = std::make_unique<NtFx::Generator::WhiteNoise>();
  auto& noise       = *noiseStorage;
  NTFX_ADD_TEST(set, noise, "impulse");
  auto filteredNoiseDefaultStorage =
      std::make_unique<NtFx::Generator::FilteredWhiteNoise>();
  auto& filteredNoiseDefault = *filteredNoiseDefaultStorage;
  NTFX_ADD_TEST(set, filteredNoiseDefault, "impulse");
  auto filteredNoiseStorage =
      std::make_unique<NtFx::Generator::FilteredWhiteNoise>();
  auto& filteredNoise     = *filteredNoiseStorage;
  filteredNoise.lpf.fc_hz = 1e3;
  NTFX_ADD_TEST(set, filteredNoise, "impulse");
  auto pinkNoiseStorage = std::make_unique<NtFx::Generator::PinkNoise>();
  auto& pinkNoise       = *pinkNoiseStorage;
  NTFX_ADD_TEST(set, pinkNoise, "impulse");
  auto filteredPinkNoiseStorage =
      std::make_unique<NtFx::Generator::FilteredPinkNoise>();
  auto& filteredPinkNoise = *filteredPinkNoiseStorage;
  NTFX_ADD_TEST(set, filteredPinkNoise, "impulse");
  auto Sin1kStorage = std::make_unique<NtFx::Generator::Sin>();
  auto& Sin1k       = *Sin1kStorage;
  NTFX_ADD_TEST(set, Sin1k, "impulse");
  auto Sin100Storage = std::make_unique<NtFx::Generator::Sin>();
  auto& Sin100       = *Sin100Storage;
  Sin100.f_hz        = 100;
  NTFX_ADD_TEST(set, Sin100, "impulse");
  auto Sin100ph90Storage = std::make_unique<NtFx::Generator::Sin>();
  auto& Sin100ph90       = *Sin100ph90Storage;
  Sin100ph90.f_hz        = 100;
  Sin100ph90.ph_rad      = NTFX_PI / 2;
  NTFX_ADD_TEST(set, Sin100ph90, "impulse");
  auto Saw100Storage = std::make_unique<NtFx::Generator::Saw>();
  auto& Saw100       = *Saw100Storage;
  Saw100.f_hz        = 100;
  NTFX_ADD_TEST(set, Saw100, "impulse");
  auto Saw100ph90Storage = std::make_unique<NtFx::Generator::Saw>();
  auto& Saw100ph90       = *Saw100ph90Storage;
  Saw100ph90.f_hz        = 100;
  Saw100ph90.ph_rad      = NTFX_PI / 2;
  NTFX_ADD_TEST(set, Saw100ph90, "impulse");
  auto square1kStorage = std::make_unique<NtFx::Generator::Square>();
  auto& square1k       = *square1kStorage;
  NTFX_ADD_TEST(set, square1k, "impulse");
  auto square100Storage = std::make_unique<NtFx::Generator::Square>();
  auto& square100       = *square100Storage;
  square100.f_hz        = 100;
  NTFX_ADD_TEST(set, square100, "impulse");
  return set.runAllTests();
}