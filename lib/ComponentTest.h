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
// #include <boost/core/demangle.hpp>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdlib.h>
#include <string>
#include <vector>

consteval auto testFileBaseName(std::string_view fileName) {
  auto _begin        = fileName.find("/") + 1;
  auto n             = fileName.length();
  auto baseName      = fileName.substr(_begin, n);
  auto _end          = baseName.find(".cpp");
  auto baseNameNoExt = baseName.substr(0, _end);
  _end               = baseNameNoExt.find("_test");
  baseNameNoExt      = baseNameNoExt.substr(0, _end);
  return baseNameNoExt;
}

#define _NTFX_QUOTE(str) #str
#define _NTFX_EXPAND_AND_QUOTE(str) _NTFX_QUOTE(str)

#define _NTFX_ADD_TEST_IMPL(object, stimuli)                                   \
  NtFx::ComponentTest<float>::addTest(object,                                  \
      _NTFX_EXPAND_AND_QUOTE(object),                                          \
      std::string(testFileBaseName(__FILE__)),                                 \
      { stimuli })

/**
 * @brief Adds a new test to ComponentTest.
 *
 * @param object Object derived from Component class that need to be tested.
 * Note that the object is NOT copied and must stay in scope until the test has
 * run.
 */
#define NTFX_ADD_TEST(object, stimuli) _NTFX_ADD_TEST_IMPL(object, stimuli)

#define _NTFX_TEST_BEGIN_IMPL                                                  \
  namespace NtFx {                                                             \
    template <typename T>                                                      \
    int NtFx::ComponentTest<T>::nTests = 0;                                    \
    template <typename T>                                                      \
    int NtFx::ComponentTest<T>::nSuccessful = 0;                               \
    template <typename T>                                                      \
    int NtFx::ComponentTest<T>::nObjects = 0;                                  \
    template <typename T>                                                      \
    std::vector<std::unique_ptr<NtFx::ComponentTest<T>>>                       \
        NtFx::ComponentTest<T>::tests { };                                     \
    template <typename T>                                                      \
    std::string NtFx::ComponentTest<T>::testFile { std::string(                \
        testFileBaseName(__FILE__)) };                                         \
  }

#ifndef _NTFX_TEST_STARTED
  /**
   * @brief Add this to the beginning of you test suite to instantiate the
   * needed statics of ComponentTest. Sucks, I know, but that's what C++
   * demands.
   *
   */
  #define NTFX_TEST_BEGIN _NTFX_TEST_BEGIN_IMPL
  #define _NTFX_TEST_STARTED
#else
  #define NTFX_TEST_BEGIN
#endif

// TODO: Replace 'main' with something else when multiple files are in one test
// run. But what? __FILE__ won't cut it, it has the full path.
// Alternatively, we could store a CSV with the resuls and read that from Pyton
// once all the tests are done. It's not as efficient, but it'll work.
#define _NTFX_TEST_IMPL() int main()
#define NTFX_TEST() _NTFX_TEST_IMPL()

#define NTFX_RUN_TESTS() NtFx::ComponentTest<float>::runAllTests()

