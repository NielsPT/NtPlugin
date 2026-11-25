
#include "lib/ComponentTest.h"
#include "plugins/ntKleinReverb.h"
#include <memory>

int main() {
  auto set = NtFx::ComponentTestSet(std::string(testFileBaseName(__FILE__)));
  auto bypass_        = std::make_unique<ntKleinReverb>();
  auto& bypass        = *bypass_;
  bypass.bypassEnable = true;
  NTFX_ADD_TEST(set, bypass, "impulse");
  auto defaults_ = std::make_unique<ntKleinReverb>();
  auto& defaults = *defaults_;
  NTFX_ADD_TEST(set, defaults, "impulse");
  auto shortTail_  = std::make_unique<ntKleinReverb>();
  auto& shortTail  = *shortTail_;
  shortTail.size_m = 5;
  shortTail.t60_s  = 0.1;
  NTFX_ADD_TEST(set, shortTail, "impulse");
  return set.runAllTests();
}
