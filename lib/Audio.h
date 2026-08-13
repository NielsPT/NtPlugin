#pragma once

/**
 * @file Stereo.h
 * @author Niels Thøgersen (niels.thoegersen@gmail.com)
 * @brief A data type for stereo signals.
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

#include "lib/gcem.h"
#include "lib/utils.h"
#include <cstddef>
#include <type_traits>

namespace NtFx {

template <typename signal_t>
struct Stereo;
template <typename signal_t>
struct Mono {
  signal_t l { 0 };
  Mono() : l(0) { }
  Mono(signal_t v) : l(v) { }
  Mono(signal_t _l, signal_t _r) : l((_l + _r) / 2) { }
  Mono(Stereo<signal_t> x) : l(x.l, x.r) { }

  Mono(Mono&&) noexcept            = default;
  Mono(Mono&) noexcept             = default;
  ~Mono()                          = default;
  Mono& operator=(Mono&&) noexcept = default;
  Mono<signal_t>& operator=(const Mono<signal_t>& x) noexcept {
    this->l = x.l;
    return *this;
  }
  Mono<signal_t>& operator=(const signal_t& x) noexcept {
    this->l = x;
    return *this;
  }
  bool operator==(const Mono<signal_t>& x) const noexcept {
    return (this->l == x.l);
  }
  bool operator<(const Mono<signal_t>& x) const noexcept {
    return this->absMin() < x.absMin();
  }
  bool operator>(const Mono<signal_t>& x) const noexcept {
    return this->absMax() > x.absMax();
  }
  bool operator<=(const Mono<signal_t>& x) const noexcept {
    return this->absMin() <= x.absMin();
  }
  bool operator>=(const Mono<signal_t>& x) const noexcept {
    return this->absMax() >= x.absMax();
  }
  bool operator<(const signal_t& x) const noexcept {
    return this->absMin() < x;
  }
  bool operator>(const signal_t& x) const noexcept {
    return this->absMax() > x;
  }
  bool operator<=(const signal_t& x) const noexcept {
    return this->absMin() <= x;
  }
  bool operator>=(const signal_t& x) const noexcept {
    return this->absMax() >= x;
  }

  Mono<signal_t>& operator*=(const signal_t x) noexcept {
    this->l = this->l * x;
    return *this;
  }
  Mono<signal_t>& operator*=(const Mono<signal_t> x) noexcept {
    this->l = this->l * x.l;
    return *this;
  }
  Mono<signal_t>& operator+=(const signal_t x) noexcept {
    this->l = this->l + x;
    return *this;
  }
  Mono<signal_t>& operator+=(const Mono<signal_t> x) noexcept {
    this->l = this->l + x.l;
    return *this;
  }
  Mono<signal_t> operator-() const { return -this->l; }
  /**
   * @brief Square left and right channels and take the average.
   *
   * @return signal_t Result.
   */
  signal_t avgSquared() const noexcept { return this->l; }

  /**
   * @brief Returns the larges of left and right disregarding sign.
   *
   * @return signal_t
   */
  signal_t absMax() const noexcept { return gcem::abs(this->l); }

  /**
   * @brief Returns the smallest of left and right disregarding sign.
   *
   * @return signal_t
   */
  signal_t absMin() const noexcept { return gcem::abs(this->l); }

  /**
   * @brief Returns the abs of both channels.
   *
   * @return Mono<signal_t>
   */
  Mono<signal_t> abs() const noexcept { return gcem::abs(this->l); }
};

template <typename signal_t>
static inline Mono<signal_t> operator+(
    const Mono<signal_t>& y, const signal_t& x) noexcept {
  return y.l + x;
}
template <typename signal_t>
static inline Mono<signal_t> operator-(
    const Mono<signal_t>& y, const signal_t& x) noexcept {
  return y.l - x;
}
template <typename signal_t>
static inline Mono<signal_t> operator*(
    const Mono<signal_t>& y, const signal_t& x) noexcept {
  return y.l * x;
}
template <typename signal_t>
static inline Mono<signal_t> operator/(
    const Mono<signal_t>& y, const signal_t& x) noexcept {
  return y.l / x;
}
template <typename signal_t>
static inline Mono<signal_t> operator+(
    const Mono<signal_t>& y, const int& x) noexcept {
  return y.l + x;
}
template <typename signal_t>
static inline Mono<signal_t> operator-(
    const Mono<signal_t>& y, const int& x) noexcept {
  return y.l - x;
}
template <typename signal_t>
static inline Mono<signal_t> operator*(
    const Mono<signal_t>& y, const int& x) noexcept {
  return y.l * x;
}
template <typename signal_t>
static inline Mono<signal_t> operator/(
    const Mono<signal_t>& y, const int& x) noexcept {
  return y.l / x;
}
template <typename signal_t>
static inline Mono<signal_t> operator+(
    const Mono<signal_t>& y, const Mono<signal_t>& x) noexcept {
  return y.l + x.l;
}
template <typename signal_t>
static inline Mono<signal_t> operator-(
    const Mono<signal_t>& y, const Mono<signal_t>& x) noexcept {
  return y.l - x.l;
}
template <typename signal_t>
static inline Mono<signal_t> operator*(
    const Mono<signal_t>& y, const Mono<signal_t>& x) noexcept {
  return y.l * x.l;
}
template <typename signal_t>
static inline Mono<signal_t> operator/(
    const Mono<signal_t>& y, const Mono<signal_t>& x) noexcept {
  return y.l / x.l;
}
template <typename signal_t>
static inline Mono<signal_t> operator+(
    const signal_t& x, const Mono<signal_t>& y) noexcept {
  return x + y.l;
}
template <typename signal_t>
static inline Mono<signal_t> operator-(
    const signal_t& x, const Mono<signal_t>& y) noexcept {
  return x - y.l;
}
template <typename signal_t>
static inline Mono<signal_t> operator*(
    const signal_t& x, const Mono<signal_t>& y) noexcept {
  return x * y.l;
}
template <typename signal_t>
static inline Mono<signal_t> operator/(
    const signal_t& x, const Mono<signal_t>& y) noexcept {
  return x / y.l;
}
template <typename signal_t>
static inline Mono<signal_t> operator*(
    const Mono<signal_t>& y, const size_t& x) noexcept {
  return y.l * signal_t(x);
}
template <typename signal_t>
static inline Mono<signal_t> operator*(
    const size_t x, const Mono<signal_t>& y) noexcept {
  return y.l * signal_t(x);
}
template <typename signal_t>
static inline void ensureFinite(
    Mono<signal_t>& x, signal_t def = signal_t(0)) noexcept {
  ensureFinite(x.l, def);
}

