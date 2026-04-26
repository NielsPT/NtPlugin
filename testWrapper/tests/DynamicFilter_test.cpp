#include "lib/ComponentTest.h"
#include "lib/DynamicFilter.h"

NTFX_TEST_BEGIN

NTFX_TEST() {
  auto flat     = NtFx::DynamidShelf<double>();
  flat.gain_lin = 1;
  NTFX_ADD_TEST(flat, "impulse");
  auto minus6     = NtFx::DynamidShelf<double>();
  minus6.gain_lin = 0.5;
  NTFX_ADD_TEST(minus6, "impulse");
  auto minus12     = NtFx::DynamidShelf<double>();
  minus12.gain_lin = 0.25;
  NTFX_ADD_TEST(minus12, "impulse");
  return NTFX_RUN_TESTS();
}