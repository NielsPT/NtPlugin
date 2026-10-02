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

  // Tape echo with longer delay
  auto tapeEchoLongStorage = std::make_unique<ntTapeEcho>();
  auto* tapeEchoLong       = tapeEchoLongStorage.get();
  tapeEchoLong->syncEnable = true;
  tapeEchoLong->tempo      = 120;
  tapeEchoLong->subDevL    = ntTapeEcho::SubDev::fourth;
  tapeEchoLong->subDevR    = ntTapeEcho::SubDev::fourth;
  tapeEchoLong->fb_lin     = 0.4f;
  NTFX_ADD_TEST_PTR(set, tapeEchoLong, "linearSweep");

  // Tape echo with high feedback
  auto tapeEchoHighFbStorage = std::make_unique<ntTapeEcho>();
  auto* tapeEchoHighFb       = tapeEchoHighFbStorage.get();
  tapeEchoHighFb->tempo      = 120;
  tapeEchoHighFb->syncEnable = true;
  tapeEchoHighFb->subDevL    = ntTapeEcho::SubDev::eighth;
  tapeEchoHighFb->subDevR    = ntTapeEcho::SubDev::eighth;
  tapeEchoHighFb->fb_lin     = 0.8f;
  NTFX_ADD_TEST_PTR(set, tapeEchoHighFb, "linearSweep");

  // Tape echo with dotted eighth note
  auto tapeEchoDottedStorage = std::make_unique<ntTapeEcho>();
  auto* tapeEchoDotted       = tapeEchoDottedStorage.get();
  tapeEchoDotted->tempo      = 120;
  tapeEchoDotted->syncEnable = true;
  tapeEchoDotted->subDevL    = ntTapeEcho::SubDev::eighth_dot;
  tapeEchoDotted->subDevR    = ntTapeEcho::SubDev::eighth_dot;
  tapeEchoDotted->fb_lin     = 0.5f;
  NTFX_ADD_TEST_PTR(set, tapeEchoDotted, "linearSweep");

  return set.runAllTests();
}
