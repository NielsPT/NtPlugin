#include "lib/ComponentTest.h"
#include "plugins/ntDeEsser.h"

NTFX_TEST_BEGIN

NTFX_TEST() {
  // auto defaults = ntDeEsser();
  // NTFX_ADD_TEST(defaults, "dynamic_alternating");
  // NTFX_ADD_TEST(defaults, "impulse");
  auto shelf                  = ntDeEsser();
  shelf.mode                  = Mode::shelf;
  shelf.fc_hz                 = 10e3;
  shelf.sc.settings.thresh_db = -6;
  shelf.sc.settings.tRel_ms   = 10;
  NTFX_ADD_TEST(shelf, "dynamic_alternating");
  NTFX_ADD_TEST(shelf, "dynamic_matched");
  NTFX_ADD_TEST(shelf, "impulse");
  auto bell = shelf;
  bell.mode = Mode::bell;
  NTFX_ADD_TEST(bell, "dynamic_alternating");
  NTFX_ADD_TEST(bell, "dynamic_matched");
  NTFX_ADD_TEST(bell, "impulse");
  auto bell1    = bell;
  bell1.dl.t_ms = 0;
  NTFX_ADD_TEST(bell1, "dynamic_matched");
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
