#include "lib/ComponentTest.h"
#include "plugins/ntDeEsser.h"

int main() {
  auto set = NtFx::ComponentTestSet(std::string(testFileBaseName(__FILE__)));

  // auto defaults = ntDeEsser();
  // NTFX_ADD_TEST(set, defaults, "dynamic_alternating");
  // NTFX_ADD_TEST(set, defaults, "impulse");
  auto shelf                  = ntDeEsser();
  shelf.mode                  = Mode::shelf;
  shelf.fc_hz                 = 10e3;
  shelf.sc.settings.thresh_db = -6;
  shelf.sc.settings.tRel_ms   = 10;
  NTFX_ADD_TEST(set, shelf, "dynamic_alternating");
  NTFX_ADD_TEST(set, shelf, "dynamic_matched");
  NTFX_ADD_TEST(set, shelf, "impulse");
  auto bell                  = ntDeEsser();
  bell.mode                  = Mode::bell;
  bell.fc_hz                 = 10e3;
  bell.sc.settings.thresh_db = -6;
  bell.sc.settings.tRel_ms   = 10;
  NTFX_ADD_TEST(set, bell, "dynamic_alternating");
  NTFX_ADD_TEST(set, bell, "dynamic_matched");
  NTFX_ADD_TEST(set, bell, "impulse");
  auto bell1                  = ntDeEsser();
  bell1.mode                  = Mode::bell;
  bell1.fc_hz                 = 10e3;
  bell1.sc.settings.thresh_db = -6;
  bell1.sc.settings.tRel_ms   = 10;
  bell1.dl.t_ms               = 0;
  NTFX_ADD_TEST(set, bell1, "dynamic_matched");
  auto scHpf           = ntDeEsser();
  scHpf.mode           = Mode::shelf;
  scHpf.scListenEnable = true;
  NTFX_ADD_TEST(set, scHpf, "impulse");
  auto scBpf           = ntDeEsser();
  scBpf.mode           = Mode::bell;
  scBpf.scListenEnable = true;
  NTFX_ADD_TEST(set, scBpf, "impulse");
  return set.runAllTests();
}
