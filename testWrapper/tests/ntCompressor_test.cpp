#include "lib/ComponentTest.h"
#include "plugins/ntCompressor.h"

NTFX_TEST_BEGIN

NTFX_TEST() {
  auto compressor_dB                     = ntCompressor<float>();
  compressor_dB.scSettings.thresh_db     = -20;
  compressor_dB.linEnable                = false;
  auto compressor_lin                    = ntCompressor<float>();
  compressor_lin.scSettings.thresh_db    = -20;
  compressor_lin.linEnable               = true;
  auto compressor_lin_fb                 = ntCompressor<float>();
  compressor_lin_fb.scSettings.thresh_db = -20;
  compressor_lin_fb.linEnable            = true;
  compressor_lin_fb.scMode               = 1;
  auto compressor_dB_fb                  = ntCompressor<float>();
  compressor_dB_fb.scSettings.thresh_db  = -20;
  compressor_dB_fb.linEnable             = false;
  compressor_dB_fb.scMode                = 1;

  NTFX_ADD_TEST(compressor_dB, "dynamic");
  NTFX_ADD_TEST(compressor_lin, "dynamic");
  NTFX_ADD_TEST(compressor_lin_fb, "dynamic");
  NTFX_ADD_TEST(compressor_dB_fb, "dynamic");
  NTFX_ADD_TEST(compressor_dB, "linearSweep");
  NTFX_ADD_TEST(compressor_lin, "linearSweep");
  NTFX_ADD_TEST(compressor_lin_fb, "linearSweep");
  NTFX_ADD_TEST(compressor_dB_fb, "linearSweep");

  return NTFX_RUN_TESTS();
  ;
}