#include "lib/ComponentTest.h"
#include "lib/DynamicFilter.h"

NTFX_TEST_BEGIN

NTFX_TEST() {
  auto flat     = NtFx::DynamicFilter::Shelf<double>();
  flat.gain_lin = 1;
  NTFX_ADD_TEST(flat, "impulse");
  auto minus6     = NtFx::DynamicFilter::Shelf<double>();
  minus6.gain_lin = 0.5;
  NTFX_ADD_TEST(minus6, "impulse");
  auto minus12     = NtFx::DynamicFilter::Shelf<double>();
  minus12.gain_lin = 0.25;
  NTFX_ADD_TEST(minus12, "impulse");
  auto minus6lowQ2     = NtFx::DynamicFilter::Shelf<double>();
  minus6lowQ2.gain_lin = 0.5;
  minus6lowQ2.q1       = 2;
  NTFX_ADD_TEST(minus6lowQ2, "impulse");
  auto minus12lowQ2     = NtFx::DynamicFilter::Shelf<double>();
  minus12lowQ2.gain_lin = 0.25;
  minus12lowQ2.q1       = 2;
  NTFX_ADD_TEST(minus12lowQ2, "impulse");
  auto minus6bothQ05     = NtFx::DynamicFilter::Shelf<double>();
  minus6bothQ05.gain_lin = 0.5;
  minus6bothQ05.q1       = 0.5;
  minus6bothQ05.q2       = 0.5;
  NTFX_ADD_TEST(minus6bothQ05, "impulse");
  auto minus12bothQ05     = NtFx::DynamicFilter::Shelf<double>();
  minus12bothQ05.gain_lin = 0.25;
  minus12bothQ05.q1       = 0.5;
  minus12bothQ05.q2       = 0.5;
  NTFX_ADD_TEST(minus12bothQ05, "impulse");
  auto minusInf     = NtFx::DynamicFilter::Shelf<double>();
  minusInf.gain_lin = 0;
  NTFX_ADD_TEST(minusInf, "impulse");
  return NTFX_RUN_TESTS();
}