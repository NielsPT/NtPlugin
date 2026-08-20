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
 **/

#include "lib/utils.h"

#define NTFX_PLUGIN_NAME juce::String(NTFX_EXPAND_AND_QUOTE(NTFX_PLUGIN))

// https://stackoverflow.com/questions/38955940/how-to-concatenate-static-strings-at-compile-time
template <std::string_view const&... Strs>
struct join {
  // Join all strings into a single std::array of chars
  static constexpr auto impl() noexcept {
    constexpr std::size_t len = (Strs.size() + ... + 0);
    std::array<char, len + 1> arr { };
    auto append = [i = 0, &arr](auto const& s) mutable {
      for (auto c : s) arr[size_t(i++)] = c;
    };
    (append(Strs), ...);
    arr[len] = 0;
    return arr;
  }
  // Give the joined string static storage
  static constexpr auto _arr = impl();
  // View as a std::string_view
  static constexpr std::string_view value { _arr.data(), _arr.size() - 1 };
};
// Helper to get the value out
template <std::string_view const&... Strs>
static constexpr auto join_v = join<Strs...>::value;
