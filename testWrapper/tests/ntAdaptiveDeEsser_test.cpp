
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
  // NTFX_ADD_TEST(set, defaults, "dynamic_matched");
  auto red50p_ = std::make_unique<ntAdaptiveDeEsser>();
  auto& red50p = *red50p_;
  red50p.red_p = 50;
  NTFX_ADD_TEST(set, red50p, "impulse");
  // NTFX_ADD_TEST(set, red50p, "dynamic_matched");
  auto red20p_ = std::make_unique<ntAdaptiveDeEsser>();
  auto& red20p = *red20p_;
  red20p.red_p = 20;
  NTFX_ADD_TEST(set, red20p, "impulse");
  // NTFX_ADD_TEST(set, red20p, "dynamic_matched");
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
  // NTFX_ADD_TEST(set, rmsDefaults, "dynamic_matched");
  auto rmsNoLookahead_        = std::make_unique<ntAdaptiveDeEsser>();
  auto& rmsNoLookahead        = *rmsNoLookahead_;
  rmsNoLookahead.sc.rmsEnable = true;
  rmsNoLookahead.dl.t_ms      = 0;
  // NTFX_ADD_TEST(set, rmsNoLookahead, "dynamic_matched");
  auto firstDefaults_ = std::make_unique<ntAdaptiveDeEsser>();
  auto& firstDefaults = *firstDefaults_;
  firstDefaults.order = ntAdaptiveDeEsser::e_first;
  NTFX_ADD_TEST(set, firstDefaults, "impulse");
  NTFX_ADD_TEST(set, firstDefaults, "dynamic_matched");
  NTFX_ADD_TEST(set, firstDefaults, "whiteNoise");
  auto firstHard_        = std::make_unique<ntAdaptiveDeEsser>();
  auto& firstHard        = *firstHard_;
  firstHard.order        = ntAdaptiveDeEsser::e_first;
  firstHard.sc.offset_db = -24;
  NTFX_ADD_TEST(set, firstHard, "impulse");
  NTFX_ADD_TEST(set, firstHard, "dynamic_matched");
  NTFX_ADD_TEST(set, firstHard, "whiteNoise");
  NTFX_ADD_TEST(set, firstHard, "pinkNoise");
  auto firstRmsHard_        = std::make_unique<ntAdaptiveDeEsser>();
  auto& firstRmsHard        = *firstRmsHard_;
  firstRmsHard.order        = ntAdaptiveDeEsser::e_first;
  firstRmsHard.sc.rmsEnable = true;
  firstRmsHard.sc.offset_db = -24;
  NTFX_ADD_TEST(set, firstRmsHard, "impulse");
  NTFX_ADD_TEST(set, firstRmsHard, "dynamic_matched");
  NTFX_ADD_TEST(set, firstRmsHard, "whiteNoise");
  NTFX_ADD_TEST(set, firstRmsHard, "pinkNoise");
  return set.runAllTests();
}
