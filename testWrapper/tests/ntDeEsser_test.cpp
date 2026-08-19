#include "lib/ComponentTest.h"
#include "plugins/ntDeEsser.h"
#include <memory>

int main() {
  auto set = NtFx::ComponentTestSet(std::string(testFileBaseName(__FILE__)));

  // De-esser with constructor defaults
  auto defaultsStorage = std::make_unique<ntDeEsser>();
  auto& defaults       = *defaultsStorage;
  NTFX_ADD_TEST(set, defaults, "impulse");

  // De-esser bypass
  auto bypassStorage  = std::make_unique<ntDeEsser>();
  auto& bypass        = *bypassStorage;
  bypass.bypassEnable = true;
  NTFX_ADD_TEST(set, bypass, "impulse");

  auto shelfStorage           = std::make_unique<ntDeEsser>();
  auto& shelf                 = *shelfStorage;
  shelf.mode                  = Mode::shelf;
  shelf.fc_hz                 = 10e3;
  shelf.sc.settings.thresh_db = -6;
  shelf.sc.settings.tRel_ms   = 10;
  NTFX_ADD_TEST(set, shelf, "dynamic_alternating");
  NTFX_ADD_TEST(set, shelf, "dynamic_matched");
  NTFX_ADD_TEST(set, shelf, "impulse");
  auto bellStorage           = std::make_unique<ntDeEsser>();
  auto& bell                 = *bellStorage;
  bell.mode                  = Mode::bell;
  bell.fc_hz                 = 10e3;
  bell.sc.settings.thresh_db = -6;
  bell.sc.settings.tRel_ms   = 10;
  NTFX_ADD_TEST(set, bell, "dynamic_alternating");
  NTFX_ADD_TEST(set, bell, "dynamic_matched");
  NTFX_ADD_TEST(set, bell, "impulse");
  auto bell1Storage           = std::make_unique<ntDeEsser>();
  auto& bell1                 = *bell1Storage;
  bell1.mode                  = Mode::bell;
  bell1.fc_hz                 = 10e3;
  bell1.sc.settings.thresh_db = -6;
  bell1.sc.settings.tRel_ms   = 10;
  bell1.dl.t_ms               = 0;
  NTFX_ADD_TEST(set, bell1, "dynamic_matched");
  auto scHpfStorage    = std::make_unique<ntDeEsser>();
  auto& scHpf          = *scHpfStorage;
  scHpf.mode           = Mode::shelf;
  scHpf.scListenEnable = true;
  NTFX_ADD_TEST(set, scHpf, "impulse");
  auto scBpfStorage    = std::make_unique<ntDeEsser>();
  auto& scBpf          = *scBpfStorage;
  scBpf.mode           = Mode::bell;
  scBpf.scListenEnable = true;
  NTFX_ADD_TEST(set, scBpf, "impulse");
  return set.runAllTests();
}
