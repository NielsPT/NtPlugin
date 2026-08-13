#include "lib/Biquad.h"
#include "lib/ComponentTest.h"
#include "plugins/ntEqualizer.h"

int main() {
  auto set    = NtFx::ComponentTestSet(std::string(testFileBaseName(__FILE__)));
  auto bypass = ntEqualizer();
  bypass.bypassEnable = true;
  NTFX_ADD_TEST(set, bypass, "impulse");
  auto defaults = ntEqualizer();
  NTFX_ADD_TEST(set, defaults, "impulse");
  auto hpfLpf1st     = ntEqualizer();
  hpfLpf1st.orderHpf = Order::first;
  hpfLpf1st.orderLpf = Order::first;
  hpfLpf1st.fHpf_hz  = 200;
  hpfLpf1st.fLpf_hz  = 2000;
  NTFX_ADD_TEST(set, hpfLpf1st, "impulse");
  auto hpfLpf2nd     = ntEqualizer();
  hpfLpf2nd.orderHpf = Order::second;
  hpfLpf2nd.orderLpf = Order::second;
  hpfLpf2nd.fHpf_hz  = 200;
  hpfLpf2nd.fLpf_hz  = 2000;
  NTFX_ADD_TEST(set, hpfLpf2nd, "impulse");
  auto hpfLpf3rd     = ntEqualizer();
  hpfLpf3rd.orderHpf = Order::third;
  hpfLpf3rd.orderLpf = Order::third;
  hpfLpf3rd.fHpf_hz  = 200;
  hpfLpf3rd.fLpf_hz  = 2000;
  NTFX_ADD_TEST(set, hpfLpf3rd, "impulse");
  auto hpfLpf4th     = ntEqualizer();
  hpfLpf4th.orderHpf = Order::fourth;
  hpfLpf4th.orderLpf = Order::fourth;
  hpfLpf4th.fHpf_hz  = 200;
  hpfLpf4th.fLpf_hz  = 2000;
  NTFX_ADD_TEST(set, hpfLpf4th, "impulse");
  auto shelf = ntEqualizer();
  // shelf.cascade.settings[fLo].shape   = NtFx::Biquad::Shape::loShelf;
  shelf.cascade.settings[fLo].fc_hz   = 100;
  shelf.cascade.settings[fLo].gain_db = 12;
  shelf.cascade.settings[fHi].fc_hz   = 4e3;
  shelf.cascade.settings[fHi].gain_db = 12;
  NTFX_ADD_TEST(set, shelf, "impulse");
  auto bell                            = ntEqualizer();
  bell.cascade.settings[loMid].gain_db = -12;
  bell.cascade.settings[loMid].fc_hz   = 50;
  bell.cascade.settings[loMid].q       = 2;
  NTFX_ADD_TEST(set, bell, "impulse");
  return set.runAllTests();
}