/**
 * @brief Wraps two values as a stereo signal.This allows us to do DSP math on
 * both channels at the same time. The class overloads all the normal math
 * functions. Please note that comparison operators are different for stereo
 * signals and that a < b not necessarily equals b > a since '<' compares the
 * minimum values while '>' compares maximums.
 *
 * @tparam signal_t Audio datatype.
 */
template <typename signal_t>
struct Stereo {

  signal_t l { 0 };
  signal_t r { 0 };
  Stereo() : l(0.0f), r(0.0f) {
    static_assert(
        std::is_floating_point_v<signal_t>, "Type must be floating point.");
  }
  Stereo(signal_t x) : l(x), r(x) { }
  Stereo(signal_t _l, signal_t _r) : l(_l), r(_r) { }
  Stereo(Mono<signal_t> _l, Mono<signal_t> _r) : l(_l.l), r(_r.l) { }
  Stereo(Stereo&& x) noexcept : l(x.l), r(x.r) { }
  Stereo(Stereo& x) noexcept : l(x.l), r(x.r) { }
  Stereo(const Stereo& x) noexcept : l(x.l), r(x.r) { }
  ~Stereo()                            = default;
  Stereo& operator=(Stereo&&) noexcept = default;
  Stereo<signal_t>& operator=(const Stereo<signal_t>& x) noexcept {
    this->l = x.l;
    this->r = x.r;
    return *this;
  }

  Stereo<signal_t>& operator=(const signal_t& x) noexcept {
    this->l = x;
    this->r = x;
    return *this;
  }
  bool operator==(const Stereo<signal_t>& x) const noexcept {
    return (this->l == x.l) && (this->r == x.r);
  }
  bool operator<(const Stereo<signal_t>& x) const noexcept {
    return this->absMin() < x.absMin();
  }
  bool operator>(const Stereo<signal_t>& x) const noexcept {
    return this->absMax() > x.absMax();
  }
  bool operator<=(const Stereo<signal_t>& x) const noexcept {
    return this->absMin() <= x.absMin();
  }
  bool operator>=(const Stereo<signal_t>& x) const noexcept {
    return this->absMax() >= x.absMax();
  }
  bool operator<(const signal_t& x) const noexcept {
    return this->absMin() < x;
  }
  bool operator>(const signal_t& x) const noexcept {
    return this->absMax() > x;
  }
  bool operator<=(const signal_t& x) const noexcept {
    return this->absMin() <= x;
  }
  bool operator>=(const signal_t& x) const noexcept {
    return this->absMax() >= x;
  }

  Stereo<signal_t>& operator*=(const signal_t x) noexcept {
    this->l = this->l * x;
    this->r = this->r * x;
    return *this;
  }
  Stereo<signal_t>& operator*=(const Stereo<signal_t> x) noexcept {
    this->l = this->l * x.l;
    this->r = this->r * x.r;
    return *this;
  }
  Stereo<signal_t>& operator+=(const signal_t x) noexcept {
    this->l = this->l + x;
    this->r = this->r + x;
    return *this;
  }
  Stereo<signal_t>& operator+=(const Stereo<signal_t> x) noexcept {
    this->l = this->l + x.l;
    this->r = this->r + x.r;
    return *this;
  }
  Stereo<signal_t> operator-() const { return { -this->l, -this->r }; }

  /**
   * @brief Square left and right channels and take the average.
   *
   * @return signal_t Result.
   */
  signal_t avgSquared() const noexcept {
    return (this->l * this->l + this->r * this->r) * 0.5f;
  }

