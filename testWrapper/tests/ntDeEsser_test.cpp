#include "lib/ComponentTest.h"
#include "plugins/ntDeEsser.h"

NTFX_TEST_BEGIN

NTFX_TEST() {
  auto defaults = ntDeEsser();
  NTFX_ADD_TEST(defaults, "dynamic_alternating");
  NTFX_ADD_TEST(defaults, "impulse");
  auto lookahead_shelf                  = ntDeEsser();
  lookahead_shelf.mode                  = Mode::shelf;
  lookahead_shelf.fc_hz                 = 5e3;
  lookahead_shelf.sc.settings.thresh_db = -6;
  lookahead_shelf.lookaheadEnable       = true;
  lookahead_shelf.dl.t_ms               = 0.25;
  NTFX_ADD_TEST(lookahead_shelf, "dynamic_alternating");
  NTFX_ADD_TEST(lookahead_shelf, "impulse");
  auto lookahead_bell                  = ntDeEsser();
  lookahead_bell.sc.settings.thresh_db = -6;
  lookahead_bell.lookaheadEnable       = true;
  lookahead_bell.scSettings.linkEnable = true;
  lookahead_bell.mode                  = Mode::bell;
  lookahead_bell.fc_hz                 = 10e3;
  lookahead_bell.dl.t_ms               = 0.25;
  NTFX_ADD_TEST(lookahead_bell, "dynamic_alternating");
  NTFX_ADD_TEST(lookahead_bell, "impulse");
  auto scHpf           = ntDeEsser();
  scHpf.mode           = Mode::shelf;
  scHpf.scListenEnable = true;
  NTFX_ADD_TEST(scHpf, "impulse");
  auto scBpf           = ntDeEsser();
  scBpf.mode           = Mode::bell;
  scBpf.scListenEnable = true;
  NTFX_ADD_TEST(scBpf, "impulse");
  return NTFX_RUN_TESTS();
}