namespace NtFx {
static const std::vector<std::string> STIMULI_NAMES {
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
  // TODO: Instead of using statics, add another class which encapsulates all
  // the ComponentTest objects.
  static int nObjects;    ///< Number of components tested.
  static int nTests;      ///< Total tests run.
  static int nSuccessful; ///< Number of successful tests.
  static std::vector<std::unique_ptr<ComponentTest<signal_t>>> tests;
  static std::string testFile;
  const std::string objName;
  Component<Stereo<signal_t>>& cut; ///< Component under test.
  std::vector<std::string> activeStimuli =
      STIMULI_NAMES; ///< List of names of tests to run.

  // TODO: dox
  ComponentTest(std::string objName,
      Component<Stereo<signal_t>>& cut,
      std::vector<std::string> stimuli = { })
      : objName(objName), cut(cut) {
    nObjects++;
    if (stimuli.empty()) { return; }
    for (auto& stimulus : stimuli) {
      if (std::find(STIMULI_NAMES.begin(), STIMULI_NAMES.end(), stimulus)
          == STIMULI_NAMES.end()) {
        std::cout << "Stimuli name '" << stimulus << "' not accepted.";
        this->activeStimuli.clear();
        return;
      }
    }
    this->activeStimuli = stimuli;
  }

  // TODO: dox
  bool run(std::string stimulus) {
    if (!this->stimulusIsActive(stimulus)) { return true; }
    nTests++;
    auto x = this->readInput(stimulus);
    std::vector<Stereo<signal_t>> y;
    this->cut.reset(NTFX_FS);
    for (auto _x : x) { y.push_back(cut.process(_x)); }
    const auto yPath = "testWrapper/out/" + testFile + SEPARATOR + this->objName
        + SEPARATOR + stimulus + SEPARATOR + "result.txt";
    std::ofstream yFile(yPath);
    auto success = this->compareExpected(stimulus, y);
    // TODO: Comma
    for (auto _y : y) { yFile << _y.l << " " << _y.r << std::endl; }
    if (success) {
      std::cout << "\033[32m";
    } else {
      std::cout << "\033[31m";
    }
    std::cout << "Test '" << stimulus << "' for object '" << this->objName
              << "' in file '" << testFile << "'";
    std::cout << (success ? " passed." : " failed.") << "\033[0m" << std::endl;
    if (success) { nSuccessful++; }
    return success;
  }

  std::vector<Stereo<signal_t>> readInput(std::string stimulus) {
    auto xPath = "testWrapper/in/" + stimulus + ".txt";
    if (!std::filesystem::exists(xPath)) {
      std::cout << "Input file '" << xPath << "'not found. Aborting test."
                << std::endl;
      return { };
    }
    std::fstream xFile(xPath);
    std::vector<Stereo<signal_t>> x;
    signal_t l, r;
    // TODO: comma
    while (xFile >> l >> r) { x.push_back({ l, r }); }
    return x;
  }

  static bool addTest(Component<Stereo<signal_t>>& componentObj,
      std::string objectName,
      std::string fileBaseName,
      std::vector<std::string> stimuliToUse = { }) {
    NtFx::ComponentTest<float>::tests.push_back(
        std::make_unique<NtFx::ComponentTest<float>>(
            objectName, componentObj, stimuliToUse));
    return true;
  }

  // TODO: dox
  static int runAllTests() {
    auto n = tests.size();
    for (auto& stimulus : STIMULI_NAMES) {
      for (size_t i = 0; i < n; i++) { tests[i]->run(stimulus); }
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
    std::cout << "Ran a total of " << nTests << " test on " << nObjects
              << " objects. " << nSuccessful << " succeeded. ("
              << 100.0 * double(nSuccessful) / double(nTests) << "%)."
              << "\033[0m" << std::endl;
    auto f = std::ofstream("testWrapper/out/results.txt", std::ios_base::app);
    f << testFile << "," << nTests << "," << nObjects << "," << nSuccessful
      << std::endl;
    return nSuccessful == nTests;
  }

  bool stimulusIsActive(std::string stimulus) {
    if (std::find(
            this->activeStimuli.begin(), this->activeStimuli.end(), stimulus)
        == this->activeStimuli.end()) {
      return false;
    }
    return true;
  }

  std::vector<Stereo<signal_t>> readExpected(std::string stimulus) {
    auto expPath = "testWrapper/in/" + testFile + SEPARATOR + this->objName
        + SEPARATOR + stimulus + SEPARATOR + "expected.txt";
    bool expFileExists = std::filesystem::exists(expPath);
    if (!expFileExists) {
      std::cout << "File with expected results not found at path '" + expPath
              + "'. Skipping comparison."
                << std::endl;
      return { };
    }
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
    return e;
  }

  bool compareExpected(
      std::string stimulus, const std::vector<Stereo<signal_t>>& y) {
    auto e = this->readExpected(stimulus);
    if (e.size() != y.size()) {
      std::cout << "Expected result has different length that result. Aborting."
                << " e: " << e.size() << ", y: " << y.size() << std::endl;
      return false;
    }
    signal_t acceptedDiff = 0.00001;
    bool success          = true;
    for (size_t i = 0; i < y.size(); i++) {
      auto diff = gcem::abs(y[i] - e[i]);
      if (diff > acceptedDiff) { success = false; }
    }
    return success;
  }
};
}
