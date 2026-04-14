#include "lib/ComponentTest.h"
#include "plugins/ntCompressor.h"

NTFX_TEST_BEGIN

NTFX_TEST() {
  auto compressor_dB                     = ntCompressor<double>();
  compressor_dB.scSettings.thresh_db     = -20;
  compressor_dB.linEnable                = false;
  compressor_dB.clip                     = false;
  auto compressor_lin                    = ntCompressor<double>();
  compressor_lin.scSettings.thresh_db    = -20;
  compressor_lin.linEnable               = true;
  compressor_lin.clip                    = false;
  auto compressor_lin_fb                 = ntCompressor<double>();
  compressor_lin_fb.scSettings.thresh_db = -20;
  compressor_lin_fb.linEnable            = true;
  compressor_lin_fb.scMode               = 1;
  compressor_lin_fb.clip                 = false;
  auto compressor_dB_fb                  = ntCompressor<double>();
  compressor_dB_fb.scSettings.thresh_db  = -20;
  compressor_dB_fb.linEnable             = false;
  compressor_dB_fb.scMode                = 1;
  compressor_dB_fb.clip                  = false;
  auto compressor_dB_softClip            = compressor_dB;
  auto compressor_lin_softClip           = compressor_lin;
  auto compressor_lin_fb_softClip        = compressor_lin_fb;
  auto compressor_dB_fb_softClip         = compressor_dB_fb;
  compressor_dB_softClip.clip            = true;
  compressor_lin_softClip.clip           = true;
  compressor_lin_fb_softClip.clip        = true;
  compressor_dB_fb_softClip.clip         = true;

  NTFX_ADD_TEST(compressor_dB, "dynamic");
  NTFX_ADD_TEST(compressor_lin, "dynamic");
  NTFX_ADD_TEST(compressor_lin_fb, "dynamic");
  NTFX_ADD_TEST(compressor_dB_fb, "dynamic");
  NTFX_ADD_TEST(compressor_dB, "linearSweep");
  NTFX_ADD_TEST(compressor_lin, "linearSweep");
  NTFX_ADD_TEST(compressor_lin_fb, "linearSweep");
  NTFX_ADD_TEST(compressor_dB_fb, "linearSweep");
  NTFX_ADD_TEST(compressor_dB_softClip, "dynamic");
  NTFX_ADD_TEST(compressor_lin_softClip, "dynamic");
  NTFX_ADD_TEST(compressor_lin_fb_softClip, "dynamic");
  NTFX_ADD_TEST(compressor_dB_fb_softClip, "dynamic");
  NTFX_ADD_TEST(compressor_dB_softClip, "linearSweep");
  NTFX_ADD_TEST(compressor_lin_softClip, "linearSweep");
  NTFX_ADD_TEST(compressor_lin_fb_softClip, "linearSweep");
  NTFX_ADD_TEST(compressor_dB_fb_softClip, "linearSweep");

  return NTFX_RUN_TESTS();
  ;
}