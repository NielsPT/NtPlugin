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
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdlib.h>
#include <string>
#include <vector>

// https://stackoverflow.com/questions/15884793/how-to-get-the-name-or-file-and-line-of-caller-method
// #include <source_location>
// #include <string_view>
// template <typename T>
// consteval auto _func_name() {
//   const auto& loc = std::source_location::current();
//   return loc.function_name();
// }

// template <typename T>
// consteval std::string_view _type_of_impl() {
//   constexpr std::string_view functionName = _func_name<T>();
//   // since func_name_ is 'consteval auto func_name() [with T = ...]'
//   // we can simply get the subrange
//   // because the position after the equal will never change since
//   // the same function name is used

//   // another notice: these magic numbers will not work on MSVC or GCC
//   // TODO: Make this work on all platforms.
//   return { functionName.begin() + 23, functionName.end() - 3 };
// }

// template <typename T>
// constexpr auto type_of(T&& arg) {
//   return _type_of_impl<decltype(arg)>();
// }

// template <typename T>
// constexpr auto type_of() {
//   return _type_of_impl<T>();
// }

// template <typename T>
// constexpr auto componentNameOf(T&& arg) {
//   auto componentName          = type_of(arg);
//   auto n                      = componentName.length();
//   auto _end                   = componentName.find("<float");
//   auto nameWithSpace          = componentName.substr(0, _end);
//   auto _begin                 = nameWithSpace.rfind("::") + 2;
//   auto componentNameDemangled = nameWithSpace.substr(_begin, n);
//   return std::string(componentNameDemangled);
// }

constexpr auto testFileBaseName(std::string fileName) {
  auto _begin        = fileName.find("/") + 1;
  auto n             = fileName.length();
  auto baseName      = fileName.substr(_begin, n);
  auto _end          = baseName.find(".cpp");
  auto baseNameNoExt = baseName.substr(0, _end);
  _end               = baseNameNoExt.find("_test");
  baseNameNoExt      = baseNameNoExt.substr(0, _end);
  return baseNameNoExt;
}

#define QUOTE(str) #str
#define EXPAND_AND_QUOTE(str) QUOTE(str)

#define _ADD_TEST_IMPL(object, stimuli)                                        \
  NtFx::ComponentTest<float>::addTest(object,                                  \
      EXPAND_AND_QUOTE(object),                                                \
      testFileBaseName(__FILE__),                                              \
      { stimuli })
// object, EXPAND_AND_QUOTE(object), componentNameOf(object), { stimuli })

// TODO: Copy the component somewhere.
#define ADD_TEST(object, stimuli) _ADD_TEST_IMPL(object, stimuli)

#define NTFX_TEST_BEGIN                                                        \
  namespace NtFx {                                                             \
    template <>                                                                \
    int NtFx::ComponentTest<float>::nTests = 0;                                \
    template <>                                                                \
    int NtFx::ComponentTest<float>::nSuccessful = 0;                           \
    template <>                                                                \
    int NtFx::ComponentTest<float>::nComponents = 0;                           \
    template <>                                                                \
    std::vector<std::unique_ptr<NtFx::ComponentTest<float>>>                   \
        NtFx::ComponentTest<float>::objects { };                               \
    template <>                                                                \
    std::vector<std::string> NtFx::ComponentTest<float>::objectNames { };      \
    template <>                                                                \
    std::vector<std::string> NtFx::ComponentTest<float>::testFilesNames { };   \
  }

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
  static int nComponents; ///< Number of components tested.
  static int nTests;      ///< Total tests run.
  static int nSuccessful; ///< Number of successful tests.
  static std::vector<std::unique_ptr<ComponentTest<signal_t>>> objects;
  static std::vector<std::string> objectNames;
  static std::vector<std::string> testFilesNames;

  Component<Stereo<signal_t>>& cut; ///< Component under test.

  std::vector<std::string> activeStimuli =
      STIMULI_NAMES; ///< List of names of tests to run.

  // TODO: dox
  ComponentTest(
      Component<Stereo<signal_t>>& cut, std::vector<std::string> stimuli = { })
      : cut(cut) {
    nComponents++;
    if (stimuli.empty()) { return; }
    for (auto& x : stimuli) {
      if (std::find(STIMULI_NAMES.begin(), STIMULI_NAMES.end(), x)
          == STIMULI_NAMES.end()) {
        std::cout << "Stimuli name '" << x << "' not accepted.";
        this->activeStimuli.clear();
        return;
      }
    }
    this->activeStimuli = stimuli;
  }

  static bool addTest(Component<Stereo<signal_t>>& componentObj,
      std::string objectName,
      std::string fileBaseName,
      std::vector<std::string> stimuliToUse = { }) {
    if (std::find(objectNames.begin(), objectNames.end(), objectName)
        != objectNames.end()) {
      std::cout << "Test '" << objectName << "' already registered."
                << std::endl;
      return false;
    }
    NtFx::ComponentTest<float>::objectNames.push_back(objectName);
    NtFx::ComponentTest<float>::objects.push_back(
        std::make_unique<NtFx::ComponentTest<float>>(
            componentObj, stimuliToUse));
    NtFx::ComponentTest<float>::testFilesNames.push_back(fileBaseName);
    return true;
  }

  // TODO: dox
  bool run(std::string object,
      std::string testFile,
      std::string stimulus,
      float fs = 48.0e3f) {
    if (std::find(
            this->activeStimuli.begin(), this->activeStimuli.end(), stimulus)
        == this->activeStimuli.end()) {
      return true;
    }
    this->nTests++;
    auto testName = object + SEPARATOR + stimulus;
    auto xPath    = "testWrapper/in/" + stimulus + ".txt";

    if (!std::filesystem::exists(xPath)) {
      std::cout << "Input file '" << xPath << "'not found. Aborting test."
                << std::endl;
      return false;
    }
    auto yPath = "testWrapper/out/" + testFile + SEPARATOR + testName
        + SEPARATOR + "result.txt";
    auto expPath = "testWrapper/in/" + testFile + SEPARATOR + testName
        + SEPARATOR + "expected.txt";
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
    std::cout << "Test '" << stimulus << "' for object '" << object
              << "' of component '" << testFile << "'";
    std::cout << (success ? " passed." : " failed.") << "\033[0m" << std::endl;
    if (success) { nSuccessful++; }
    return success;
  }

  static int runAllTests() {
    auto n = objects.size();
    if (n != objectNames.size()) {
      std::cout << "Test name look up mismatch." << std::endl;
      return false;
    }
    for (auto& stimulus : STIMULI_NAMES) {
      for (size_t i = 0; i < n; i++) {
        objects[i]->run(objectNames[i], testFilesNames[i], stimulus);
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
    // TODO: Count components.
    std::cout << "Ran a total of " << nTests << " test on " << nComponents
              << " objects. " << nSuccessful << " succeeded. ("
              << 100.0 * double(nSuccessful) / double(nTests) << "%)."
              << "\033[0m" << std::endl;
    return nSuccessful == nTests;
  }
};
}
