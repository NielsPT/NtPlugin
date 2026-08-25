#include "lib/AdaptiveDeEssSc.h"
#include "lib/ComponentTest.h"
#include <memory>

int main() {
  auto set = NtFx::ComponentTestSet(std::string(testFileBaseName(__FILE__)));
  auto defaultsStorage = std::make_unique<NtFx::AdaptiveDeEssSc>();
  auto& defaults       = *defaultsStorage;
  NTFX_ADD_TEST(set, defaults, "dynamic_matched");
  return set.runAllTests();
}