#pragma once

/**
 * @file DryMix.h
 * @author Niels Thøgersen (niels.thoegersen@gmail.com)
 * @brief Simple dry mix component.
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

#include "lib/Audio.h"

namespace NtFx {
struct DryMix {
  signal_t mix_p { 100 };
  signal_t _dryMix_lin { 0 };
  signal_t _wetMix_lin { 1 };

  Audio process(Audio wet, Audio dry) noexcept {
    return wet * this->_wetMix_lin + dry * this->_dryMix_lin;
  }

  void update() {
    this->_dryMix_lin = signal_t(gcem::sqrt(1 - mix_p / 100.0));
    this->_wetMix_lin = signal_t(gcem::sqrt(mix_p / 100.0));
  }
};
}