  /**
   * @brief Returns the larges of left and right disregarding sign.
   *
   * @return signal_t
   */
  signal_t absMax() const noexcept {
    return gcem::abs(this->l) > gcem::abs(this->r) ? this->l : this->r;
  }

  /**
   * @brief Returns the smallest of left and right disregarding sign.
   *
   * @return signal_t
   */
  signal_t absMin() const noexcept {
    return gcem::abs(this->l) < gcem::abs(this->r) ? this->l : this->r;
  }

  /**
   * @brief Returns the abs of both channels.
   *
   * @return Stereo<signal_t>
   */
  Stereo<signal_t> abs() const noexcept {
    return { gcem::abs(this->l), gcem::abs(this->r) };
  }
};
template <typename signal_t>
static inline Stereo<signal_t> operator+(
    const Stereo<signal_t>& y, const signal_t& x) noexcept {
  return { (y.l + x), (y.r + x) };
}
template <typename signal_t>
static inline Stereo<signal_t> operator-(
    const Stereo<signal_t>& y, const signal_t& x) noexcept {
  return { (y.l - x), (y.r - x) };
}
template <typename signal_t>
static inline Stereo<signal_t> operator*(
    const Stereo<signal_t>& y, const signal_t& x) noexcept {
  return { (y.l * x), (y.r * x) };
}
template <typename signal_t>
static inline Stereo<signal_t> operator/(
    const Stereo<signal_t>& y, const signal_t& x) noexcept {
  return { (y.l / x), (y.r / x) };
}
template <typename signal_t>
static inline Stereo<signal_t> operator+(
    const Stereo<signal_t>& y, const int& x) noexcept {
  return { (y.l + x), (y.r + x) };
}
template <typename signal_t>
static inline Stereo<signal_t> operator-(
    const Stereo<signal_t>& y, const int& x) noexcept {
  return { (y.l - x), (y.r - x) };
}
template <typename signal_t>
static inline Stereo<signal_t> operator*(
    const Stereo<signal_t>& y, const int& x) noexcept {
  return { (y.l * x), (y.r * x) };
}
template <typename signal_t>
static inline Stereo<signal_t> operator/(
    const Stereo<signal_t>& y, const int& x) noexcept {
  return { (y.l / x), (y.r / x) };
}
template <typename signal_t>
static inline Stereo<signal_t> operator+(
    const Stereo<signal_t>& y, const Stereo<signal_t>& x) noexcept {
  return { (y.l + x.l), (y.r + x.r) };
}
template <typename signal_t>
static inline Stereo<signal_t> operator-(
    const Stereo<signal_t>& y, const Stereo<signal_t>& x) noexcept {
  return { (y.l - x.l), (y.r - x.r) };
}
template <typename signal_t>
static inline Stereo<signal_t> operator*(
    const Stereo<signal_t>& y, const Stereo<signal_t>& x) noexcept {
  return { (y.l * x.l), (y.r * x.r) };
}
template <typename signal_t>
static inline Stereo<signal_t> operator/(
    const Stereo<signal_t>& y, const Stereo<signal_t>& x) noexcept {
  return { (y.l / x.l), (y.r / x.r) };
}
template <typename signal_t>
static inline Stereo<signal_t> operator+(
    const signal_t& x, const Stereo<signal_t>& y) noexcept {
  return { (x + y.l), (x + y.r) };
}
template <typename signal_t>
static inline Stereo<signal_t> operator-(
    const signal_t& x, const Stereo<signal_t>& y) noexcept {
  return { (x - y.l), (x - y.r) };
}
template <typename signal_t>
static inline Stereo<signal_t> operator*(
    const signal_t& x, const Stereo<signal_t>& y) noexcept {
  return { (x * y.l), (x * y.r) };
}
template <typename signal_t>
static inline Stereo<signal_t> operator/(
    const signal_t& x, const Stereo<signal_t>& y) noexcept {
  return { (x / y.l), (x / y.r) };
}
template <typename signal_t>
static inline Stereo<signal_t> operator*(
    const Stereo<signal_t>& y, const size_t& x) noexcept {
  return { (y.l * signal_t(x)), (y.r * signal_t(x)) };
}
template <typename signal_t>
static inline Stereo<signal_t> operator*(
    const size_t x, const Stereo<signal_t>& y) noexcept {
  return { (y.l * signal_t(x)), (y.r * signal_t(x)) };
}

/**
 * @brief Sets 'x' to 0 if it is not a finite number.
 *
 * @tparam signal_t Stereo datatype.
 * @param x Value to validate.
 * @param def Value to set 'x' to in case it's not a finite number.
 */
template <typename signal_t>
static inline void ensureFinite(
    Stereo<signal_t>& x, signal_t def = signal_t(0)) noexcept {
  ensureFinite(x.l, def);
  ensureFinite(x.r, def);
}
} // namespace NtFx

#ifdef NTFX_TESTING
typedef double signal_t;
#else
typedef float signal_t;
#endif
using Audio = NtFx::Stereo<signal_t>;