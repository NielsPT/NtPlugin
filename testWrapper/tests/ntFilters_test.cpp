#include "lib/ComponentTest.h"
#include "plugins/ntFilters.h"
#include <memory>

int main() {
  auto set = NtFx::ComponentTestSet(std::string(testFileBaseName(__FILE__)));

  // Filters with constructor defaults
  auto filtersDefaultsStorage = std::make_unique<ntFilters>();
  auto& filtersDefaults       = *filtersDefaultsStorage;
  NTFX_ADD_TEST(set, filtersDefaults, "impulse");

  // Filters with default first-order settings
  auto filtersFirstOrderStorage = std::make_unique<ntFilters>();
  auto& filtersFirstOrder       = *filtersFirstOrderStorage;
  filtersFirstOrder.orderHpf    = Order::first;
  filtersFirstOrder.orderLpf    = Order::first;
  filtersFirstOrder.fHpf        = 100;
  filtersFirstOrder.fLpf        = 10000;
  NTFX_ADD_TEST(set, filtersFirstOrder, "impulse");

  // Filters with second-order (Butterworth)
  auto filtersSecondStorage = std::make_unique<ntFilters>();
  auto& filtersSecond       = *filtersSecondStorage;
  filtersSecond.orderHpf    = Order::second;
  filtersSecond.orderLpf    = Order::second;
  filtersSecond.fHpf        = 100;
  filtersSecond.fLpf        = 10000;
  // filtersSecond.qHpf        = 0.707;
  // filtersSecond.qLpf        = 0.707;
  NTFX_ADD_TEST(set, filtersSecond, "impulse");

  // Filters with third-order Butterworth settings (Q = 1)
  auto filtersThirdStorage = std::make_unique<ntFilters>();
  auto& filtersThird       = *filtersThirdStorage;
  filtersThird.orderHpf    = Order::third;
  filtersThird.orderLpf    = Order::third;
  filtersThird.fHpf        = 100;
  filtersThird.fLpf        = 10000;
  // filtersThird.qHpf        = 1.0;
  // filtersThird.qLpf        = 1.0;
  NTFX_ADD_TEST(set, filtersThird, "impulse");

  // Fourth-order filters with low Q
  auto filtersFourthLowQStorage = std::make_unique<ntFilters>();
  auto& filtersFourthLowQ       = *filtersFourthLowQStorage;
  filtersFourthLowQ.orderHpf    = Order::fourth;
  filtersFourthLowQ.orderLpf    = Order::fourth;
  filtersFourthLowQ.fHpf        = 100;
  filtersFourthLowQ.fLpf        = 10000;
  filtersFourthLowQ.qHpf        = 0.4;
  filtersFourthLowQ.qLpf        = 0.4;
  NTFX_ADD_TEST(set, filtersFourthLowQ, "impulse");

  // Fourth-order filters with high Q
  auto filtersFourthHighQStorage = std::make_unique<ntFilters>();
  auto& filtersFourthHighQ       = *filtersFourthHighQStorage;
  filtersFourthHighQ.orderHpf    = Order::fourth;
  filtersFourthHighQ.orderLpf    = Order::fourth;
  filtersFourthHighQ.fHpf        = 100;
  filtersFourthHighQ.fLpf        = 10000;
  filtersFourthHighQ.qHpf        = 1.2;
  filtersFourthHighQ.qLpf        = 1.2;
  NTFX_ADD_TEST(set, filtersFourthHighQ, "impulse");

  // Filters with high-pass steep slope
  auto filtersHpfSteepStorage = std::make_unique<ntFilters>();
  auto& filtersHpfSteep       = *filtersHpfSteepStorage;
  filtersHpfSteep.enableLpf   = false;
  filtersHpfSteep.orderHpf    = Order::fourth;
  filtersHpfSteep.fHpf        = 100;
  NTFX_ADD_TEST(set, filtersHpfSteep, "impulse");

  // Filters with low-pass steep slope
  auto filtersLpfSteepStorage = std::make_unique<ntFilters>();
  auto& filtersLpfSteep       = *filtersLpfSteepStorage;
  filtersLpfSteep.enableHpf   = false;
  filtersLpfSteep.orderLpf    = Order::fourth;
  filtersLpfSteep.fLpf        = 10000;
  NTFX_ADD_TEST(set, filtersLpfSteep, "impulse");

  return set.runAllTests();
}
