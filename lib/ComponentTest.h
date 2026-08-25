#pragma once

/**
 * @file ComponentTest.h
 * @author Niels Thøgersen (niels.thoegersen@gmail.com)
 * @brief Test of audio components.
 * @details  A unit test framework for NtPlugin library components and plugins.
 * Works in conjunction with 'testWrapper/test.py', which builds and runs the
 * tests. Tests are specified in 'testWrapper/tests' in the form of cpp files
 * named after the Component to test appended '_test.cpp'. Inputs are stored in
 * 'testWrapper/in' and results are stored in 'testWrapper/out'. 'in' is under
 * git source control since it contains expected test vectors along with
 * stimuli. Input and output files must contain two columns of floating point
 * numbers.
 *
 * @copyright Copyright (c) 2026
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Affero General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option) any
 * later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 * You are free to download, build and use this code for commercial
 * purposes. Just don't resell it or a build of it, modified or otherwise.
 *
 */

#include "lib/Audio.h"
#include "lib/Component.h"
#include "lib/utils.h"
#include <algorithm>
#include <cassert>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdlib.h>
#include <string>
#include <vector>

consteval auto testFileBaseName(std::string_view fileName) {
  auto _begin =
      fileName.find("tests/") + std::char_traits<char>::length("tests/");
  auto n             = fileName.length();
  auto baseName      = fileName.substr(_begin, n);
  auto _end          = baseName.find(".cpp");
  auto baseNameNoExt = baseName.substr(0, _end);
  _end               = baseNameNoExt.find("_test");
  baseNameNoExt      = baseNameNoExt.substr(0, _end);
  return baseNameNoExt;
}

#define NTFX_ADD_TEST_IMPL(set, object, stimuli)                               \
  set.addTest(&(object), NTFX_EXPAND_AND_QUOTE(object), { stimuli })

/**
 * @brief Adds a new test to ComponentTest.
 *
 * @param object Object derived from Component class that need to be tested.
 * Note that the object is NOT copied and must stay in scope until the test has
 * run.
 */
#define NTFX_ADD_TEST(set, object, stimuli)                                    \
  NTFX_ADD_TEST_IMPL(set, object, stimuli)

#define NTFX_ADD_TEST_PTR_IMPL(set, object, stimuli)                           \
  set.addTest(object, NTFX_EXPAND_AND_QUOTE(object), { stimuli })

/**
 * @brief Adds a new test to ComponentTest.
 *
 * @param object Object derived from Component class that need to be tested.
 * Note that the object is NOT copied and must stay in scope until the test has
 * run.
 */
#define NTFX_ADD_TEST_PTR(set, object, stimuli)                                \
  NTFX_ADD_TEST_PTR_IMPL(set, object, stimuli)

namespace NtFx {
static const std::vector<std::string> STIMULI_NAMES { "impulse",
  "syncSweep",
  "linearSweep",
  "dynamic_alternating",
  "dynamic_matched" };

constexpr char SEPARATOR = '.';

struct ComponentTest;

/**
 * @brief Set of a number of tests. Represents a full test file and this a test
 * of a Component in the library or an NtPlugin.
 *
 * @tparam signal_t Audio datatype
 */
struct ComponentTestSet {
  int nTests { 0 };      ///< Total tests run.
  int nSuccessful { 0 }; ///< Number of successful tests.
  std::vector<std::unique_ptr<ComponentTest>>
      tests;        ///< Vector of tests to run.
  std::string name; ///< Name of set of tests.

  /**
   * @brief Construct a new Component Test Set object
   *
   * @param name Name of component under test.
   */
  ComponentTestSet(std::string _name) : name(_name) { }

