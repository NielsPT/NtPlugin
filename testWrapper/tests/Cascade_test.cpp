#include "lib/Biquad.h"
#include "lib/ComponentTest.h"
#include <memory>

int main() {
  auto set = NtFx::ComponentTestSet(std::string(testFileBaseName(__FILE__)));
  auto cascadeSingle_ = std::make_unique<NtFx::Biquad::Cascade<1>>();
  auto& cascadeSingle = *cascadeSingle_;
  cascadeSingle.settings[0].shape = NtFx::Biquad::Shape::notch;
  NTFX_ADD_TEST(set, cascadeSingle, "impulse");
  auto cascade8notch_ = std::make_unique<NtFx::Biquad::Cascade<8>>();
  auto& cascade8notch = *cascade8notch_;
  for (size_t i = 0; i < 8; i++) {
    cascade8notch.settings[i].shape = NtFx::Biquad::Shape::notch;
    cascade8notch.settings[i].q     = 10;
    cascade8notch.settings[i].fc_hz = 100 * signal_t(i * i);
  }
  NTFX_ADD_TEST(set, cascade8notch, "impulse");
  return set.runAllTests();
}