
#include "lib/ComponentTest.h"
#include "lib/utils.h"
#include "plugins/ntTilt.h"
#include <format>
#include <memory>
#include <string>
#include <vector>

int main() {
  auto set = NtFx::ComponentTestSet(std::string(testFileBaseName(__FILE__)));
#if 1
  auto bypass_        = std::make_unique<ntTilt>();
  auto& bypass        = *bypass_;
  bypass.bypassEnable = true;
  NTFX_ADD_TEST(set, bypass, "impulse");
  auto defaults_ = std::make_unique<ntTilt>();
  auto& defaults = *defaults_;
  NTFX_ADD_TEST(set, defaults, "impulse");
  auto plus6_          = std::make_unique<ntTilt>();
  auto& plus6          = *plus6_;
  plus6.filter.tilt_db = 6;
  NTFX_ADD_TEST(set, plus6, "impulse");
  auto minus6_          = std::make_unique<ntTilt>();
  auto& minus6          = *minus6_;
  minus6.filter.tilt_db = -6;
  NTFX_ADD_TEST(set, minus6, "impulse");
  auto plus12_          = std::make_unique<ntTilt>();
  auto& plus12          = *plus12_;
  plus12.filter.tilt_db = 12;
  NTFX_ADD_TEST(set, plus12, "impulse");
  auto minus12_          = std::make_unique<ntTilt>();
  auto& minus12          = *minus12_;
  minus12.filter.tilt_db = -12;
  NTFX_ADD_TEST(set, minus12, "impulse");
#endif
#if 0
  ntTilt t;
  std::vector<std::unique_ptr<
      NtFx::FirstOrder::StereoFilter<NtFx::FirstOrder::Shape::hpf>>>
      v;
  for (size_t i = 0; i < nStages; i++) {
    auto p = std::make_unique<
        NtFx::FirstOrder::StereoFilter<NtFx::FirstOrder::Shape::hpf>>();
    p->fc_hz = t.filter.filters[i].fc_hz;
    std::cout << t.filter.filters[i].fc_hz << "\n";
    v.push_back(std::move(p));
  }
  for (size_t i = 0; i < nStages; i++) {
    auto p = v[i].get();
    set.addTest(p, std::format("Hpf{}_{}", i, int(p->fc_hz)), { "impulse" });
  }
#endif
  return set.runAllTests();
}
