
#include "lib/Audio.h"
#include "lib/Component.h"
#include "lib/ComponentTest.h"
#include "lib/Tilt.h"
#include "plugins/ntTilt.h"
#include <memory>
#include <string>

struct EmphDeemph final : public NtFx::ComponentBase<Audio> {
  NtFx::Tilt<> emph;
  NtFx::Tilt<> deemph;
  signal_t corr_lin { 1 };
  Audio process(Audio x) noexcept override {
    return this->deemph.process(this->emph.process(x)) * this->corr_lin;
  }

  void update() noexcept override {
    // this->corr_lin       = this->emph.calcCorr();
    this->deemph.tilt_db = -this->emph.tilt_db;
    this->emph.update();
    this->deemph.update();
  }
  void reset(signal_t fs) noexcept override {
    this->_fs = fs;
    this->emph.reset(fs);
    this->deemph.reset(fs);
    this->update();
  }
};

int main() {
  auto set = NtFx::ComponentTestSet(std::string(testFileBaseName(__FILE__)));
  auto bypass_        = std::make_unique<ntTilt>();
  auto& bypass        = *bypass_;
  bypass.bypassEnable = true;
  NTFX_ADD_TEST(set, bypass, "impulse");
  auto defaults_ = std::make_unique<ntTilt>();
  auto& defaults = *defaults_;
  NTFX_ADD_TEST(set, defaults, "impulse");
  auto plus5_          = std::make_unique<ntTilt>();
  auto& plus5          = *plus5_;
  plus5.filter.tilt_db = 5;
  NTFX_ADD_TEST(set, plus5, "impulse");
  auto minus5_          = std::make_unique<ntTilt>();
  auto& minus5          = *minus5_;
  minus5.filter.tilt_db = -5;
  NTFX_ADD_TEST(set, minus5, "impulse");
  auto plus10_          = std::make_unique<ntTilt>();
  auto& plus10          = *plus10_;
  plus10.filter.tilt_db = 10;
  NTFX_ADD_TEST(set, plus10, "impulse");
  auto minus10_          = std::make_unique<ntTilt>();
  auto& minus10          = *minus10_;
  minus10.filter.tilt_db = -10;
  NTFX_ADD_TEST(set, minus10, "impulse");
  auto plus20_          = std::make_unique<ntTilt>();
  auto& plus20          = *plus20_;
  plus20.filter.tilt_db = 20;
  NTFX_ADD_TEST(set, plus20, "impulse");
  auto minus20_          = std::make_unique<ntTilt>();
  auto& minus20          = *minus20_;
  minus20.filter.tilt_db = -20;
  NTFX_ADD_TEST(set, minus20, "impulse");
  auto emphDeemph20_        = std::make_unique<EmphDeemph>();
  auto& emphDeemph20        = *emphDeemph20_;
  emphDeemph20.emph.tilt_db = 20;
  NTFX_ADD_TEST(set, emphDeemph20, "impulse");
  auto emphDeemph15_        = std::make_unique<EmphDeemph>();
  auto& emphDeemph15        = *emphDeemph15_;
  emphDeemph15.emph.tilt_db = 15;
  NTFX_ADD_TEST(set, emphDeemph15, "impulse");
  auto emphDeemph10_        = std::make_unique<EmphDeemph>();
  auto& emphDeemph10        = *emphDeemph10_;
  emphDeemph10.emph.tilt_db = 10;
  NTFX_ADD_TEST(set, emphDeemph10, "impulse");
  auto emphDeemph5_        = std::make_unique<EmphDeemph>();
  auto& emphDeemph5        = *emphDeemph5_;
  emphDeemph5.emph.tilt_db = 5;
  NTFX_ADD_TEST(set, emphDeemph5, "impulse");

  auto emphDeemphMinus20_        = std::make_unique<EmphDeemph>();
  auto& emphDeemphMinus20        = *emphDeemphMinus20_;
  emphDeemphMinus20.emph.tilt_db = -20;
  NTFX_ADD_TEST(set, emphDeemphMinus20, "impulse");
  auto emphDeemphMinus15_        = std::make_unique<EmphDeemph>();
  auto& emphDeemphMinus15        = *emphDeemphMinus15_;
  emphDeemphMinus15.emph.tilt_db = -15;
  NTFX_ADD_TEST(set, emphDeemphMinus15, "impulse");
  auto emphDeemphMinus10_        = std::make_unique<EmphDeemph>();
  auto& emphDeemphMinus10        = *emphDeemphMinus10_;
  emphDeemphMinus10.emph.tilt_db = -10;
  NTFX_ADD_TEST(set, emphDeemphMinus10, "impulse");
  auto emphDeemphMinus5_        = std::make_unique<EmphDeemph>();
  auto& emphDeemphMinus5        = *emphDeemphMinus5_;
  emphDeemphMinus5.emph.tilt_db = -5;
  NTFX_ADD_TEST(set, emphDeemphMinus5, "impulse");

  return set.runAllTests();
}