  /**
   * @brief Get and print the results of all tests.
   *
   * @return true If all tests passed.
   * @return false If any test failed. Missing expected vector is a failure.
   */
  bool getResults() {
    auto success = nSuccessful == this->nTests && this->nTests > 0;
    std::cout << std::fixed << std::setprecision(2);
    if (success) {
      std::cout << "\033[32m";
    } else {
      std::cout << "\033[31m";
    }
    if (!this->nTests) {
      std::cout << "No tests.\033[0m\n";
      return false;
    }
    std::cout << this->name << ": Ran a total of " << this->nTests
              << " test on " << this->tests.size() << " objects. "
              << this->nSuccessful << " succeeded. ("
              << 100.0 * double(this->nSuccessful) / double(this->nTests)
              << "%)."
              << "\033[0m" << "\n";
    auto f = std::ofstream("testWrapper/out/results.txt", std::ios_base::app);
    f << this->name << "," << this->nTests << "," << this->tests.size() << ","
      << this->nSuccessful << "\n";
    return success;
  }

  /**
   * @brief Adds a new test to set.
   *
   * @param componentObj Reference to object under test.
   * @param objName Name of object under test.
   * @param stimuli Vector of stimuli to test against.
   * @return true if tests pass.
   * @return false if tests fail.
   */
  bool addTest(ComponentBase<Stereo<signal_t>>* componentObj,
      std::string objName,
      std::vector<std::string> stimuli);

  // bool addTest(ComponentBase<Stereo<signal_t>>** componentObj,
  //     std::string objName,
  //     std::vector<std::string> stimuli) {
  //   return this->addTest(*componentObj, objName, stimuli);
  // }

  /**
   * @brief Runs all tests added to test set.
   *
   * @return int 0 on success, 1 on failure.
   */
  int runAllTests();
};
/**
 * @brief Processes a vector of test data with audio component and compares
 * to expected results. Total number of tests and number of succesful test
 * are stored as statics in order to make statistics.
 *
 */
struct ComponentTest {
  ComponentTestSet* p_owner;              ///< Set this test belongs to.
  std::string objName;                    ///< Object to be tested.
  ComponentBase<Stereo<signal_t>>* p_cut; ///< Component under test.
  std::vector<std::string> activeStimuli = STIMULI_NAMES; ///< Tests to run.

  /**
   * @brief Construct a new Component Test object
   *
   * @param owner Test set this test belongs to.
   * @param objName A name for the current object under test.
   * @param cut Reference to the object under test.
   * @param stimuli Vector of strings naming stimuli to test object against.
   * Accepted values are defined in STIMULI_NAMES. Defaults to an empty vector,
   * selecting all knows stimuli as defined in STIMULI_NAMES.
   */
  ComponentTest(ComponentTestSet* _p_owner,
      std::string _objName,
      ComponentBase<Stereo<signal_t>>* _p_cut,
      std::vector<std::string> stimuli = { })
      : p_owner(_p_owner), objName(_objName), p_cut(_p_cut) {
    if (stimuli.empty()) { return; }
    for (auto& stimulus : stimuli) {
      if (std::find(STIMULI_NAMES.begin(), STIMULI_NAMES.end(), stimulus)
          == STIMULI_NAMES.end()) {
        std::cout << "\033[31mStimuli name '" << stimulus
                  << "' not accepted.\033[0m\n";
        assert(false);
      }
    }
    this->activeStimuli = stimuli;
  }

  /**
   * @brief Processes the object under test with 'stimulus', writes out results
   * to a file and compares to expected results if found.
   *
   * @param stimulus Name of stimulus to process with object under test.
   * @return true
   * @return false
   */
  bool run(std::string stimulus) {
    if (!this->_stimulusIsActive(stimulus)) { return true; }
    this->p_owner->nTests++;
    auto x = this->_readInput(stimulus);
    std::vector<Stereo<signal_t>> y;
    this->p_cut->reset(NTFX_FS);
    for (auto _x : x) { y.push_back(p_cut->process(_x)); }
    const auto yPath = "testWrapper/out/" + this->p_owner->name + SEPARATOR
        + this->objName + SEPARATOR + stimulus + SEPARATOR + "result.txt";
    std::ofstream yFile(yPath);
    yFile << std::fixed << std::setprecision(16);
    for (auto _y : y) { yFile << _y.l << " " << _y.r << "\n"; }
    auto success = this->_compareExpected(stimulus, y);
    if (success) {
      std::cout << "\033[32m";
      this->p_owner->nSuccessful++;
    } else {
      std::cout << "\033[31m";
    }
    std::cout << this->p_owner->name << ": Test '" << stimulus
              << "' for object '" << this->objName << "' in file '"
              << this->p_owner->name << "'";
    std::cout << (success ? " passed." : " failed.") << "\033[0m" << "\n";
    return success;
  }

