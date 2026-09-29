
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
  auto red50p_ = std::make_unique<ntAdaptiveDeEsser>();
  auto& red50p = *red50p_;
  red50p.red_p = 50;
  NTFX_ADD_TEST(set, red50p, "impulse");
  NTFX_ADD_TEST(set, red50p, "dynamic_matched");
  auto red20p_ = std::make_unique<ntAdaptiveDeEsser>();
  auto& red20p = *red20p_;
  red20p.red_p = 20;
  NTFX_ADD_TEST(set, red20p, "impulse");
  NTFX_ADD_TEST(set, red20p, "dynamic_matched");
  auto red70p_ = std::make_unique<ntAdaptiveDeEsser>();
  auto& red70p = *red70p_;
  red70p.red_p = 70;
  NTFX_ADD_TEST(set, red70p, "impulse");
  auto red0p_ = std::make_unique<ntAdaptiveDeEsser>();
  auto& red0p = *red0p_;
  red0p.red_p = 0;
  NTFX_ADD_TEST(set, red0p, "impulse");
  auto rmsDefaults_        = std::make_unique<ntAdaptiveDeEsser>();
  auto& rmsDefaults        = *rmsDefaults_;
  rmsDefaults.sc.rmsEnable = true;
  NTFX_ADD_TEST(set, rmsDefaults, "dynamic_matched");
  auto rmsNoLookahead_        = std::make_unique<ntAdaptiveDeEsser>();
  auto& rmsNoLookahead        = *rmsNoLookahead_;
  rmsNoLookahead.sc.rmsEnable = true;
  rmsNoLookahead.dl.t_ms      = 0;
  NTFX_ADD_TEST(set, rmsNoLookahead, "dynamic_matched");
  return set.runAllTests();
}
