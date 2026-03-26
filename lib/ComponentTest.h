/*
 * Copyright (C) 2026 Niels Thøgersen, NTlyd
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
 **/

/**
 * @brief Test of audio components.
 *
 * Input and output files must contain two columns of floating point numbers.
 *
 */

#pragma once

#include "lib/Component.h"
#include "lib/Stereo.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdlib.h>
#include <string>
#include <vector>

#define QUOTE(str) #str
#define EXPAND_AND_QUOTE(str) QUOTE(str)
// TODO: Copy the component somewhere.
// TODO: This suck. It's a list of strings in the class, but it's a string in
// the macro.
#define ADD_TEST(component, testsToPerform)                                    \
  NtFx::ComponentTest<float>::addTest(                                         \
      component, EXPAND_AND_QUOTE(component), { testsToPerform })

#define NTFX_TEST_BEGIN                                                        \
  namespace NtFx {                                                             \
  template <>                                                                  \
  int NtFx::ComponentTest<float>::nTests = 0;                                  \
  template <>                                                                  \
  int NtFx::ComponentTest<float>::nSuccessful = 0;                             \
  template <>                                                                  \
  int NtFx::ComponentTest<float>::nComponents = 0;                             \
  template <>                                                                  \
  std::vector<std::unique_ptr<NtFx::ComponentTest<float>>>                     \
      NtFx::ComponentTest<float>::allTests { };                                \
  template <>                                                                  \
  std::vector<std::string> NtFx::ComponentTest<float>::allTestNames { };       \
  }

namespace NtFx {
static const std::vector<std::string> acceptedSubtests {
  "impulse", "syncSweep", "linearSweep", "dynamic"
};

constexpr char SEPARATOR = '.';

/**
 * @brief Processes a vector of test data with audio component and compares
 * to expected results. Total number of tests and number of succesful test
 * are stored as statics in order to make statistics.
 *
 */
template <typename signal_t>
struct ComponentTest {
  static int nComponents; ///< Number of components tested.
  static int nTests;      ///< Total tests run.
  static int nSuccessful; ///< Number of successful tests.
  static std::vector<std::unique_ptr<ComponentTest<signal_t>>> allTests;
  static std::vector<std::string> allTestNames;

  Component<Stereo<signal_t>>& cut; ///< Component under test.

  std::vector<std::string> activeTests =
      acceptedSubtests; ///< List of names of tests to run.
  /**
   * @brief Construct a new Component Test object.
   *
   * @param cut Component to be tested.
   */
  ComponentTest(
      Component<Stereo<signal_t>>& cut, std::vector<std::string> subtests = { })
      : cut(cut) {
    nComponents++;
    if (subtests.empty()) { return; }
    for (auto& subtest : subtests) {
      if (std::find(acceptedSubtests.begin(), acceptedSubtests.end(), subtest)
          == acceptedSubtests.end()) {
        std::cout << "Subtest '" << subtest << "' not accepted.";
        this->activeTests.clear();
        return;
      }
    }
    this->activeTests = subtests;
  }

  static bool addTest(Component<Stereo<signal_t>>& component,
      std::string componentName,
      std::vector<std::string> testsToPerform = { }) {
    if (std::find(allTestNames.begin(), allTestNames.end(), componentName)
        != allTestNames.end()) {
      std::cout << "Test '" << componentName << "' already registered."
                << std::endl;
      return false;
    }
    NtFx::ComponentTest<float>::allTestNames.push_back(componentName);
    NtFx::ComponentTest<float>::allTests.push_back(
        std::make_unique<NtFx::ComponentTest<float>>(
            component, testsToPerform));
    return true;
  }

  /**
   * @brief Runs test for component.
   *
   * @param testName Name of test.
   * @param xFilePath Input file.
   * @param yFilePath Output file. If not given, [testName]_result.txt will be
   * used.
   * @param expFilePath Input file containing expected output. If not given,
   * [testName]_expected.txt will be used. If file is missing, the test will run
   * but always failed.
   * @return true If test succeeds.
   * @return false If test fails.
   */
  bool run(std::string component, std::string subtest, float fs = 48.0e3f) {
    if (std::find(this->activeTests.begin(), this->activeTests.end(), subtest)
        == this->activeTests.end()) {
      return true;
    }
    this->nTests++;
    auto testName = component + SEPARATOR + subtest;
    auto xPath    = "testWrapper/in/" + subtest + ".txt";

    if (!std::filesystem::exists(xPath)) {
      std::cout << "Input file '" << xPath << "'not found. Aborting test."
                << std::endl;
      return false;
    }
    auto yPath   = "testWrapper/out/" + testName + SEPARATOR + "result.txt";
    auto expPath = "testWrapper/in/" + testName + SEPARATOR + "expected.txt";
    bool expFileExists = std::filesystem::exists(expPath);
    bool success       = true;
    if (!expFileExists) {
      std::cout << "File with expected results not found at path '" + expPath
              + "'. Skipping comparison."
                << std::endl;
      // TODO: What to do when the test failed to run. It's neither succeeded
      // nor failed.
      success = false;
    }
    this->cut.reset(fs);
    std::vector<Stereo<signal_t>> x;
    std::fstream xFile(xPath);
    signal_t l, r;
    while (xFile >> l >> r) { x.push_back({ l, r }); }
    std::vector<Stereo<signal_t>> y;
    for (auto _x : x) { y.push_back(cut.process(_x)); }
    signal_t acceptedDiff = 0.00001;
    if (expFileExists) {
      std::vector<Stereo<signal_t>> e;
      std::fstream eFile(expPath);
      std::string line;
      while (std::getline(eFile, line)) {
        std::istringstream iss(line);
        float l, r;
        if (iss >> l >> r) {
          e.push_back({ l, r });
        } else {
          e.push_back({ 0, 0 });
        }
      }
      if (e.size() != y.size()) {
        std::cout
            << "Expected result has different length that result. Aborting."
            << " e: " << e.size() << ", y: " << y.size() << std::endl;
        return false;
      }
      for (size_t i = 0; i < x.size(); i++) {
        auto diff = gcem::abs(y[i] - e[i]);
        if (diff > acceptedDiff) { success = false; }
      }
    }
    std::ofstream yFile(yPath);
    for (auto _y : y) { yFile << _y.l << " " << _y.r << std::endl; }
    if (success) {
      std::cout << "\033[32m";
    } else {
      std::cout << "\033[31m";
    }
    std::cout << "Test '" << subtest << "' for component '" << component << "'";
    std::cout << (success ? " passed." : " failed.") << "\033[0m" << std::endl;
    if (success) { nSuccessful++; }
    return success;
  }

  static int runAllTests() {
    auto n = allTests.size();
    if (n != allTestNames.size()) {
      std::cout << "Test name look up mismatch." << std::endl;
      return false;
    }
    for (auto& subtest : acceptedSubtests) {
      for (size_t i = 0; i < n; i++) {
        allTests[i]->run(allTestNames[i], subtest);
      }
    }
    return !NtFx::ComponentTest<float>::getResults();
  }

  /**
   * @brief Get and print the results of all tests.
   *
   * @return true If all tests passed.
   * @return false If any test failed. Missing expected vector is a failure.
   */
  static bool getResults() {
    if (nSuccessful == nTests) {
      std::cout << "\033[32m";
    } else {
      std::cout << "\033[31m";
    }
    std::cout << "Ran a total of " << nTests << " test on " << nComponents
              << " components. " << nSuccessful << " succeeded. ("
              << 100.0 * double(nSuccessful) / double(nTests) << "%)."
              << "\033[0m" << std::endl;
    return nSuccessful == nTests;
  }
};
}
