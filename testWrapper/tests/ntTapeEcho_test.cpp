#include "lib/ComponentTest.h"
#include "plugins/ntTapeEcho.h"
#include <memory>

int main() {
  auto set = NtFx::ComponentTestSet(std::string(testFileBaseName(__FILE__)));

  // Tape echo with constructor defaults
  auto tapeEchoDefaultsStorage = std::make_unique<ntTapeEcho>();
  auto* tapeEchoDefaults       = tapeEchoDefaultsStorage.get();
  NTFX_ADD_TEST_PTR(set, tapeEchoDefaults, "impulse");

  // Tape echo bypass
  auto tapeEchoBypassStorage   = std::make_unique<ntTapeEcho>();
  auto* tapeEchoBypass         = tapeEchoBypassStorage.get();
  tapeEchoBypass->bypassEnable = true;
  NTFX_ADD_TEST_PTR(set, tapeEchoBypass, "impulse");

  // Tape echo with default settings
  auto tapeEchoDefaultStorage = std::make_unique<ntTapeEcho>();
  auto* tapeEchoDefault       = tapeEchoDefaultStorage.get();
  tapeEchoDefault->subDevL    = SubDev::eighth;
  tapeEchoDefault->subDevR    = SubDev::eighth;
  tapeEchoDefault->fb_lin     = 0.5f;
  NTFX_ADD_TEST_PTR(set, tapeEchoDefault, "linearSweep");

  // Tape echo with longer delay
  auto tapeEchoLongStorage = std::make_unique<ntTapeEcho>();
  auto* tapeEchoLong       = tapeEchoLongStorage.get();
  tapeEchoLong->subDevL    = SubDev::fourth;
  tapeEchoLong->subDevR    = SubDev::fourth;
  tapeEchoLong->fb_lin     = 0.4f;
  NTFX_ADD_TEST_PTR(set, tapeEchoLong, "linearSweep");

  // Tape echo with high feedback
  auto tapeEchoHighFbStorage = std::make_unique<ntTapeEcho>();
  auto* tapeEchoHighFb       = tapeEchoHighFbStorage.get();
  tapeEchoHighFb->subDevL    = SubDev::eighth;
  tapeEchoHighFb->subDevR    = SubDev::eighth;
  tapeEchoHighFb->fb_lin     = 0.8f;
  NTFX_ADD_TEST_PTR(set, tapeEchoHighFb, "linearSweep");

  // Tape echo with dotted eighth note
  auto tapeEchoDottedStorage = std::make_unique<ntTapeEcho>();
  auto* tapeEchoDotted       = tapeEchoDottedStorage.get();
  tapeEchoDotted->subDevL    = SubDev::eighth_dot;
  tapeEchoDotted->subDevR    = SubDev::eighth_dot;
  tapeEchoDotted->fb_lin     = 0.5f;
  NTFX_ADD_TEST_PTR(set, tapeEchoDotted, "linearSweep");

  return set.runAllTests();
}
