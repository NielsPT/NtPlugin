
/**
 * @file DelayLine.h
 * @author Niels Thøgersen (niels.thoegersen@gmail.com)
 * @brief Audio delay lines.
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
#include "lib/Component.h"
namespace NtFx {
namespace Delay {
  template <int dlLen>
  struct ShortDelayLine : public ComponentBase<Audio> {
    signal_t t_ms { 0.25 };
    int n { 0 };
    int i { 0 };
    std::array<Audio, dlLen> dl;
    ShortDelayLine() { }
    virtual Audio process(Audio x) noexcept override {
      this->dl[this->i++] = x;
      if (this->i >= dlLen) { this->i = 0; }
      auto _i = this->i - this->n;
      if (_i < 0) { _i += dlLen; }
      return this->dl[_i];
    }
    virtual void update() noexcept override {
      this->n = int(this->t_ms * 0.001 * this->fs);
      if (this->n >= dlLen) { this->n = dlLen; }
    }
    virtual void reset(float fs) noexcept override {
      this->fs = fs;
      std::fill(this->dl.begin(), this->dl.end(), 0);
      this->update();
    }
  };

}
}
