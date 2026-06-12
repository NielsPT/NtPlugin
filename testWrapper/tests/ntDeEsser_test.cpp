#include "lib/ComponentTest.h"
#include "plugins/ntDeEsser.h"

NTFX_TEST_BEGIN

NTFX_TEST() {
  auto bypass         = ntDeEsser();
  bypass.bypassEnable = true;
  NTFX_ADD_TEST(bypass, "impulse");
  auto defaults = ntDeEsser();
  NTFX_ADD_TEST(defaults, "impulse");
  defaults.sc.settings.thresh_db = -6;
  // NTFX_ADD_TEST(defaults, "dynamic_matched");
  auto lookahead_bell            = defaults;
  lookahead_bell.lookaheadEnable = true;
  lookahead_bell.mode            = Mode::bell;
  lookahead_bell.fc_hz           = 10e3;
  lookahead_bell.dl.t_ms         = 0.25;
  NTFX_ADD_TEST(lookahead_bell, "dynamic_matched");
  NTFX_ADD_TEST(lookahead_bell, "impulse");
  auto lookahead_shelf  = lookahead_bell;
  lookahead_shelf.mode  = Mode::shelf;
  lookahead_shelf.fc_hz = 5e3;
  NTFX_ADD_TEST(lookahead_shelf, "dynamic_matched");
  NTFX_ADD_TEST(lookahead_shelf, "impulse");
  return NTFX_RUN_TESTS();
}
