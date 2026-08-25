
#include "lib/ComponentTest.h"
#include "plugins/ntAdaptiveDeEsser.h"
#include <memory>

int main() {
  auto set = NtFx::ComponentTestSet(std::string(testFileBaseName(__FILE__)));
  auto bypass_        = std::make_unique<ntAdaptiveDeEsser>();
  auto& bypass        = *bypass_;
  bypass.bypassEnable = true;
  NTFX_ADD_TEST(set, bypass, "impulse");
  auto defaults_ = std::make_unique<ntAdaptiveDeEsser>();
  auto& defaults = *defaults_;
  NTFX_ADD_TEST(set, defaults, "impulse");
  NTFX_ADD_TEST(set, defaults, "dynamic_matched");
  auto red50pStorage = std::make_unique<ntAdaptiveDeEsser>();
  auto& red50p       = *red50pStorage;
  red50p.red_p       = 50;
  NTFX_ADD_TEST(set, red50p, "impulse");
  NTFX_ADD_TEST(set, red50p, "dynamic_matched");
  auto red20pStorage = std::make_unique<ntAdaptiveDeEsser>();
  auto& red20p       = *red20pStorage;
  red20p.red_p       = 20;
  NTFX_ADD_TEST(set, red20p, "impulse");
  NTFX_ADD_TEST(set, red20p, "dynamic_matched");
  auto red70pStorage = std::make_unique<ntAdaptiveDeEsser>();
  auto& red70p       = *red70pStorage;
  red70p.red_p       = 70;
  NTFX_ADD_TEST(set, red70p, "impulse");
  auto red0pStorage = std::make_unique<ntAdaptiveDeEsser>();
  auto& red0p       = *red0pStorage;
  red0p.red_p       = 0;
  NTFX_ADD_TEST(set, red0p, "impulse");
  return set.runAllTests();
}
