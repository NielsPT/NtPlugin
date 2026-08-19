#include "lib/Biquad.h"
#include "lib/ComponentTest.h"
#include "plugins/ntEqualizer.h"
#include <memory>

int main() {
  auto set = NtFx::ComponentTestSet(std::string(testFileBaseName(__FILE__)));
  auto bypassStorage  = std::make_unique<ntEqualizer>();
  auto& bypass        = *bypassStorage;
  bypass.bypassEnable = true;
  NTFX_ADD_TEST(set, bypass, "impulse");
  auto defaultsStorage = std::make_unique<ntEqualizer>();
  auto& defaults       = *defaultsStorage;
  NTFX_ADD_TEST(set, defaults, "impulse");
  auto hpfLpf1stStorage = std::make_unique<ntEqualizer>();
  auto& hpfLpf1st       = *hpfLpf1stStorage;
  hpfLpf1st.orderHpf    = Order::first;
  hpfLpf1st.orderLpf    = Order::first;
  hpfLpf1st.fHpf_hz     = 200;
  hpfLpf1st.fLpf_hz     = 2000;
  NTFX_ADD_TEST(set, hpfLpf1st, "impulse");
  auto hpfLpf2ndStorage = std::make_unique<ntEqualizer>();
  auto& hpfLpf2nd       = *hpfLpf2ndStorage;
  hpfLpf2nd.orderHpf    = Order::second;
  hpfLpf2nd.orderLpf    = Order::second;
  hpfLpf2nd.fHpf_hz     = 200;
  hpfLpf2nd.fLpf_hz     = 2000;
  NTFX_ADD_TEST(set, hpfLpf2nd, "impulse");
  auto hpfLpf3rdStorage = std::make_unique<ntEqualizer>();
  auto& hpfLpf3rd       = *hpfLpf3rdStorage;
  hpfLpf3rd.orderHpf    = Order::third;
  hpfLpf3rd.orderLpf    = Order::third;
  hpfLpf3rd.fHpf_hz     = 200;
  hpfLpf3rd.fLpf_hz     = 2000;
  NTFX_ADD_TEST(set, hpfLpf3rd, "impulse");
  auto hpfLpf4thStorage = std::make_unique<ntEqualizer>();
  auto& hpfLpf4th       = *hpfLpf4thStorage;
  hpfLpf4th.orderHpf    = Order::fourth;
  hpfLpf4th.orderLpf    = Order::fourth;
  hpfLpf4th.fHpf_hz     = 200;
  hpfLpf4th.fLpf_hz     = 2000;
  NTFX_ADD_TEST(set, hpfLpf4th, "impulse");
  auto shelfStorage = std::make_unique<ntEqualizer>();
  auto& shelf       = *shelfStorage;
  // shelf.cascade.settings[fLo].shape   = NtFx::Biquad::Shape::loShelf;
  shelf.cascade.settings[fLo].fc_hz   = 100;
  shelf.cascade.settings[fLo].gain_db = 12;
  shelf.cascade.settings[fHi].fc_hz   = 4e3;
  shelf.cascade.settings[fHi].gain_db = 12;
  NTFX_ADD_TEST(set, shelf, "impulse");
  auto bellStorage                     = std::make_unique<ntEqualizer>();
  auto& bell                           = *bellStorage;
  bell.cascade.settings[loMid].gain_db = -12;
  bell.cascade.settings[loMid].fc_hz   = 50;
  bell.cascade.settings[loMid].q       = 2;
  NTFX_ADD_TEST(set, bell, "impulse");
  return set.runAllTests();
}
