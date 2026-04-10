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
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdlib.h>
#include <string>
#include <unordered_map>
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
  componentTestSet.addTest(object, _NTFX_EXPAND_AND_QUOTE(object), { stimuli })

/**
 * @brief Adds a new test to ComponentTest.
 *
 * @param object Object derived from Component class that need to be tested.
 * Note that the object is NOT copied and must stay in scope until the test has
 * run.
 */
#define NTFX_ADD_TEST(object, stimuli) _NTFX_ADD_TEST_IMPL(object, stimuli)

#define _NTFX_TEST_BEGIN_IMPL                                                  \
  auto componentTestSet =                                                      \
      NtFx::ComponentTestSet<float>(std::string(testFileBaseName(__FILE__)));

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

#define _NTFX_TEST_IMPL() int main()
#define NTFX_TEST() _NTFX_TEST_IMPL()

#define NTFX_RUN_TESTS() componentTestSet.runAllTests()

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
  const std::string testSetName;
  const std::string objName;
  Component<Stereo<signal_t>>& cut; ///< Component under test.
  std::vector<std::string> activeStimuli =
      STIMULI_NAMES; ///< List of names of tests to run.

  // TODO: dox
  ComponentTest(const std::string testSetName,
      std::string objName,
      Component<Stereo<signal_t>>& cut,
      std::vector<std::string> stimuli = { })
      : testSetName(testSetName), objName(objName), cut(cut) {
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
    auto x = this->readInput(stimulus);
    std::vector<Stereo<signal_t>> y;
    this->cut.reset(NTFX_FS);
    for (auto _x : x) { y.push_back(cut.process(_x)); }
    const auto yPath = "testWrapper/out/" + testSetName + SEPARATOR
        + this->objName + SEPARATOR + stimulus + SEPARATOR + "result.txt";
    std::ofstream yFile(yPath);
    for (auto _y : y) { yFile << _y.l << " " << _y.r << std::endl; }
    auto success = this->compareExpected(stimulus, y);
    if (success) {
      std::cout << "\033[32m";
    } else {
      std::cout << "\033[31m";
    }
    std::cout << "Test '" << stimulus << "' for object '" << this->objName
              << "' in file '" << testSetName << "'";
    std::cout << (success ? " passed." : " failed.") << "\033[0m" << std::endl;
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
    while (xFile >> l >> r) { x.push_back({ l, r }); }
    return x;
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
    auto expPath = "testWrapper/in/" + testSetName + SEPARATOR + this->objName
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

template <typename signal_t>
struct ComponentTestSet {
  int nTests;      ///< Total tests run.
  int nSuccessful; ///< Number of successful tests.
  std::vector<std::unique_ptr<ComponentTest<signal_t>>> tests;
  std::string name;

  ComponentTestSet(std::string name) : name(name) { }

  /**
   * @brief Get and print the results of all tests.
   *
   * @return true If all tests passed.
   * @return false If any test failed. Missing expected vector is a failure.
   */
  bool getResults() {
    if (nSuccessful == nTests) {
      std::cout << "\033[32m";
    } else {
      std::cout << "\033[31m";
    }
    std::cout << "Ran a total of " << nTests << " test on "
              << this->tests.size() << " objects. " << nSuccessful
              << " succeeded. ("
              << 100.0 * double(this->nSuccessful) / double(this->nTests)
              << "%)."
              << "\033[0m" << std::endl;
    auto f = std::ofstream("testWrapper/out/results.txt", std::ios_base::app);
    f << this->name << "," << this->nTests << "," << this->tests.size() << ","
      << this->nSuccessful << std::endl;
    return nSuccessful == nTests;
  }

  bool addTest(Component<Stereo<signal_t>>& componentObj,
      std::string objName,
      std::vector<std::string> stimuli) {
    this->tests.push_back(std::make_unique<NtFx::ComponentTest<float>>(
        this->name, objName, componentObj, stimuli));
    return true;
  }

  // TODO: dox
  int runAllTests() {
    bool success = true;
    for (auto& stimulus : STIMULI_NAMES) {
      for (size_t i = 0; i < tests.size(); i++) {
        this->nTests++;
        success &= tests[i]->run(stimulus);
        if (success) { this->nSuccessful++; }
      }
    }
    return !this->getResults();
  }
};

// TODO: This is a bad idea. Where do we add tests?
template <typename signal_t>
struct ComponentTestMaster {
  std::unordered_map<std::string, std::unique_ptr<ComponentTestSet<signal_t>>>
      allSets;
  void addSet(std::string name, NtFx::ComponentTestSet<float>& set) {
    this->allSets.insert({ name, set });
  }
  int runAllTestSets() {
    int res = 0;
    for (auto& set : this->allSets) { res |= set.second->runAllTests(); }
    return res;
  }
};
}