  std::vector<Stereo<signal_t>> _readInput(std::string stimulus) {
    auto xPath = "testWrapper/in/" + stimulus + ".txt";
    if (!std::filesystem::exists(xPath)) {
      std::cout << "Input file '" << xPath << "'not found. Aborting test."
                << "\n";
      return { };
    }
    std::fstream xFile(xPath);
    std::vector<Stereo<signal_t>> x;
    signal_t l { 0 }, r { 0 };
    while (xFile >> l >> r) { x.push_back({ l, r }); }
    return x;
  }

  bool _stimulusIsActive(std::string stimulus) {
    if (std::find(
            this->activeStimuli.begin(), this->activeStimuli.end(), stimulus)
        == this->activeStimuli.end()) {
      return false;
    }
    return true;
  }

  std::vector<Stereo<signal_t>> _readExpected(std::string stimulus) {
    auto expPath = "testWrapper/in/" + this->p_owner->name + SEPARATOR
        + this->objName + SEPARATOR + stimulus + SEPARATOR + "expected.txt";
    bool expFileExists = std::filesystem::exists(expPath);
    if (!expFileExists) {
      std::cout << this->p_owner->name
                << ": File with expected results not found at path '" + expPath
              + "'. Skipping comparison."
                << "\n";
      return { };
    }
    std::vector<Stereo<signal_t>> e;
    std::fstream eFile(expPath);
    std::string line;
    while (std::getline(eFile, line)) {
      std::istringstream iss(line);
      signal_t l { 0 }, r { 0 };
      if (iss >> l >> r) {
        e.push_back({ l, r });
      } else {
        e.push_back({ 0, 0 });
      }
    }
    return e;
  }

  bool _compareExpected(
      std::string stimulus, const std::vector<Stereo<signal_t>>& y) {
    auto e = this->_readExpected(stimulus);
    if (e.size() != y.size()) {
      std::cout
          << this->p_owner->name
          << ": Expected result has different length that result. Aborting."
          << " e: " << e.size() << ", y: " << y.size() << "\n";
      return false;
    }
    auto acceptedDiff = signal_t(0.0001);
    for (size_t i = 0; i < y.size(); i++) {
      auto diff = gcem::abs(y[i] - e[i]);
      if (diff > acceptedDiff) {
        std::cout << this->p_owner->name << "." << this->objName << "."
                  << stimulus << ":" << " output: {" << y[i].l << ", " << y[i].r
                  << "}, expected: {" << e[i].l << ", " << e[i].r
                  << "}, at index: " << i << ". Diff: {" << diff.l << ", "
                  << diff.r << "}." << "\n";
        return false;
      }
    }
    return true;
  }
};

bool ComponentTestSet::addTest(ComponentBase<Stereo<signal_t>>* componentObj,
    std::string objName,
    std::vector<std::string> stimuli) {
  this->tests.push_back(std::make_unique<NtFx::ComponentTest>(
      this, objName, componentObj, stimuli));
  return true;
}

int ComponentTestSet::runAllTests() {
  for (auto& stimulus : STIMULI_NAMES) {
    for (size_t i = 0; i < tests.size(); i++) { tests[i]->run(stimulus); }
  }
  return !this->getResults();
}
}
