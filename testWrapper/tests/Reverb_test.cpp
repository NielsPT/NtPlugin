#include "lib/ComponentTest.h"
#include "lib/Reverb.h"
#include <memory>
#include <string>

static void printCoeffs(NtFx::Reverb::DelayNetwork<>& obj, std::string name) {
  std::cout << name << " a: ";
  for (auto v : obj._a) { std::cout << v << " "; }
  std::cout << std::endl;
  std::cout << name << " t: ";
  for (auto& dl : obj._dls) { std::cout << dl.t_ms << " "; }
  std::cout << std::endl;
  std::cout << name << " n: ";
  for (auto& dl : obj._dls) { std::cout << dl._n << " "; }
  std::cout << std::endl;
}

int main() {
  auto set = NtFx::ComponentTestSet(std::string(testFileBaseName(__FILE__)));

  auto _tail1s = std::make_unique<NtFx::Reverb::DelayNetwork<>>();
  auto& tail1s = *_tail1s;
  tail1s.t60_s = 1;
  NTFX_ADD_TEST(set, tail1s, "impulse");

  auto _tail05s = std::make_unique<NtFx::Reverb::DelayNetwork<>>();
  auto& tail05s = *_tail05s;
  tail05s.t60_s = 0.5;
  NTFX_ADD_TEST(set, tail05s, "impulse");

  auto _tail01s = std::make_unique<NtFx::Reverb::DelayNetwork<>>();
  auto& tail01s = *_tail01s;
  tail01s.t60_s = 0.1;
  NTFX_ADD_TEST(set, tail01s, "impulse");

  auto _goldenRatioAllPass =
      std::make_unique<NtFx::Reverb::GoldenRatioAllPass<Audio>>();
  auto goldenRatioAllPass    = *_goldenRatioAllPass;
  goldenRatioAllPass.t60_ms  = 100;
  goldenRatioAllPass.size_ms = 5;
  NTFX_ADD_TEST(set, goldenRatioAllPass, "impulse");

  auto res = set.runAllTests();
  printCoeffs(tail1s, "1 s");

  constexpr size_t n = 8;
  std::cout << "Hadamard: \n";
  for (size_t i = 0; i < n; i++) {
    for (size_t j = 0; j < n; j++) {
      std::cout << std::to_string(
          NtFx::Reverb::FeedbackMatrix<n>::hadamard[i][j])
                << " ";
    }
    std::cout << "\n";
  }
  std::cout << "Diagonal: \n";
  for (size_t i = 0; i < n; i++) {
    for (size_t j = 0; j < n; j++) {
      std::cout << std::to_string(
          NtFx::Reverb::FeedbackMatrix<n>::diagonal[i][j])
                << " ";
    }
    std::cout << "\n";
  }
  std::cout << "Hadamard - diagonal: \n";
  for (size_t i = 0; i < n; i++) {
    for (size_t j = 0; j < n; j++) {
      std::cout << std::to_string(
          NtFx::Reverb::FeedbackMatrix<n>::hadMinusDiag[i][j])
                << " ";
    }
    std::cout << "\n";
  }
  return res;
}
