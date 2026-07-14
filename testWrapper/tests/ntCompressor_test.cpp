#include "lib/Comp.h"
#include "lib/ComponentTest.h"
#include "plugins/ntCompressor.h"

NTFX_TEST_BEGIN

NtFx::Comp::ScSettings settings = {
  .thresh_db = signal_t(0),
  .ratio     = signal_t(2),
  .knee_db   = signal_t(12),
  .tAtt_ms   = signal_t(1),
  .tRel_ms   = signal_t(100),
  .tRms_ms   = signal_t(80),
  .tPeak_ms  = signal_t(20.0),
};
NTFX_TEST() {
  auto compressor_dB                     = ntCompressor();
  compressor_dB.scSettings.ratio         = 3;
  compressor_dB.scSettings.knee_db       = 0;
  compressor_dB.scSettings.thresh_db     = -6;
  compressor_dB.scSettings.tRel_ms       = 20;
  compressor_dB.scSettings.tAtt_ms       = 1;
  compressor_dB.scSettings.linkEnable    = false;
  compressor_dB.linEnable                = false;
  compressor_dB.clip                     = false;
  auto compressor_lin                    = ntCompressor();
  compressor_lin.scSettings.ratio        = 3;
  compressor_lin.scSettings.tRel_ms      = 100;
  compressor_lin.scSettings.tAtt_ms      = 1;
  compressor_lin.scSettings.thresh_db    = -12;
  compressor_lin.linEnable               = true;
  compressor_lin.clip                    = false;
  auto compressor_lin_fb                 = ntCompressor();
  compressor_lin_fb.scSettings.thresh_db = -24;
  compressor_lin_fb.linEnable            = true;
  compressor_lin_fb.scMode               = 1;
  compressor_lin_fb.clip                 = false;
  auto compressor_dB_fb                  = ntCompressor();
  compressor_dB_fb.scSettings.thresh_db  = -30;
  compressor_dB_fb.linEnable             = false;
  compressor_dB_fb.scMode                = 1;
  compressor_dB_fb.clip                  = false;
  // auto compressor_dB_softClip            = compressor_dB;
  // auto compressor_lin_softClip           = compressor_lin;
  // auto compressor_lin_fb_softClip        = compressor_lin_fb;
  // auto compressor_dB_fb_softClip         = compressor_dB_fb;
  // compressor_dB_softClip.clip            = true;
  // compressor_lin_softClip.clip           = true;
  // compressor_lin_fb_softClip.clip        = true;
  // compressor_dB_fb_softClip.clip         = true;

  NTFX_ADD_TEST(compressor_dB, "dynamic_alternating");
  NTFX_ADD_TEST(compressor_lin, "dynamic_alternating");
  NTFX_ADD_TEST(compressor_lin_fb, "dynamic_alternating");
  NTFX_ADD_TEST(compressor_dB_fb, "dynamic_alternating");
  NTFX_ADD_TEST(compressor_dB, "linearSweep");
  NTFX_ADD_TEST(compressor_lin, "linearSweep");
  NTFX_ADD_TEST(compressor_lin_fb, "linearSweep");
  NTFX_ADD_TEST(compressor_dB_fb, "linearSweep");
  // NTFX_ADD_TEST(compressor_dB_softClip, "dynamic_alternating");
  // NTFX_ADD_TEST(compressor_lin_softClip, "dynamic_alternating");
  // NTFX_ADD_TEST(compressor_lin_fb_softClip, "dynamic_alternating");
  // NTFX_ADD_TEST(compressor_dB_fb_softClip, "dynamic_alternating");
  // NTFX_ADD_TEST(compressor_dB_softClip, "linearSweep");
  // NTFX_ADD_TEST(compressor_lin_softClip, "linearSweep");
  // NTFX_ADD_TEST(compressor_lin_fb_softClip, "linearSweep");
  // NTFX_ADD_TEST(compressor_dB_fb_softClip, "linearSweep");

  return NTFX_RUN_TESTS();
}