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

#pragma once

#include "lib/Stereo.h"

namespace NtFx {
/**
 * @brief Base class for audio components in NtPlugin. All processors adhere to
 * this interface.
 *
 * @tparam signal_t Datatype for audio signals.
 */
template <typename T>
struct Component {
  /**
   * @brief Sample rate of component.
   *
   */
  float fs;

  /**
   * @brief Called for every sample as audio is processed.
   *
   * @param x T Input
   * @return T output.
   */
  virtual T process(T x) noexcept = 0;

  /**
   * @brief Called when ever a parameter (knob or toggle) changes. Update your
   * coefficients here.
   *
   */
  virtual void update() noexcept { }

  /**
   * @brief Called when the plugin loads and when ever the samplerate or buffer
   * size changes.
   *
   * @param fs Sample rate. If you need it for calculateCoeffs, store it.
   */
  virtual void reset(float fs) noexcept {
    this->fs = fs;
    this->update();
  };
};

/**
 * @brief Wraps two objects of type Component<signal_t> as a
 * Component<Stereo<signal_t>> and calls through the both objects.
 *
 * @tparam signal_t Datatype for signal. Must be a floating point type.
 * @tparam component_t Type of Component to wrap.
 */
template <typename signal_t, typename component_t>
struct StereoComponent : public Component<Stereo<signal_t>> {
  // TODO: constraint component_t to Component base class.
  component_t l;
  component_t r;

  virtual Stereo<signal_t> process(Stereo<signal_t> x) noexcept override {
    return { this->l.process(x.l), this->r.process(x.r) };
  }

  virtual void update() noexcept override {
    this->l.update();
    this->r.update();
  }

  virtual void reset(float fs) noexcept override {
    this->l.reset(fs);
    this->r.reset(fs);
  }
};
}
