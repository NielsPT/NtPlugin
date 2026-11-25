
#include "lib/ComponentTest.h"
#include "plugins/ntMoritzKleinReverb.h"
#include <memory>

int main() {
  auto set = NtFx::ComponentTestSet(std::string(testFileBaseName(__FILE__)));
  auto bypass_ = std::make_unique<ntMoritzKleinReverb>();
  auto& bypass = *bypass_;
  bypass.bypassEnable = true;
  NTFX_ADD_TEST(set, bypass, "impulse");
  auto defaults_ = std::make_unique<ntMoritzKleinReverb>();
  auto& defaults = *defaults_;
  NTFX_ADD_TEST(set, defaults, "impulse");
  return set.runAllTests();
}
