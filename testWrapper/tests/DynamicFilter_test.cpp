#include "lib/ComponentTest.h"
#include "lib/DynamicFilter.h"

int main() {
  auto set  = NtFx::ComponentTestSet(std::string(testFileBaseName(__FILE__)));
  auto flat = NtFx::DynamicFilter::ShelfFixedPoles();
  flat.gain_lin = 1;
  NTFX_ADD_TEST(set, flat, "impulse");
  auto minus6     = NtFx::DynamicFilter::ShelfFixedPoles();
  minus6.gain_lin = 0.5;
  NTFX_ADD_TEST(set, minus6, "impulse");
  auto minus12     = NtFx::DynamicFilter::ShelfFixedPoles();
  minus12.gain_lin = 0.25;
  NTFX_ADD_TEST(set, minus12, "impulse");
  auto minus6lowQ2     = NtFx::DynamicFilter::ShelfFixedPoles();
  minus6lowQ2.gain_lin = 0.5;
  minus6lowQ2.q1       = 2;
  NTFX_ADD_TEST(set, minus6lowQ2, "impulse");
  auto minus12lowQ2     = NtFx::DynamicFilter::ShelfFixedPoles();
  minus12lowQ2.gain_lin = 0.25;
  minus12lowQ2.q1       = 2;
  NTFX_ADD_TEST(set, minus12lowQ2, "impulse");
  auto minus6bothQ05     = NtFx::DynamicFilter::ShelfFixedPoles();
  minus6bothQ05.gain_lin = 0.5;
  minus6bothQ05.q1       = 0.5;
  minus6bothQ05.q2       = 0.5;
  NTFX_ADD_TEST(set, minus6bothQ05, "impulse");
  auto minus12bothQ05     = NtFx::DynamicFilter::ShelfFixedPoles();
  minus12bothQ05.gain_lin = 0.25;
  minus12bothQ05.q1       = 0.5;
  minus12bothQ05.q2       = 0.5;
  NTFX_ADD_TEST(set, minus12bothQ05, "impulse");
  auto minusInf     = NtFx::DynamicFilter::ShelfFixedPoles();
  minusInf.gain_lin = 0;
  NTFX_ADD_TEST(set, minusInf, "impulse");
  auto fixedZeros     = NtFx::DynamicFilter::ShelfFixedZeros();
  fixedZeros.gain_lin = 0.25;
  NTFX_ADD_TEST(set, fixedZeros, "impulse");
  return set.runAllTests();
}