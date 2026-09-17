
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
  auto plus5_          = std::make_unique<ntTilt>();
  auto& plus5          = *plus5_;
  plus5.filter.tilt_db = 5;
  NTFX_ADD_TEST(set, plus5, "impulse");
  auto minus5_          = std::make_unique<ntTilt>();
  auto& minus5          = *minus5_;
  minus5.filter.tilt_db = -5;
  NTFX_ADD_TEST(set, minus5, "impulse");
  auto plus10_          = std::make_unique<ntTilt>();
  auto& plus10          = *plus10_;
  plus10.filter.tilt_db = 10;
  NTFX_ADD_TEST(set, plus10, "impulse");
  auto minus10_          = std::make_unique<ntTilt>();
  auto& minus10          = *minus10_;
  minus10.filter.tilt_db = -10;
  NTFX_ADD_TEST(set, minus10, "impulse");
  auto plus20_          = std::make_unique<ntTilt>();
  auto& plus20          = *plus20_;
  plus20.filter.tilt_db = 20;
  NTFX_ADD_TEST(set, plus20, "impulse");
  auto minus20_          = std::make_unique<ntTilt>();
  auto& minus20          = *minus20_;
  minus20.filter.tilt_db = -20;
  NTFX_ADD_TEST(set, minus20, "impulse");
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
