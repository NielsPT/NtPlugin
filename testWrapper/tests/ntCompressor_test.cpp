#include "lib/ComponentTest.h"
#include "plugins/ntCompressor.h"

int main() {
  auto set = NtFx::ComponentTestSet(std::string(testFileBaseName(__FILE__)));
  auto compressor_dB                   = new ntCompressor();
  compressor_dB->scSettings.ratio      = 3;
  compressor_dB->scSettings.knee_db    = 0;
  compressor_dB->scSettings.thresh_db  = -6;
  compressor_dB->scSettings.tRel_ms    = 20;
  compressor_dB->scSettings.tAtt_ms    = 1;
  compressor_dB->scSettings.linkEnable = false;
  compressor_dB->linEnable             = false;
  compressor_dB->clip                  = false;
  NTFX_ADD_TEST_PTR(set, compressor_dB, "linearSweep");
  NTFX_ADD_TEST_PTR(set, compressor_dB, "dynamic_alternating");
  auto compressor_lin                  = new ntCompressor();
  compressor_lin->scSettings.ratio     = 3;
  compressor_lin->scSettings.tRel_ms   = 100;
  compressor_lin->scSettings.tAtt_ms   = 1;
  compressor_lin->scSettings.thresh_db = -12;
  compressor_lin->linEnable            = true;
  compressor_lin->clip                 = false;
  NTFX_ADD_TEST_PTR(set, compressor_lin, "linearSweep");
  NTFX_ADD_TEST_PTR(set, compressor_lin, "dynamic_alternating");
  auto compressor_lin_fb                  = new ntCompressor();
  compressor_lin_fb->scSettings.thresh_db = -24;
  compressor_lin_fb->linEnable            = true;
  compressor_lin_fb->scMode               = 1;
  compressor_lin_fb->clip                 = false;
  NTFX_ADD_TEST_PTR(set, compressor_lin_fb, "linearSweep");
  NTFX_ADD_TEST_PTR(set, compressor_lin_fb, "dynamic_alternating");
  auto compressor_dB_fb                  = new ntCompressor();
  compressor_dB_fb->scSettings.thresh_db = -30;
  compressor_dB_fb->linEnable            = false;
  compressor_dB_fb->scMode               = 1;
  compressor_dB_fb->clip                 = false;
  NTFX_ADD_TEST_PTR(set, compressor_dB_fb, "dynamic_alternating");
  NTFX_ADD_TEST_PTR(set, compressor_dB_fb, "linearSweep");
  return set.runAllTests();
}