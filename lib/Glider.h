#pragma once

/**
 * @file Glider.h
 * @author Niels Thøgersen (niels.thoegersen@gmail.com)
 * @brief Glider smooths out changes in parameters.
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
 */

#include "gcem.hpp"

// TODO: Make this a Component.
/**
 * @brief An exponential glider. Member value 'ui' is set from the UI side of
 * the plugin and 'pr' is read from the processing side.
 *
 */
namespace NtFx {
template <typename signal_t>
struct ExpGlider {
  signal_t ui = 0; ///< UI side input value.
  signal_t pr = 0; ///< Processing side output of glider.
  signal_t _a = 0;

  /**
   * @brief Construct a new Exp Glider object
   *
   * @param def Default value the glider should start at.
   */
  ExpGlider(signal_t def = 0) : ui(def), pr(def) { }

  /**
   * @brief Processes glider. To be called every sample.
   *
   * @return signal_t Audio datatype.
   */
  inline signal_t process() noexcept {
    this->pr += this->_a * (this->ui - this->pr);
    return this->pr;
  }

  /**
   * @brief Update the glider coefficient.
   *
   * @param fs Sample rate
   * @param tSmooth Smoothing time constant.
   */
  inline void update(signal_t fs, signal_t tSmooth) noexcept {
    if (tSmooth <= 0) {
      this->_a = 0;
      this->pr = this->ui;
      return;
    }
    this->_a = 1.0 - gcem::exp(-1.0 / (fs * tSmooth));
  }
};

/**
 * @brief Glides member 'pr' linearly towards 'ui'.
 *
 * @tparam signal_t
 */
template <typename signal_t>
struct LinGlider {
  signal_t ui = 0.0;
  signal_t pr = 0.0;
  signal_t s  = 0.0;

  /**
   * @brief Construct a new Lin Glider object
   *
   * @param def Default value to start glider at.
   */
  LinGlider(signal_t def = 0) : ui(def), pr(def) { }

  /**
   * @brief Processes glider. Should be called every sample.
   *
   * @return signal_t Audio datatype.
   */
  inline signal_t process() noexcept {
    if (this->ui == this->pr) { return this->pr; }
    if (this->pr < this->ui) {
      this->pr += s;
    } else {
      this->pr -= s;
    }
    if (gcem::abs(this->ui - this->pr) < gcem::abs(this->s)) {
      this->pr = this->ui;
    }
    return this->pr;
  }

  /**
   * @brief Updates glider coeff.
   *
   * @param fs Sample rate.
   * @param tSmooth Time to glide 'pr' to 'ui'.
   */
  inline void update(signal_t fs, signal_t tSmooth) {
    if (tSmooth <= 0) {
      this->pr = this->ui;
      return;
    }
    this->s = gcem::abs(this->ui - this->pr) / (tSmooth * fs);
  }
};
}
