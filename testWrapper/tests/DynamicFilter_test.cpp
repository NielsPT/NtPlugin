#include "lib/ComponentTest.h"
#include "lib/DynamicFilter.h"
#include <memory>

int main() {
  auto set = NtFx::ComponentTestSet(std::string(testFileBaseName(__FILE__)));
  auto flatStorage = std::make_unique<NtFx::DynamicFilter::ShelfFixedPoles>();
  auto& flat       = *flatStorage;
  flat.gain_lin    = 1;
  NTFX_ADD_TEST(set, flat, "impulse");
  auto minus6Storage = std::make_unique<NtFx::DynamicFilter::ShelfFixedPoles>();
  auto& minus6       = *minus6Storage;
  minus6.gain_lin    = 0.5;
  NTFX_ADD_TEST(set, minus6, "impulse");
  auto minus12Storage =
      std::make_unique<NtFx::DynamicFilter::ShelfFixedPoles>();
  auto& minus12    = *minus12Storage;
  minus12.gain_lin = 0.25;
  NTFX_ADD_TEST(set, minus12, "impulse");
  auto minus6lowQ2Storage =
      std::make_unique<NtFx::DynamicFilter::ShelfFixedPoles>();
  auto& minus6lowQ2    = *minus6lowQ2Storage;
  minus6lowQ2.gain_lin = 0.5;
  minus6lowQ2.q1       = 2;
  NTFX_ADD_TEST(set, minus6lowQ2, "impulse");
  auto minus12lowQ2Storage =
      std::make_unique<NtFx::DynamicFilter::ShelfFixedPoles>();
  auto& minus12lowQ2    = *minus12lowQ2Storage;
  minus12lowQ2.gain_lin = 0.25;
  minus12lowQ2.q1       = 2;
  NTFX_ADD_TEST(set, minus12lowQ2, "impulse");
  auto minus6bothQ05Storage =
      std::make_unique<NtFx::DynamicFilter::ShelfFixedPoles>();
  auto& minus6bothQ05    = *minus6bothQ05Storage;
  minus6bothQ05.gain_lin = 0.5;
  minus6bothQ05.q1       = 0.5;
  minus6bothQ05.q2       = 0.5;
  NTFX_ADD_TEST(set, minus6bothQ05, "impulse");
  auto minus12bothQ05Storage =
      std::make_unique<NtFx::DynamicFilter::ShelfFixedPoles>();
  auto& minus12bothQ05    = *minus12bothQ05Storage;
  minus12bothQ05.gain_lin = 0.25;
  minus12bothQ05.q1       = 0.5;
  minus12bothQ05.q2       = 0.5;
  NTFX_ADD_TEST(set, minus12bothQ05, "impulse");
  auto minusInfStorage =
      std::make_unique<NtFx::DynamicFilter::ShelfFixedPoles>();
  auto& minusInf    = *minusInfStorage;
  minusInf.gain_lin = 0;
  NTFX_ADD_TEST(set, minusInf, "impulse");
  auto fixedZerosStorage =
      std::make_unique<NtFx::DynamicFilter::ShelfFixedZeros>();
  auto& fixedZeros    = *fixedZerosStorage;
  fixedZeros.gain_lin = 0.25;
  NTFX_ADD_TEST(set, fixedZeros, "impulse");
  return set.runAllTests();
}