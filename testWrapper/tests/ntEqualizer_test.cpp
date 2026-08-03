#include "lib/Biquad.h"
#include "lib/ComponentTest.h"
#include "plugins/ntEqualizer.h"

NTFX_TEST_BEGIN

NTFX_TEST() {
  auto bypass         = ntEqualizer();
  bypass.bypassEnable = true;
  NTFX_ADD_TEST(bypass, "impulse");
  auto defaults = ntEqualizer();
  NTFX_ADD_TEST(defaults, "impulse");
  auto individualBypass    = ntEqualizer();
  individualBypass.fHpf_hz = 1e3;
  individualBypass.fLpf_hz = 1e3;
  for (size_t i = 0; i < Bands::n; i++) {
    individualBypass.cascade.settings[i].gain_db = -12;
    individualBypass.cascade.settings[i].q       = 10;
  }
  individualBypass.orderHpf = Order::none;
  individualBypass.orderLpf = Order::none;
  for (size_t i = 0; i < Bands::n; i++) { individualBypass.bypasses[i] = true; }
  NTFX_ADD_TEST(individualBypass, "impulse");
  auto hpfLpf1st     = ntEqualizer();
  hpfLpf1st.orderHpf = Order::first;
  hpfLpf1st.orderLpf = Order::first;
  hpfLpf1st.fHpf_hz  = 200;
  hpfLpf1st.fLpf_hz  = 2000;
  NTFX_ADD_TEST(hpfLpf1st, "impulse");
  auto hpfLpf2nd     = ntEqualizer();
  hpfLpf2nd.orderHpf = Order::second;
  hpfLpf2nd.orderLpf = Order::second;
  hpfLpf2nd.fHpf_hz  = 200;
  hpfLpf2nd.fLpf_hz  = 2000;
  NTFX_ADD_TEST(hpfLpf2nd, "impulse");
  auto hpfLpf3rd     = ntEqualizer();
  hpfLpf3rd.orderHpf = Order::third;
  hpfLpf3rd.orderLpf = Order::third;
  hpfLpf3rd.fHpf_hz  = 200;
  hpfLpf3rd.fLpf_hz  = 2000;
  NTFX_ADD_TEST(hpfLpf3rd, "impulse");
  auto hpfLpf4th     = ntEqualizer();
  hpfLpf4th.orderHpf = Order::fourth;
  hpfLpf4th.orderLpf = Order::fourth;
  hpfLpf4th.fHpf_hz  = 200;
  hpfLpf4th.fLpf_hz  = 2000;
  NTFX_ADD_TEST(hpfLpf4th, "impulse");
  auto shelf                         = ntEqualizer();
  shelf.cascade.settings[nHpf].shape = NtFx::Biquad::Shape::loShelf;
  shelf.cascade.settings[nHpf].fc_hz = 100;
  shelf.cascade.settings[nHpf + Bands::n - 1].fc_hz   = 4e3;
  shelf.cascade.settings[nHpf].gain_db                = 12;
  shelf.cascade.settings[nHpf + Bands::n - 1].gain_db = 12;
  NTFX_ADD_TEST(shelf, "impulse");
  auto bell                               = ntEqualizer();
  bell.cascade.settings[nHpf + 1].gain_db = -12;
  bell.cascade.settings[nHpf + 1].fc_hz   = 50;
  bell.cascade.settings[nHpf + 1].q       = 2;
  NTFX_ADD_TEST(bell, "impulse");
  return NTFX_RUN_TESTS();
}
