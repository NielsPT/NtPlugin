#pragma once

/**
 * @file Detection.h
 * @author Morten Hornbæk Korstgård (mortenhkorstgaard@gmail.com)
 * @brief Detection algorithm.
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
 */
#include "gcem.hpp"
#include "lib/Audio.h"
#include "lib/Component.h"
#include <span>
#include <vector>

namespace NtFx {
struct Detection : public ComponentBase<signal_t> {

  template <typename T>
  std::vector<double> weightedAutocorrelation(
      std::span<const T> samples, double k = 1.0) {
    const std::size_t N = samples.size();

    std::vector<double> result(N, 0.0);

    if (N == 0) return result;

    for (std::size_t lag = 0; lag < N; ++lag) {
      // AutoCorrelation Functioon
      double acf = 0.0;
      // Average Magnitude Difference Function
      double amdf = 0.0;

      for (std::size_t n = 0; n < N - lag; ++n) {
        const double a = static_cast<double>(samples[n]);

        const double b = static_cast<double>(samples[n + lag]);
        // x(n) * x(n+theta)
        acf += a * b;
        // |x(n) * x(n+theta)|
        amdf += gcem::abs(a - b);
      }

      acf /= static_cast<double>(N - lag);
      amdf /= static_cast<double>(N - lag);
      result[lag] = acf / (amdf + k);
    }

    return result;
  }

  virtual signal_t process(signal_t x) noexcept override {return {};}
};
} // namespace NtFx