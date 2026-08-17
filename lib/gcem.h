#pragma once

/*################################################################################
  ##
  ##   Copyright (C) 2016-2020 Keith O'Hara
  ##
  ##   This file is part of the GCE-Math C++ library.
  ##
  ##   Licensed under the Apache License, Version 2.0 (the "License");
  ##   you may not use this file except in compliance with the License.
  ##   You may obtain a copy of the License at
  ##
  ##       http://www.apache.org/licenses/LICENSE-2.0
  ##
  ##   Unless required by applicable law or agreed to in writing, software
  ##   distributed under the License is distributed on an "AS IS" BASIS,
  ##   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  ##   See the License for the specific language governing permissions and
  ##   limitations under the License.
  ##
  ################################################################################*/

#include <cstddef> // size_t
#include <limits>
#include <type_traits>

// undef macros in math.h
#undef abs
#undef round
#undef signbit

//
// version

#ifndef GCEM_VERSION_MAJOR
  #define GCEM_VERSION_MAJOR 1
#endif

#ifndef GCEM_VERSION_MINOR
  #define GCEM_VERSION_MINOR 13
#endif

#ifndef GCEM_VERSION_PATCH
  #define GCEM_VERSION_PATCH 1
#endif

//
// types

namespace gcem {
using uint_t   = unsigned int;
using ullint_t = unsigned long long int;

using llint_t = long long int;

template <class T>
using GCLIM = std::numeric_limits<T>;

template <typename T>
using return_t =
    typename std::conditional<std::is_integral<T>::value, double, T>::type;

template <typename... T>
using common_t = typename std::common_type<T...>::type;

template <typename... T>
using common_return_t = return_t<common_t<T...>>;
}

//
// constants

#ifndef GCEM_LOG_2
  #define GCEM_LOG_2 0.6931471805599453094172321214581765680755L
#endif

#ifndef GCEM_LOG_10
  #define GCEM_LOG_10 2.3025850929940456840179914546843642076011L
#endif

#ifndef GCEM_PI
  #define GCEM_PI 3.1415926535897932384626433832795028841972L
#endif

#ifndef GCEM_LOG_PI
  #define GCEM_LOG_PI 1.1447298858494001741434273513530587116473L
#endif

#ifndef GCEM_LOG_2PI
  #define GCEM_LOG_2PI 1.8378770664093454835606594728112352797228L
#endif

#ifndef GCEM_LOG_SQRT_2PI
  #define GCEM_LOG_SQRT_2PI 0.9189385332046727417803297364056176398614L
#endif

#ifndef GCEM_SQRT_2
  #define GCEM_SQRT_2 1.4142135623730950488016887242096980785697L
#endif

#ifndef GCEM_HALF_PI
  #define GCEM_HALF_PI 1.5707963267948966192313216916397514420986L
#endif

#ifndef GCEM_SQRT_PI
  #define GCEM_SQRT_PI 1.7724538509055160272981674833411451827975L
#endif

#ifndef GCEM_SQRT_HALF_PI
  #define GCEM_SQRT_HALF_PI 1.2533141373155002512078826424055226265035L
#endif

#ifndef GCEM_E
  #define GCEM_E 2.7182818284590452353602874713526624977572L
#endif

//
// convergence settings

#ifndef GCEM_ERF_MAX_ITER
  #define GCEM_ERF_MAX_ITER 60
#endif

#ifndef GCEM_ERF_INV_MAX_ITER
  #define GCEM_ERF_INV_MAX_ITER 55
#endif

#ifndef GCEM_EXP_MAX_ITER_SMALL
  #define GCEM_EXP_MAX_ITER_SMALL 25
#endif

// #ifndef GCEM_LOG_TOL
//     #define GCEM_LOG_TOL 1E-14
// #endif

#ifndef GCEM_LOG_MAX_ITER_SMALL
  #define GCEM_LOG_MAX_ITER_SMALL 25
#endif

#ifndef GCEM_LOG_MAX_ITER_BIG
  #define GCEM_LOG_MAX_ITER_BIG 255
#endif

#ifndef GCEM_INCML_BETA_TOL
  #define GCEM_INCML_BETA_TOL 1E-15
#endif

#ifndef GCEM_INCML_BETA_MAX_ITER
  #define GCEM_INCML_BETA_MAX_ITER 205
#endif

#ifndef GCEM_INCML_BETA_INV_MAX_ITER
  #define GCEM_INCML_BETA_INV_MAX_ITER 35
#endif

#ifndef GCEM_INCML_GAMMA_MAX_ITER
  #define GCEM_INCML_GAMMA_MAX_ITER 55
#endif

#ifndef GCEM_INCML_GAMMA_INV_MAX_ITER
  #define GCEM_INCML_GAMMA_INV_MAX_ITER 35
#endif

#ifndef GCEM_SQRT_MAX_ITER
  #define GCEM_SQRT_MAX_ITER 100
#endif

#ifndef GCEM_TAN_MAX_ITER
  #define GCEM_TAN_MAX_ITER 35
#endif

#ifndef GCEM_TANH_MAX_ITER
  #define GCEM_TANH_MAX_ITER 35
#endif

//
// Macros

#ifdef _MSC_VER
  #ifndef GCEM_SIGNBIT
    #define GCEM_SIGNBIT(x) _signbit(x)
  #endif
  #ifndef GCEM_COPYSIGN
    #define GCEM_COPYSIGN(x, y) _copysign(x, y)
  #endif
#else
  #ifndef GCEM_SIGNBIT
    #define GCEM_SIGNBIT(x) __builtin_signbit(x)
  #endif
  #ifndef GCEM_COPYSIGN
    #define GCEM_COPYSIGN(x, y) __builtin_copysign(x, y)
  #endif
#endif

/*################################################################################
  ##
  ##   Copyright (C) 2016-2020 Keith O'Hara
  ##
  ##   This file is part of the GCE-Math C++ library.
  ##
  ##   Licensed under the Apache License, Version 2.0 (the "License");
  ##   you may not use this file except in compliance with the License.
  ##   You may obtain a copy of the License at
  ##
  ##       http://www.apache.org/licenses/LICENSE-2.0
  ##
  ##   Unless required by applicable law or agreed to in writing, software
  ##   distributed under the License is distributed on an "AS IS" BASIS,
  ##   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  ##   See the License for the specific language governing permissions and
  ##   limitations under the License.
  ##
  ################################################################################*/

/*
 * Gauss-Legendre quadrature: 50 points
 */

namespace gcem {
static const long double gauss_legendre_50_points[50] = {
  -0.03109833832718887611232898966595L,
  0.03109833832718887611232898966595L,
  -0.09317470156008614085445037763960L,
  0.09317470156008614085445037763960L,
  -0.15489058999814590207162862094111L,
  0.15489058999814590207162862094111L,
  -0.21600723687604175684728453261710L,
  0.21600723687604175684728453261710L,
  -0.27628819377953199032764527852113L,
  0.27628819377953199032764527852113L,
  -0.33550024541943735683698825729107L,
  0.33550024541943735683698825729107L,
  -0.39341431189756512739422925382382L,
  0.39341431189756512739422925382382L,
  -0.44980633497403878914713146777838L,
  0.44980633497403878914713146777838L,
  -0.50445814490746420165145913184914L,
  0.50445814490746420165145913184914L,
  -0.55715830451465005431552290962580L,
  0.55715830451465005431552290962580L,
  -0.60770292718495023918038179639183L,
  0.60770292718495023918038179639183L,
  -0.65589646568543936078162486400368L,
  0.65589646568543936078162486400368L,
  -0.70155246870682225108954625788366L,
  0.70155246870682225108954625788366L,
  -0.74449430222606853826053625268219L,
  0.74449430222606853826053625268219L,
  -0.78455583290039926390530519634099L,
  0.78455583290039926390530519634099L,
  -0.82158207085933594835625411087394L,
  0.82158207085933594835625411087394L,
  -0.85542976942994608461136264393476L,
  0.85542976942994608461136264393476L,
  -0.88596797952361304863754098246675L,
  0.88596797952361304863754098246675L,
  -0.91307855665579189308973564277166L,
  0.91307855665579189308973564277166L,
  -0.93665661894487793378087494727250L,
  0.93665661894487793378087494727250L,
  -0.95661095524280794299774564415662L,
  0.95661095524280794299774564415662L,
  -0.97286438510669207371334410460625L,
  0.97286438510669207371334410460625L,
  -0.98535408404800588230900962563249L,
  0.98535408404800588230900962563249L,
  -0.99403196943209071258510820042069L,
  0.99403196943209071258510820042069L,
  -0.99886640442007105018545944497422L,
  0.99886640442007105018545944497422L
};

static const long double gauss_legendre_50_weights[50] = {
  0.06217661665534726232103310736061L,
  0.06217661665534726232103310736061L,
  0.06193606742068324338408750978083L,
  0.06193606742068324338408750978083L,
  0.06145589959031666375640678608392L,
  0.06145589959031666375640678608392L,
  0.06073797084177021603175001538481L,
  0.06073797084177021603175001538481L,
  0.05978505870426545750957640531259L,
  0.05978505870426545750957640531259L,
  0.05860084981322244583512243663085L,
  0.05860084981322244583512243663085L,
  0.05718992564772838372302931506599L,
  0.05718992564772838372302931506599L,
  0.05555774480621251762356742561227L,
  0.05555774480621251762356742561227L,
  0.05371062188899624652345879725566L,
  0.05371062188899624652345879725566L,
  0.05165570306958113848990529584010L,
  0.05165570306958113848990529584010L,
  0.04940093844946631492124358075143L,
  0.04940093844946631492124358075143L,
  0.04695505130394843296563301363499L,
  0.04695505130394843296563301363499L,
  0.04432750433880327549202228683039L,
  0.04432750433880327549202228683039L,
  0.04152846309014769742241197896407L,
  0.04152846309014769742241197896407L,
  0.03856875661258767524477015023639L,
  0.03856875661258767524477015023639L,
  0.03545983561514615416073461100098L,
  0.03545983561514615416073461100098L,
  0.03221372822357801664816582732300L,
  0.03221372822357801664816582732300L,
  0.02884299358053519802990637311323L,
  0.02884299358053519802990637311323L,
  0.02536067357001239044019487838544L,
  0.02536067357001239044019487838544L,
  0.02178024317012479298159206906269L,
  0.02178024317012479298159206906269L,
  0.01811556071348939035125994342235L,
  0.01811556071348939035125994342235L,
  0.01438082276148557441937890892732L,
  0.01438082276148557441937890892732L,
  0.01059054838365096926356968149924L,
  0.01059054838365096926356968149924L,
  0.00675979919574540150277887817799L,
  0.00675979919574540150277887817799L,
  0.00290862255315514095840072434286L,
  0.00290862255315514095840072434286L
};
/*################################################################################
  ##
  ##   Copyright (C) 2016-2020 Keith O'Hara
  ##
  ##   This file is part of the GCE-Math C++ library.
  ##
  ##   Licensed under the Apache License, Version 2.0 (the "License");
  ##   you may not use this file except in compliance with the License.
  ##   You may obtain a copy of the License at
  ##
  ##       http://www.apache.org/licenses/LICENSE-2.0
  ##
  ##   Unless required by applicable law or agreed to in writing, software
  ##   distributed under the License is distributed on an "AS IS" BASIS,
  ##   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  ##   See the License for the specific language governing permissions and
  ##   limitations under the License.
  ##
  ################################################################################*/

/*
 * compile-time check if a float is +/-Inf
 */

namespace internal {

  template <typename T>
  constexpr bool is_neginf(const T x) noexcept {
    return x == -GCLIM<T>::infinity();
  }

  template <typename T1, typename T2>
  constexpr bool any_neginf(const T1 x, const T2 y) noexcept {
    return (is_neginf(x) || is_neginf(y));
  }

  template <typename T1, typename T2>
  constexpr bool all_neginf(const T1 x, const T2 y) noexcept {
    return (is_neginf(x) && is_neginf(y));
  }

  template <typename T1, typename T2, typename T3>
  constexpr bool any_neginf(const T1 x, const T2 y, const T3 z) noexcept {
    return (is_neginf(x) || is_neginf(y) || is_neginf(z));
  }

  template <typename T1, typename T2, typename T3>
  constexpr bool all_neginf(const T1 x, const T2 y, const T3 z) noexcept {
    return (is_neginf(x) && is_neginf(y) && is_neginf(z));
  }

  //

  template <typename T>
  constexpr bool is_posinf(const T x) noexcept {
    return x == GCLIM<T>::infinity();
  }

  template <typename T1, typename T2>
  constexpr bool any_posinf(const T1 x, const T2 y) noexcept {
    return (is_posinf(x) || is_posinf(y));
  }

  template <typename T1, typename T2>
  constexpr bool all_posinf(const T1 x, const T2 y) noexcept {
    return (is_posinf(x) && is_posinf(y));
  }

  template <typename T1, typename T2, typename T3>
  constexpr bool any_posinf(const T1 x, const T2 y, const T3 z) noexcept {
    return (is_posinf(x) || is_posinf(y) || is_posinf(z));
  }

  template <typename T1, typename T2, typename T3>
  constexpr bool all_posinf(const T1 x, const T2 y, const T3 z) noexcept {
    return (is_posinf(x) && is_posinf(y) && is_posinf(z));
  }

  //

  template <typename T>
  constexpr bool is_inf(const T x) noexcept {
    return (is_neginf(x) || is_posinf(x));
  }

  template <typename T1, typename T2>
  constexpr bool any_inf(const T1 x, const T2 y) noexcept {
    return (is_inf(x) || is_inf(y));
  }

  template <typename T1, typename T2>
  constexpr bool all_inf(const T1 x, const T2 y) noexcept {
    return (is_inf(x) && is_inf(y));
  }

  template <typename T1, typename T2, typename T3>
  constexpr bool any_inf(const T1 x, const T2 y, const T3 z) noexcept {
    return (is_inf(x) || is_inf(y) || is_inf(z));
  }

  template <typename T1, typename T2, typename T3>
  constexpr bool all_inf(const T1 x, const T2 y, const T3 z) noexcept {
    return (is_inf(x) && is_inf(y) && is_inf(z));
  }

}

/*
 * compile-time check if a float is NaN-valued
 */

namespace internal {

  // future: consider using __builtin_isnan(__x)

  template <typename T>
  constexpr bool is_nan(const T x) noexcept {
    return x != x;
  }

  template <typename T1, typename T2>
  constexpr bool any_nan(const T1 x, const T2 y) noexcept {
    return (is_nan(x) || is_nan(y));
  }

  template <typename T1, typename T2>
  constexpr bool all_nan(const T1 x, const T2 y) noexcept {
    return (is_nan(x) && is_nan(y));
  }

  template <typename T1, typename T2, typename T3>
  constexpr bool any_nan(const T1 x, const T2 y, const T3 z) noexcept {
    return (is_nan(x) || is_nan(y) || is_nan(z));
  }

  template <typename T1, typename T2, typename T3>
  constexpr bool all_nan(const T1 x, const T2 y, const T3 z) noexcept {
    return (is_nan(x) && is_nan(y) && is_nan(z));
  }

}

/*
 * compile-time check if a float is not NaN-valued or +/-Inf
 */

namespace internal {

  template <typename T>
  constexpr bool is_finite(const T x) noexcept {
    return (!is_nan(x)) && (!is_inf(x));
  }

  template <typename T1, typename T2>
  constexpr bool any_finite(const T1 x, const T2 y) noexcept {
    return (is_finite(x) || is_finite(y));
  }

  template <typename T1, typename T2>
  constexpr bool all_finite(const T1 x, const T2 y) noexcept {
    return (is_finite(x) && is_finite(y));
  }

  template <typename T1, typename T2, typename T3>
  constexpr bool any_finite(const T1 x, const T2 y, const T3 z) noexcept {
    return (is_finite(x) || is_finite(y) || is_finite(z));
  }

  template <typename T1, typename T2, typename T3>
  constexpr bool all_finite(const T1 x, const T2 y, const T3 z) noexcept {
    return (is_finite(x) && is_finite(y) && is_finite(z));
  }

}

/**
 * Compile-time absolute value function
 *
 * @param x a real-valued input.
 * @return the absolute value of \c x, \f$ |x| \f$.
 */

template <typename T>
constexpr T abs(const T x) noexcept {
  return ( // deal with signed-zeros
      x == T(0) ? T(0) :
                // else
          x < T(0) ? -x
                   : x);
}

namespace internal {

  template <typename T>
  constexpr int ceil_resid(const T x, const T x_whole) noexcept {
    return ((x > T(0)) && (x > x_whole));
  }

  template <typename T>
  constexpr T ceil_int(const T x, const T x_whole) noexcept {
    return (x_whole + static_cast<T>(ceil_resid(x, x_whole)));
  }

  template <typename T>
  constexpr T ceil_check(const T x) noexcept {
    return ( // NaN check
        is_nan(x) ? GCLIM<T>::quiet_NaN() :
                  // +/- infinite
            !is_finite(x) ? x
                          :
                          // signed-zero cases
            GCLIM<T>::epsilon() > abs(x) ? x
                                         :
                                         // else
            ceil_int(x, T(static_cast<llint_t>(x))));
  }

}

/**
 * Compile-time ceil function
 *
 * @param x a real-valued input.
 * @return computes the ceiling-value of the input.
 */

template <typename T>
constexpr return_t<T> ceil(const T x) noexcept {
  return internal::ceil_check(static_cast<return_t<T>>(x));
}

namespace internal {

  template <typename T>
  constexpr int floor_resid(const T x, const T x_whole) noexcept {
    return ((x < T(0)) && (x < x_whole));
  }

  template <typename T>
  constexpr T floor_int(const T x, const T x_whole) noexcept {
    return (x_whole - static_cast<T>(floor_resid(x, x_whole)));
  }

  template <typename T>
  constexpr T floor_check(const T x) noexcept {
    return ( // NaN check
        is_nan(x) ? GCLIM<T>::quiet_NaN() :
                  // +/- infinite
            !is_finite(x) ? x
                          :
                          // signed-zero cases
            GCLIM<T>::epsilon() > abs(x) ? x
                                         :
                                         // else
            floor_int(x, T(static_cast<llint_t>(x))));
  }

}

/**
 * Compile-time floor function
 *
 * @param x a real-valued input.
 * @return computes the floor-value of the input.
 */

template <typename T>
constexpr return_t<T> floor(const T x) noexcept {
  return internal::floor_check(static_cast<return_t<T>>(x));
}

namespace internal {

  template <typename T>
  constexpr T trunc_int(const T x) noexcept {
    return (T(static_cast<llint_t>(x)));
  }

  template <typename T>
  constexpr T trunc_check(const T x) noexcept {
    return ( // NaN check
        is_nan(x) ? GCLIM<T>::quiet_NaN() :
                  // +/- infinite
            !is_finite(x) ? x
                          :
                          // signed-zero cases
            GCLIM<T>::epsilon() > abs(x) ? x
                                         :
                                         // else
            trunc_int(x));
  }

}

/**
 * Compile-time trunc function
 *
 * @param x a real-valued input.
 * @return computes the trunc-value of the input, essentially returning the
 * integer part of the input.
 */

template <typename T>
constexpr return_t<T> trunc(const T x) noexcept {
  return internal::trunc_check(static_cast<return_t<T>>(x));
}

/*
 * compile-time check if integer is odd
 */

namespace internal {

  constexpr bool is_odd(const llint_t x) noexcept {
    // return( x % llint_t(2) == llint_t(0) ? false : true );
    return (x & 1U) != 0;
  }

}

/*
 * compile-time check if integer is even
 */

namespace internal {

  constexpr bool is_even(const llint_t x) noexcept { return !is_odd(x); }

}

/**
 * Compile-time pairwise maximum function
 *
 * @param x a real-valued input.
 * @param y a real-valued input.
 * @return Computes the maximum between \c x and \c y, where \c x and \c y have
 * the same type (e.g., \c int, \c double, etc.)
 */

template <typename T1, typename T2>
constexpr common_t<T1, T2> max(const T1 x, const T2 y) noexcept {
  return (y < x ? x : y);
}

/**
 * Compile-time pairwise minimum function
 *
 * @param x a real-valued input.
 * @param y a real-valued input.
 * @return Computes the minimum between \c x and \c y, where \c x and \c y have
 * the same type (e.g., \c int, \c double, etc.)
 */

template <typename T1, typename T2>
constexpr common_t<T1, T2> min(const T1 x, const T2 y) noexcept {
  return (y > x ? x : y);
}

/*
 * compile-time square-root function
 */

namespace internal {

  template <typename T>
  constexpr T sqrt_recur(const T x, const T xn, const int count) noexcept {
    return (abs(xn - x / xn) / (T(1) + xn) < GCLIM<T>::epsilon()
            ? // if
            xn
            : count < GCEM_SQRT_MAX_ITER ? // else
                sqrt_recur(x, T(0.5) * (xn + x / xn), count + 1)
                                         : xn);
  }

  template <typename T>
  constexpr T sqrt_check(const T x, const T m_val) noexcept {
    return (is_nan(x) ? GCLIM<T>::quiet_NaN() :
                      //
            x < T(0) ? GCLIM<T>::quiet_NaN()
                     :
                     //
            is_posinf(x) ? x
                         :
                         // indistinguishable from zero or one
            GCLIM<T>::epsilon() > abs(x)          ? T(0)
            : GCLIM<T>::epsilon() > abs(T(1) - x) ? x
                                                  :
                                                  // else
            x > T(4) ? sqrt_check(x / T(4), T(2) * m_val)
                     : m_val * sqrt_recur(x, x / T(2), 0));
  }

}

/**
 * Compile-time square-root function
 *
 * @param x a real-valued input.
 * @return Computes \f$ \sqrt{x} \f$ using a Newton-Raphson approach.
 */

template <typename T>
constexpr return_t<T> sqrt(const T x) noexcept {
  return internal::sqrt_check(static_cast<return_t<T>>(x), return_t<T>(1));
}

/**
 * Compile-time sign bit detection function
 *
 * @param x a real-valued input
 * @return return true if \c x is negative, otherwise return false.
 */

template <typename T>
constexpr bool signbit(const T x) noexcept {
#ifdef _MSC_VER
  return ((x == T(0)) ? (_fpclass(x) == _FPCLASS_NZ) : (x < T(0)));
#else
  return GCEM_SIGNBIT(x);
#endif
}

/**
 * Compile-time copy sign function
 *
 * @param x a real-valued input
 * @param y a real-valued input
 * @return replace the signbit of \c x with the signbit of \c y.
 */

template <typename T1, typename T2>
constexpr T1 copysign(const T1 x, const T2 y) noexcept {
  return (signbit(x) != signbit(y) ? -x : x);
}

/*
 * extract signbit for signed zeros
 */

namespace internal {

  template <typename T>
  constexpr bool neg_zero(const T x) noexcept {
    return ((x == T(0.0)) && (copysign(T(1.0), x) == T(-1.0)));
  }

}
/*################################################################################
  ##
  ##   Copyright (C) 2016-2020 Keith O'Hara
  ##
  ##   This file is part of the GCE-Math C++ library.
  ##
  ##   Licensed under the Apache License, Version 2.0 (the "License");
  ##   you may not use this file except in compliance with the License.
  ##   You may obtain a copy of the License at
  ##
  ##       http://www.apache.org/licenses/LICENSE-2.0
  ##
  ##   Unless required by applicable law or agreed to in writing, software
  ##   distributed under the License is distributed on an "AS IS" BASIS,
  ##   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  ##   See the License for the specific language governing permissions and
  ##   limitations under the License.
  ##
  ################################################################################*/

/**
 * Compile-time sign function
 *
 * @param x a real-valued input
 * @return a value \f$ y \f$ such that \f[ y = \begin{cases} 1 \ &\text{ if } x
 * > 0 \\ 0 \ &\text{ if } x = 0 \\ -1 \ &\text{ if } x < 0 \end{cases} \f]
 */

template <typename T>
constexpr int sgn(const T x) noexcept {
  return ( // positive
      x > T(0) ? 1 :
               // negative
          x < T(0) ? -1
                   :
                   // else
          0);
}

/*
 * compile-time find_exponent function
 */

namespace internal {

  template <typename T>
  constexpr llint_t find_exponent(const T x, const llint_t exponent) noexcept {
    return (x < T(1)    ? find_exponent(x * T(10), exponent - llint_t(1))
            : x > T(10) ? find_exponent(x / T(10), exponent + llint_t(1))
                        :
                        // else
            exponent);
  }

}

/*
 * find the fraction part of x = n + r, where -0.5 <= r <= 0.5
 */

namespace internal {

  template <typename T>
  constexpr T find_fraction(const T x) noexcept {
    return (abs(x - internal::floor_check(x)) >= T(0.5) ? // if
            x - internal::floor_check(x) - sgn(x)
                                                        :
                                                        // else
            x - internal::floor_check(x));
  }

}

/*
 * find the whole number part of x = n + r, where -0.5 <= r <= 0.5
 */

namespace internal {

  template <typename T>
  constexpr llint_t find_whole(const T x) noexcept {
    return (abs(x - internal::floor_check(x)) >= T(0.5) ? // if
            static_cast<llint_t>(internal::floor_check(x) + sgn(x))
                                                        :
                                                        // else
            static_cast<llint_t>(internal::floor_check(x)));
  }

}

/*
 * compile-time mantissa function
 */

namespace internal {

  template <typename T>
  constexpr T mantissa(const T x) noexcept {
    return (x < T(1)    ? mantissa(x * T(10))
            : x > T(10) ? mantissa(x / T(10))
                        :
                        // else
            x);
  }

}

namespace internal {

  template <typename T>
  constexpr T round_int(const T x) noexcept {
    return static_cast<T>(find_whole(x));
  }

  template <typename T>
  constexpr T round_check(const T x) noexcept {
    return ( // NaN check
        is_nan(x) ? GCLIM<T>::quiet_NaN() :
                  // +/- infinite
            !is_finite(x) ? x
                          :
                          // signed-zero cases
            GCLIM<T>::epsilon() > abs(x) ? x
                                         :
                                         // else
            sgn(x) * round_int(abs(x)));
  }

}

/**
 * Compile-time round function
 *
 * @param x a real-valued input.
 * @return computes the rounding value of the input.
 */

template <typename T>
constexpr return_t<T> round(const T x) noexcept {
  return internal::round_check(static_cast<return_t<T>>(x));
}

namespace internal {

  template <typename T>
  constexpr T fmod_check(const T x, const T y) noexcept {
    return ( // NaN check
        any_nan(x, y) ? GCLIM<T>::quiet_NaN() :
                      // +/- infinite
            !all_finite(x, y) ? GCLIM<T>::quiet_NaN()
                              :
                              // else
            x - trunc(x / y) * y);
  }

  template <typename T1, typename T2, typename TC = common_return_t<T1, T2>>
  constexpr TC fmod_type_check(const T1 x, const T2 y) noexcept {
    return fmod_check(static_cast<TC>(x), static_cast<TC>(y));
  }

}

/**
 * Compile-time remainder of division function
 * @param x a real-valued input.
 * @param y a real-valued input.
 * @return computes the floating-point remainder of \f$ x / y \f$ (rounded
 * towards zero) using \f[ \text{fmod}(x,y) = x - \text{trunc}(x/y) \times y \f]
 */

template <typename T1, typename T2>
constexpr common_return_t<T1, T2> fmod(const T1 x, const T2 y) noexcept {
  return internal::fmod_type_check(x, y);
}

/*
 * compile-time power function
 */

namespace internal {

  template <typename T1, typename T2>
  constexpr T1 pow_integral_compute(const T1 base, const T2 exp_term) noexcept;

  // integral-valued powers using method described in
  // https://en.wikipedia.org/wiki/Exponentiation_by_squaring

  template <typename T1, typename T2>
  constexpr T1 pow_integral_compute_recur(
      const T1 base, const T1 val, const T2 exp_term) noexcept {
    return (exp_term > T2(1)
            ? (is_odd(exp_term) ? pow_integral_compute_recur(
                                      base * base, val * base, exp_term / 2)
                                : pow_integral_compute_recur(
                                      base * base, val, exp_term / 2))
            : (exp_term == T2(1) ? val * base : val));
  }

  template <typename T1,
      typename T2,
      typename std::enable_if<std::is_signed<T2>::value>::type* = nullptr>
  constexpr T1 pow_integral_sgn_check(
      const T1 base, const T2 exp_term) noexcept {
    return (exp_term < T2(0) ? //
            T1(1) / pow_integral_compute(base, -exp_term)
                             :
                             //
            pow_integral_compute_recur(base, T1(1), exp_term));
  }

  template <typename T1,
      typename T2,
      typename std::enable_if<!std::is_signed<T2>::value>::type* = nullptr>
  constexpr T1 pow_integral_sgn_check(
      const T1 base, const T2 exp_term) noexcept {
    return (pow_integral_compute_recur(base, T1(1), exp_term));
  }

  template <typename T1, typename T2>
  constexpr T1 pow_integral_compute(const T1 base, const T2 exp_term) noexcept {
    return (exp_term == T2(3)   ? base * base * base
            : exp_term == T2(2) ? base * base
            : exp_term == T2(1) ? base
            : exp_term == T2(0) ? T1(1)
                                :
                                // check for overflow
            exp_term == GCLIM<T2>::min()   ? T1(0)
            : exp_term == GCLIM<T2>::max() ? GCLIM<T1>::infinity()
                                           :
                                           // else
            pow_integral_sgn_check(base, exp_term));
  }

  template <typename T1,
      typename T2,
      typename std::enable_if<std::is_integral<T2>::value>::type* = nullptr>
  constexpr T1 pow_integral_type_check(
      const T1 base, const T2 exp_term) noexcept {
    return pow_integral_compute(base, exp_term);
  }

  template <typename T1,
      typename T2,
      typename std::enable_if<!std::is_integral<T2>::value>::type* = nullptr>
  constexpr T1 pow_integral_type_check(
      const T1 base, const T2 exp_term) noexcept {
    // return GCLIM<return_t<T1>>::quiet_NaN();
    return pow_integral_compute(base, static_cast<llint_t>(exp_term));
  }

  //
  // main function

  template <typename T1, typename T2>
  constexpr T1 pow_integral(const T1 base, const T2 exp_term) noexcept {
    return internal::pow_integral_type_check(base, exp_term);
  }

}

/*
 * compile-time exponential function
 */

namespace internal {

  template <typename T>
  constexpr T exp_cf_recur(const T x, const int depth) noexcept {
    return (depth < GCEM_EXP_MAX_ITER_SMALL ? // if
            depth == 1 ? T(1) - x / exp_cf_recur(x, depth + 1)
                       : T(1) + x / T(depth - 1)
                    - x / depth / exp_cf_recur(x, depth + 1)
                                            :
                                            // else
            T(1));
  }

  template <typename T>
  constexpr T exp_cf(const T x) noexcept {
    return (T(1) / exp_cf_recur(x, 1));
  }

  template <typename T>
  constexpr T exp_split(const T x) noexcept {
    return (pow_integral(GCEM_E, find_whole(x)) * exp_cf(find_fraction(x)));
  }

  template <typename T>
  constexpr T exp_check(const T x) noexcept {
    return (is_nan(x) ? GCLIM<T>::quiet_NaN() :
                      //
            is_neginf(x) ? T(0)
                         :
                         //
            GCLIM<T>::epsilon() > abs(x) ? T(1)
                                         :
                                         //
            is_posinf(x) ? GCLIM<T>::infinity()
                         :
                         //
            abs(x) < T(2) ? exp_cf(x)
                          : exp_split(x));
  }

}

/**
 * Compile-time exponential function
 *
 * @param x a real-valued input.
 * @return \f$ \exp(x) \f$ using \f[ \exp(x) =
 * \dfrac{1}{1-\dfrac{x}{1+x-\dfrac{\frac{1}{2}x}{1 + \frac{1}{2}x -
 * \dfrac{\frac{1}{3}x}{1 + \frac{1}{3}x - \ddots}}}} \f] The continued fraction
 * argument is split into two parts: \f$ x = n + r \f$, where \f$ n \f$ is an
 * integer and \f$ r \in [-0.5,0.5] \f$.
 */

template <typename T>
constexpr return_t<T> exp(const T x) noexcept {
  return internal::exp_check(static_cast<return_t<T>>(x));
}

/*
 * compile-time exponential function
 */

namespace internal {

  template <typename T>
  constexpr T expm1_compute(const T x) noexcept {
    // return x * ( T(1) + x * ( T(1)/T(2) + x * ( T(1)/T(6) + x * ( T(1)/T(24)
    // +  x/T(120) ) ) ) ); // O(x^6)
    return x
        + x
        * (x / T(2)
            + x * (x / T(6) + x * (x / T(24) + x * x / T(120)))); // O(x^6)
  }

  template <typename T>
  constexpr T expm1_check(const T x) noexcept {
    return ( // NaN check
        is_nan(x) ? GCLIM<T>::quiet_NaN() :
                  //
            abs(x) > T(1e-04) ? // if
            exp(x) - T(1)
                              :
                              // else
            expm1_compute(x));
  }

}

/**
 * Compile-time exponential-minus-1 function
 *
 * @param x a real-valued input.
 * @return \f$ \exp(x) - 1 \f$ using \f[ \exp(x) = \sum_{k=0}^\infty
 * \dfrac{x^k}{k!} \f]
 */

template <typename T>
constexpr return_t<T> expm1(const T x) noexcept {
  return internal::expm1_check(static_cast<return_t<T>>(x));
}

/*
 * compile-time natural logarithm function
 */

namespace internal {

  // continued fraction seems to be a better approximation for small x
  // see http://functions.wolfram.com/ElementaryFunctions/Log/10/0005/

  template <typename T>
  constexpr T log_cf_main(const T xx, const int depth) noexcept {
    return (depth < GCEM_LOG_MAX_ITER_SMALL ? // if
            T(2 * depth - 1)
                - T(depth * depth) * xx / log_cf_main(xx, depth + 1)
                                            :
                                            // else
            T(2 * depth - 1));
  }

  template <typename T>
  constexpr T log_cf_begin(const T x) noexcept {
    return (T(2) * x / log_cf_main(x * x, 1));
  }

  template <typename T>
  constexpr T log_main(const T x) noexcept {
    return (log_cf_begin((x - T(1)) / (x + T(1))));
  }

  constexpr long double log_mantissa_integer(const int x) noexcept {
    return (x == 2    ? 0.6931471805599453094172321214581765680755L
            : x == 3  ? 1.0986122886681096913952452369225257046475L
            : x == 4  ? 1.3862943611198906188344642429163531361510L
            : x == 5  ? 1.6094379124341003746007593332261876395256L
            : x == 6  ? 1.7917594692280550008124773583807022727230L
            : x == 7  ? 1.9459101490553133051053527434431797296371L
            : x == 8  ? 2.0794415416798359282516963643745297042265L
            : x == 9  ? 2.1972245773362193827904904738450514092950L
            : x == 10 ? 2.3025850929940456840179914546843642076011L
                      : 0.0L);
  }

  template <typename T>
  constexpr T log_mantissa(
      const T x) noexcept { // divide by the integer part of x, which will be in
                            // [1,10], then adjust using tables
    return (log_main(x / T(static_cast<int>(x)))
        + T(log_mantissa_integer(static_cast<int>(x))));
  }

  template <typename T>
  constexpr T log_breakup(const T x) noexcept { // x = a*b, where b = 10^c
    return (
        log_mantissa(mantissa(x)) + T(GCEM_LOG_10) * T(find_exponent(x, 0)));
  }

  template <typename T>
  constexpr T log_check(const T x) noexcept {
    return (is_nan(x) ? GCLIM<T>::quiet_NaN() :
                      // x < 0
            x < T(0) ? GCLIM<T>::quiet_NaN()
                     :
                     // x ~= 0
            GCLIM<T>::epsilon() > x ? -GCLIM<T>::infinity()
                                    :
                                    // indistinguishable from 1
            GCLIM<T>::epsilon() > abs(x - T(1)) ? T(0)
                                                :
                                                //
            x == GCLIM<T>::infinity() ? GCLIM<T>::infinity()
                                      :
                                      // else
            (x < T(0.5) || x > T(1.5)) ?
                                       // if
            log_breakup(x)
                                       :
                                       // else
            log_main(x));
  }

}

/**
 * Compile-time natural logarithm function
 *
 * @param x a real-valued input.
 * @return \f$ \log_e(x) \f$ using \f[ \log\left(\frac{1+x}{1-x}\right) =
 * \dfrac{2x}{1-\dfrac{x^2}{3-\dfrac{4x^2}{5 - \dfrac{9x^3}{7 - \ddots}}}},
 * \ \ x \in [-1,1] \f] The continued fraction argument is split into two parts:
 * \f$ x = a \times 10^c \f$, where \f$ c \f$ is an integer.
 */

template <typename T>
constexpr return_t<T> log(const T x) noexcept {
  return internal::log_check(static_cast<return_t<T>>(x));
}

/*
 * compile-time natural logarithm(x+1) function
 */

namespace internal {

  // see:
  // http://functions.wolfram.com/ElementaryFunctions/Log/06/01/04/01/0003/

  template <typename T>
  constexpr T log1p_compute(const T x) noexcept {
    // return x * ( T(1) + x * ( -T(1)/T(2) +  x * ( T(1)/T(3) +  x * (
    // -T(1)/T(4) + x/T(5) ) ) ) ); // O(x^6)
    return x
        + x
        * (-x / T(2)
            + x * (x / T(3) + x * (-x / T(4) + x * x / T(5)))); // O(x^6)
  }

  template <typename T>
  constexpr T log1p_check(const T x) noexcept {
    return ( // NaN check
        is_nan(x) ? GCLIM<T>::quiet_NaN() :
                  //
            abs(x) > T(1e-04) ? // if
            log(T(1) + x)
                              :
                              // else
            log1p_compute(x));
  }

}

/**
 * Compile-time natural-logarithm-plus-1 function
 *
 * @param x a real-valued input.
 * @return \f$ \log_e(x+1) \f$ using \f[ \log(x+1) = \sum_{k=1}^\infty
 * \dfrac{(-1)^{k-1}x^k}{k}, \ \ |x| < 1 \f]
 */

template <typename T>
constexpr return_t<T> log1p(const T x) noexcept {
  return internal::log1p_check(static_cast<return_t<T>>(x));
}

/*
 * compile-time binary logarithm function
 */

namespace internal {

  template <typename T>
  constexpr T log2_check(const T x) noexcept {
    return (is_nan(x) ? GCLIM<T>::quiet_NaN() :
                      // x < 0
            x < T(0) ? GCLIM<T>::quiet_NaN()
                     :
                     // x ~= 0
            GCLIM<T>::epsilon() > x ? -GCLIM<T>::infinity()
                                    :
                                    // indistinguishable from 1
            GCLIM<T>::epsilon() > abs(x - T(1)) ? T(0)
                                                :
                                                //
            x == GCLIM<T>::infinity() ? GCLIM<T>::infinity()
                                      :
                                      // else: log_2(x) = ln(x) / ln(2)
            T(log(x) / GCEM_LOG_2));
  }

}

/**
 * Compile-time binary logarithm function
 *
 * @param x a real-valued input.
 * @return \f$ \log_2(x) \f$ using \f[ \log_{2}(x) = \frac{\log_e(x)}{\log_e(2)}
 * \f]
 */

template <typename T>
constexpr return_t<T> log2(const T x) noexcept {
  return internal::log2_check(static_cast<return_t<T>>(x));
}

namespace internal {

  template <typename T>
  constexpr return_t<T> log10_check(const T x) noexcept {
    // log_10(x) = ln(x) / ln(10)
    return return_t<T>(log(x) / GCEM_LOG_10);
  }

}

/**
 * Compile-time common logarithm function
 *
 * @param x a real-valued input.
 * @return \f$ \log_{10}(x) \f$ using \f[ \log_{10}(x) =
 * \frac{\log_e(x)}{\log_e(10)} \f]
 */

template <typename T>
constexpr return_t<T> log10(const T x) noexcept {
  return internal::log10_check(x);
}

/*
 * compile-time power function
 */

namespace internal {

  template <typename T>
  constexpr T pow_dbl(const T base, const T exp_term) noexcept {
    return exp(exp_term * log(base));
  }

  template <typename T1,
      typename T2,
      typename TC = common_t<T1, T2>,
      typename std::enable_if<!std::is_integral<T2>::value>::type* = nullptr>
  constexpr TC pow_check(const T1 base, const T2 exp_term) noexcept {
    return (base < T1(0) ? GCLIM<TC>::quiet_NaN() :
                         //
            pow_dbl(static_cast<TC>(base), static_cast<TC>(exp_term)));
  }

  template <typename T1,
      typename T2,
      typename TC = common_t<T1, T2>,
      typename std::enable_if<std::is_integral<T2>::value>::type* = nullptr>
  constexpr TC pow_check(const T1 base, const T2 exp_term) noexcept {
    return pow_integral(base, exp_term);
  }

}

/**
 * Compile-time power function
 *
 * @param base a real-valued input.
 * @param exp_term a real-valued input.
 * @return Computes \c base raised to the power \c exp_term. In the case where
 * \c exp_term is integral-valued, recursion by squaring is used, otherwise \f$
 * \text{base}^{\text{exp\_term}} = e^{\text{exp\_term} \log(\text{base})} \f$
 */

template <typename T1, typename T2>
constexpr common_t<T1, T2> pow(const T1 base, const T2 exp_term) noexcept {
  return internal::pow_check(base, exp_term);
}

namespace internal {

  template <typename T>
  constexpr T gcd_recur(const T a, const T b) noexcept {
    return (b == T(0) ? a : gcd_recur(b, a % b));
  }

  template <typename T,
      typename std::enable_if<std::is_integral<T>::value>::type* = nullptr>
  constexpr T gcd_int_check(const T a, const T b) noexcept {
    return gcd_recur(a, b);
  }

  template <typename T,
      typename std::enable_if<!std::is_integral<T>::value>::type* = nullptr>
  constexpr T gcd_int_check(const T a, const T b) noexcept {
    return gcd_recur(static_cast<ullint_t>(a), static_cast<ullint_t>(b));
  }

  template <typename T1, typename T2, typename TC = common_t<T1, T2>>
  constexpr TC gcd_type_check(const T1 a, const T2 b) noexcept {
    return gcd_int_check(static_cast<TC>(abs(a)), static_cast<TC>(abs(b)));
  }

}

/**
 * Compile-time greatest common divisor (GCD) function
 *
 * @param a integral-valued input.
 * @param b integral-valued input.
 * @return the greatest common divisor between integers \c a and \c b using a
 * Euclidean algorithm.
 */

template <typename T1, typename T2>
constexpr common_t<T1, T2> gcd(const T1 a, const T2 b) noexcept {
  return internal::gcd_type_check(a, b);
}

namespace internal {

  template <typename T>
  constexpr T lcm_compute(const T a, const T b) noexcept {
    return abs(a * (b / gcd(a, b)));
  }

  template <typename T1, typename T2, typename TC = common_t<T1, T2>>
  constexpr TC lcm_type_check(const T1 a, const T2 b) noexcept {
    return lcm_compute(static_cast<TC>(a), static_cast<TC>(b));
  }

}

/**
 * Compile-time least common multiple (LCM) function
 *
 * @param a integral-valued input.
 * @param b integral-valued input.
 * @return the least common multiple between integers \c a and \c b using the
 * representation \f[ \text{lcm}(a,b) = \dfrac{| a b |}{\text{gcd}(a,b)} \f]
 * where \f$ \text{gcd}(a,b) \f$ denotes the greatest common divisor between \f$
 * a \f$ and \f$ b \f$.
 */

template <typename T1, typename T2>
constexpr common_t<T1, T2> lcm(const T1 a, const T2 b) noexcept {
  return internal::lcm_type_check(a, b);
}

/*
 * compile-time tangent function
 */

namespace internal {

  template <typename T>
  constexpr T tan_series_exp_long(
      const T z) noexcept { // this is based on a fourth-order expansion of
                            // tan(z) using Bernoulli numbers
    return (-1 / z
        + (z / 3
            + (pow_integral(z, 3) / 45
                + (2 * pow_integral(z, 5) / 945 + pow_integral(z, 7) / 4725))));
  }

  template <typename T>
  constexpr T tan_series_exp(const T x) noexcept {
    return (GCLIM<T>::epsilon() > abs(x - T(GCEM_HALF_PI))
            ? // the value tan(pi/2) is somewhat of a convention;
              // technically the function is not defined at EXACTLY pi/2,
              // but this is floating point pi/2
            T(1.633124e+16)
            :
            // otherwise we use an expansion around pi/2
            tan_series_exp_long(x - T(GCEM_HALF_PI)));
  }

  template <typename T>
  constexpr T tan_cf_recur(
      const T xx, const int depth, const int max_depth) noexcept {
    return (depth < max_depth ? // if
            T(2 * depth - 1) - xx / tan_cf_recur(xx, depth + 1, max_depth)
                              :
                              // else
            T(2 * depth - 1));
  }

  template <typename T>
  constexpr T tan_cf_main(const T x) noexcept {
    return ((x > T(1.55) && x < T(1.60))
            ? tan_series_exp(x)
            : // deals with a singularity at tan(pi/2)
              //
            x > T(1.4) ? x / tan_cf_recur(x * x, 1, 45)
            : x > T(1) ? x / tan_cf_recur(x * x, 1, 35)
                       :
                       // else
            x / tan_cf_recur(x * x, 1, 25));
  }

  template <typename T>
  constexpr T tan_begin(
      const T x, const int count = 0) noexcept { // tan(x) = tan(x + pi)
    return (x > T(GCEM_PI) ?                     // if
            count > 1 ? GCLIM<T>::quiet_NaN()
                      : // protect against undefined behavior
                tan_begin(
                    x - T(GCEM_PI) * internal::floor_check(x / T(GCEM_PI)),
                    count + 1)
                           :
                           // else
            tan_cf_main(x));
  }

  template <typename T>
  constexpr T tan_check(const T x) noexcept {
    return ( // NaN check
        is_nan(x) ? GCLIM<T>::quiet_NaN() :
                  // indistinguishable from zero
            GCLIM<T>::epsilon() > abs(x) ? T(0)
                                         :
                                         // else
            x < T(0) ? -tan_begin(-x)
                     : tan_begin(x));
  }

}

/**
 * Compile-time tangent function
 *
 * @param x a real-valued input.
 * @return the tangent function using
 * \f[ \tan(x) = \dfrac{x}{1 - \dfrac{x^2}{3 - \dfrac{x^2}{5 - \ddots}}} \f]
 * To deal with a singularity at \f$ \pi / 2 \f$, the following expansion is
 * employed:
 * \f[ \tan(x) = - \frac{1}{x-\pi/2} - \sum_{k=1}^\infty \frac{(-1)^k 2^{2k}
 * B_{2k}}{(2k)!} (x - \pi/2)^{2k - 1} \f] where \f$ B_n \f$ is the n-th
 * Bernoulli number.
 */

template <typename T>
constexpr return_t<T> tan(const T x) noexcept {
  return internal::tan_check(static_cast<return_t<T>>(x));
}

/*
 * compile-time cosine function using tan(x/2)
 */

namespace internal {

  template <typename T>
  constexpr T cos_compute(const T x) noexcept {
    return (T(1) - x * x) / (T(1) + x * x);
  }

  template <typename T>
  constexpr T cos_check(const T x) noexcept {
    return ( // NaN check
        is_nan(x) ? GCLIM<T>::quiet_NaN() :
                  // indistinguishable from 0
            GCLIM<T>::epsilon() > abs(x) ? T(1)
                                         :
                                         // special cases: pi/2 and pi
            GCLIM<T>::epsilon() > abs(x - T(GCEM_HALF_PI))   ? T(0)
            : GCLIM<T>::epsilon() > abs(x + T(GCEM_HALF_PI)) ? T(0)
            : GCLIM<T>::epsilon() > abs(x - T(GCEM_PI))      ? -T(1)
            : GCLIM<T>::epsilon() > abs(x + T(GCEM_PI))      ? -T(1)
                                                             :
                                                             // else
            cos_compute(tan(x / T(2))));
  }

}

/**
 * Compile-time cosine function
 *
 * @param x a real-valued input.
 * @return the cosine function using \f[ \cos(x) =
 * \frac{1-\tan^2(x/2)}{1+\tan^2(x/2)} \f]
 */

template <typename T>
constexpr return_t<T> cos(const T x) noexcept {
  return internal::cos_check(static_cast<return_t<T>>(x));
}

/*
 * compile-time sine function using tan(x/2)
 *
 * see eq. 5.4.8 in Numerical Recipes
 */

namespace internal {

  template <typename T>
  constexpr T sin_compute(const T x) noexcept {
    return T(2) * x / (T(1) + x * x);
  }

  template <typename T>
  constexpr T sin_check(const T x) noexcept {
    return ( // NaN check
        is_nan(x) ? GCLIM<T>::quiet_NaN() :
                  // indistinguishable from zero
            GCLIM<T>::epsilon() > abs(x) ? T(0)
                                         :
                                         // special cases: pi/2 and pi
            GCLIM<T>::epsilon() > abs(x - T(GCEM_HALF_PI))   ? T(1)
            : GCLIM<T>::epsilon() > abs(x + T(GCEM_HALF_PI)) ? -T(1)
            : GCLIM<T>::epsilon() > abs(x - T(GCEM_PI))      ? T(0)
            : GCLIM<T>::epsilon() > abs(x + T(GCEM_PI))      ? -T(0)
                                                             :
                                                             // else
            sin_compute(tan(x / T(2))));
  }

}

/**
 * Compile-time sine function
 *
 * @param x a real-valued input.
 * @return the sine function using \f[ \sin(x) =
 * \frac{2\tan(x/2)}{1+\tan^2(x/2)} \f]
 */

template <typename T>
constexpr return_t<T> sin(const T x) noexcept {
  return internal::sin_check(static_cast<return_t<T>>(x));
}

/*
 * compile-time arctangent function
 */

// see
// http://functions.wolfram.com/ElementaryFunctions/ArcTan/10/0001/
// http://functions.wolfram.com/ElementaryFunctions/ArcTan/06/01/06/01/0002/

namespace internal {

  // Series

  template <typename T>
  constexpr T atan_series_order_calc(
      const T x, const T x_pow, const uint_t order) noexcept {
    return (T(1) / (T((order - 1) * 4 - 1) * x_pow)
        - T(1) / (T((order - 1) * 4 + 1) * x_pow * x));
  }

  template <typename T>
  constexpr T atan_series_order(const T x,
      const T x_pow,
      const uint_t order,
      const uint_t max_order) noexcept {
    return (order == 1 ? GCEM_HALF_PI - T(1) / x
                + atan_series_order(x * x, pow(x, 3), order + 1, max_order)
                       :
                       // NOTE: x changes to x*x for order > 1
            order < max_order ? atan_series_order_calc(x, x_pow, order)
                + atan_series_order(x, x_pow * x * x, order + 1, max_order)
                              :
                              // order == max_order
            atan_series_order_calc(x, x_pow, order));
  }

  template <typename T>
  constexpr T atan_series_main(const T x) noexcept {
    return (x < T(3) ? atan_series_order(x, x, 1U, 10U) : // O(1/x^39)
            x < T(4) ? atan_series_order(x, x, 1U, 9U)
                     : // O(1/x^35)
            x < T(5) ? atan_series_order(x, x, 1U, 8U)
                     : // O(1/x^31)
            x < T(7) ? atan_series_order(x, x, 1U, 7U)
                     : // O(1/x^27)
            x < T(11) ? atan_series_order(x, x, 1U, 6U)
                      : // O(1/x^23)
            x < T(25) ? atan_series_order(x, x, 1U, 5U)
                      : // O(1/x^19)
            x < T(100) ? atan_series_order(x, x, 1U, 4U)
                       : // O(1/x^15)
            x < T(1000) ? atan_series_order(x, x, 1U, 3U)
                        :                     // O(1/x^11)
            atan_series_order(x, x, 1U, 2U)); // O(1/x^7)
  }

  // CF

  template <typename T>
  constexpr T atan_cf_recur(
      const T xx, const uint_t depth, const uint_t max_depth) noexcept {
    return (depth < max_depth ? // if
            T(2 * depth - 1)
                + depth * depth * xx / atan_cf_recur(xx, depth + 1, max_depth)
                              :
                              // else
            T(2 * depth - 1));
  }

  template <typename T>
  constexpr T atan_cf_main(const T x) noexcept {
    return (x < T(0.5)   ? x / atan_cf_recur(x * x, 1U, 15U)
            : x < T(1)   ? x / atan_cf_recur(x * x, 1U, 25U)
            : x < T(1.5) ? x / atan_cf_recur(x * x, 1U, 35U)
            : x < T(2)   ? x / atan_cf_recur(x * x, 1U, 45U)
                         : x / atan_cf_recur(x * x, 1U, 52U));
  }

  //

  template <typename T>
  constexpr T atan_begin(const T x) noexcept {
    return (x > T(2.5) ? atan_series_main(x) : atan_cf_main(x));
  }

  template <typename T>
  constexpr T atan_check(const T x) noexcept {
    return ( // NaN check
        is_nan(x) ? GCLIM<T>::quiet_NaN() :
                  // indistinguishable from zero
            GCLIM<T>::epsilon() > abs(x) ? T(0)
                                         :
                                         // negative or positive
            x < T(0) ? -atan_begin(-x)
                     : atan_begin(x));
  }

}

/**
 * Compile-time arctangent function
 *
 * @param x a real-valued input.
 * @return the inverse tangent function using \f[ \text{atan}(x) = \dfrac{x}{1 +
 * \dfrac{x^2}{3 + \dfrac{4x^2}{5 + \dfrac{9x^2}{7 + \ddots}}}} \f]
 */

template <typename T>
constexpr return_t<T> atan(const T x) noexcept {
  return internal::atan_check(static_cast<return_t<T>>(x));
}

/*
 * compile-time two-argument arctangent function
 */

namespace internal {

  template <typename T>
  constexpr T atan2_compute(const T y, const T x) noexcept {
    return ( // NaN check
        any_nan(y, x) ? GCLIM<T>::quiet_NaN() :
                      //
            GCLIM<T>::epsilon() > abs(x) ? //
            GCLIM<T>::epsilon() > abs(y) ? neg_zero(y)
                    ? neg_zero(x) ? -T(GCEM_PI) : -T(0)
                    : neg_zero(x) ? T(GCEM_PI)
                                  : T(0)
                : y > T(0)               ? T(GCEM_HALF_PI)
                                         : -T(GCEM_HALF_PI)
                                         :
                                         //
            x < T(0)
                ? y < T(0) ? atan(y / x) - T(GCEM_PI) : atan(y / x) + T(GCEM_PI)
                :
                //
                atan(y / x));
  }

  template <typename T1, typename T2, typename TC = common_return_t<T1, T2>>
  constexpr TC atan2_type_check(const T1 y, const T2 x) noexcept {
    return atan2_compute(static_cast<TC>(x), static_cast<TC>(y));
  }

}

/**
 * Compile-time two-argument arctangent function
 *
 * @param y a real-valued input.
 * @param x a real-valued input.
 * @return \f[ \text{atan2}(y,x) = \begin{cases} \text{atan}(y/x) & \text{ if }
 * x > 0 \\ \text{atan}(y/x) + \pi & \text{ if } x < 0 \text{ and } y \geq 0
 * \\ \text{atan}(y/x) - \pi & \text{ if } x < 0 \text{ and } y < 0 \\ + \pi/2 &
 * \text{ if } x = 0 \text{ and } y > 0 \\ - \pi/2 & \text{ if } x = 0 \text{
 * and } y < 0 \end{cases} \f] The function is undefined at the origin, however
 * the following conventions are used.
 * \f[ \text{atan2}(y,x) = \begin{cases} +0 & \text{ if } x = +0 \text{ and } y
 * = +0 \\ -0 & \text{ if } x = +0 \text{ and } y = -0 \\ +\pi & \text{ if } x =
 * -0 \text{ and } y = +0 \\ - \pi & \text{ if } x = -0 \text{ and } y = -0
 * \end{cases} \f]
 */

template <typename T1, typename T2>
constexpr common_return_t<T1, T2> atan2(const T1 y, const T2 x) noexcept {
  return internal::atan2_type_check(x, y);
}

/*
 * compile-time arccosine function
 */

namespace internal {

  template <typename T>
  constexpr T acos_compute(const T x) noexcept {
    return ( // only defined on [-1,1]
        abs(x) > T(1) ? GCLIM<T>::quiet_NaN() :
                      // indistinguishable from one or zero
            GCLIM<T>::epsilon() > abs(x - T(1)) ? T(0)
            : GCLIM<T>::epsilon() > abs(x)      ? T(GCEM_HALF_PI)
                                                :
                                                // else
            atan(sqrt(T(1) - x * x) / x));
  }

  template <typename T>
  constexpr T acos_check(const T x) noexcept {
    return ( // NaN check
        is_nan(x) ? GCLIM<T>::quiet_NaN() :
                  //
            x > T(0) ? // if
            acos_compute(x)
                     :
                     // else
            T(GCEM_PI) - acos_compute(-x));
  }

}

/**
 * Compile-time arccosine function
 *
 * @param x a real-valued input, where \f$ x \in [-1,1] \f$.
 * @return the inverse cosine function using \f[ \text{acos}(x) = \text{atan}
 * \left( \frac{\sqrt{1-x^2}}{x} \right) \f]
 */

template <typename T>
constexpr return_t<T> acos(const T x) noexcept {
  return internal::acos_check(static_cast<return_t<T>>(x));
}

/*
 * compile-time arcsine function
 */

namespace internal {

  template <typename T>
  constexpr T asin_compute(const T x) noexcept {
    return ( // only defined on [-1,1]
        x > T(1) ? GCLIM<T>::quiet_NaN() :
                 // indistinguishable from one or zero
            GCLIM<T>::epsilon() > abs(x - T(1)) ? T(GCEM_HALF_PI)
            : GCLIM<T>::epsilon() > abs(x)      ? T(0)
                                                :
                                                // else
            atan(x / sqrt(T(1) - x * x)));
  }

  template <typename T>
  constexpr T asin_check(const T x) noexcept {
    return ( // NaN check
        is_nan(x) ? GCLIM<T>::quiet_NaN() :
                  //
            x < T(0) ? -asin_compute(-x)
                     : asin_compute(x));
  }

}

/**
 * Compile-time arcsine function
 *
 * @param x a real-valued input, where \f$ x \in [-1,1] \f$.
 * @return the inverse sine function using \f[ \text{asin}(x) = \text{atan}
 * \left( \frac{x}{\sqrt{1-x^2}} \right) \f]
 */

template <typename T>
constexpr return_t<T> asin(const T x) noexcept {
  return internal::asin_check(static_cast<return_t<T>>(x));
}

/*
 * compile-time hyperbolic tangent function
 */

namespace internal {

  template <typename T>
  constexpr T tanh_cf(const T xx, const int depth) noexcept {
    return (depth < GCEM_TANH_MAX_ITER ? // if
            (2 * depth - 1) + xx / tanh_cf(xx, depth + 1)
                                       :
                                       // else
            T(2 * depth - 1));
  }

  template <typename T>
  constexpr T tanh_begin(const T x) noexcept {
    return (x / tanh_cf(x * x, 1));
  }

  template <typename T>
  constexpr T tanh_check(const T x) noexcept {
    return ( // NaN check
        is_nan(x) ? GCLIM<T>::quiet_NaN() :
                  // indistinguishable from zero
            GCLIM<T>::epsilon() > abs(x) ? T(0)
                                         :
                                         // else
            x < T(0) ? -tanh_begin(-x)
                     : tanh_begin(x));
  }

}

/**
 * Compile-time hyperbolic tangent function
 *
 * @param x a real-valued input.
 * @return the hyperbolic tangent function using \f[ \tanh(x) = \dfrac{x}{1 +
 * \dfrac{x^2}{3 + \dfrac{x^2}{5 + \dfrac{x^2}{7 + \ddots}}}} \f]
 */

template <typename T>
constexpr return_t<T> tanh(const T x) noexcept {
  return internal::tanh_check(static_cast<return_t<T>>(x));
}

/*
 * compile-time hyperbolic cosine function
 */

namespace internal {

  template <typename T>
  constexpr T cosh_compute(const T x) noexcept {
    return ( // NaN check
        is_nan(x) ? GCLIM<T>::quiet_NaN() :
                  // indistinguishable from zero
            GCLIM<T>::epsilon() > abs(x) ? T(1)
                                         :
                                         // else
            (exp(x) + exp(-x)) / T(2));
  }

}

/**
 * Compile-time hyperbolic cosine function
 *
 * @param x a real-valued input.
 * @return the hyperbolic cosine function using \f[ \cosh(x) = \frac{\exp(x) +
 * \exp(-x)}{2} \f]
 */

template <typename T>
constexpr return_t<T> cosh(const T x) noexcept {
  return internal::cosh_compute(static_cast<return_t<T>>(x));
}

/*
 * compile-time hyperbolic sine function
 */

namespace internal {

  template <typename T>
  constexpr T sinh_check(const T x) noexcept {
    return ( // NaN check
        is_nan(x) ? GCLIM<T>::quiet_NaN() :
                  // indistinguishable from zero
            GCLIM<T>::epsilon() > abs(x) ? T(0)
                                         :
                                         // else
            (exp(x) - exp(-x)) / T(2));
  }

}

/**
 * Compile-time hyperbolic sine function
 *
 * @param x a real-valued input.
 * @return the hyperbolic sine function using \f[ \sinh(x) = \frac{\exp(x) -
 * \exp(-x)}{2} \f]
 */

template <typename T>
constexpr return_t<T> sinh(const T x) noexcept {
  return internal::sinh_check(static_cast<return_t<T>>(x));
}

/*
 * compile-time inverse hyperbolic tangent function
 */

namespace internal {

  template <typename T>
  constexpr T atanh_compute(const T x) noexcept {
    return (log((T(1) + x) / (T(1) - x)) / T(2));
  }

  template <typename T>
  constexpr T atanh_check(const T x) noexcept {
    return ( // NaN check
        is_nan(x) ? GCLIM<T>::quiet_NaN() :
                  // function is defined for |x| < 1
            T(1) < abs(x) ? GCLIM<T>::quiet_NaN()
            : GCLIM<T>::epsilon() > (T(1) - abs(x))
            ? sgn(x) * GCLIM<T>::infinity()
            :
            // indistinguishable from zero
            GCLIM<T>::epsilon() > abs(x) ? T(0)
                                         :
                                         // else
            atanh_compute(x));
  }

}

/**
 * Compile-time inverse hyperbolic tangent function
 *
 * @param x a real-valued input.
 * @return the inverse hyperbolic tangent function using \f[ \text{atanh}(x) =
 * \frac{1}{2} \ln \left( \frac{1+x}{1-x} \right) \f]
 */

template <typename T>
constexpr return_t<T> atanh(const T x) noexcept {
  return internal::atanh_check(static_cast<return_t<T>>(x));
}

/*
 * compile-time inverse hyperbolic cosine function
 */

namespace internal {

  template <typename T>
  constexpr T acosh_compute(const T x) noexcept {
    return ( // NaN check
        is_nan(x) ? GCLIM<T>::quiet_NaN() :
                  // function defined for x >= 1
            x < T(1) ? GCLIM<T>::quiet_NaN()
                     :
                     // indistinguishable from 1
            GCLIM<T>::epsilon() > abs(x - T(1)) ? T(0)
                                                :
                                                // else
            log(x + sqrt(x * x - T(1))));
  }

}

/**
 * Compile-time inverse hyperbolic cosine function
 *
 * @param x a real-valued input.
 * @return the inverse hyperbolic cosine function using \f[ \text{acosh}(x) =
 * \ln \left( x + \sqrt{x^2 - 1} \right) \f]
 */

template <typename T>
constexpr return_t<T> acosh(const T x) noexcept {
  return internal::acosh_compute(static_cast<return_t<T>>(x));
}

/*
 * compile-time inverse hyperbolic sine function
 */

namespace internal {

  template <typename T>
  constexpr T asinh_compute(const T x) noexcept {
    return ( // NaN check
        is_nan(x) ? GCLIM<T>::quiet_NaN() :
                  // indistinguishable from zero
            GCLIM<T>::epsilon() > abs(x) ? T(0)
                                         :
                                         // else
            log(x + sqrt(x * x + T(1))));
  }

}

/**
 * Compile-time inverse hyperbolic sine function
 *
 * @param x a real-valued input.
 * @return the inverse hyperbolic sine function using \f[ \text{asinh}(x) = \ln
 * \left( x + \sqrt{x^2 + 1} \right) \f]
 */

template <typename T>
constexpr return_t<T> asinh(const T x) noexcept {
  return internal::asinh_compute(static_cast<return_t<T>>(x));
}

namespace internal {

  template <typename T>
  constexpr T binomial_coef_recur(const T n, const T k) noexcept {
    return (                           // edge cases
        (k == T(0) || n == k) ? T(1) : // deals with 0 choose 0 case
            n == T(0) ? T(0)
                      :
                      // else
            binomial_coef_recur(n - 1, k - 1) + binomial_coef_recur(n - 1, k));
  }

  template <typename T,
      typename std::enable_if<std::is_integral<T>::value>::type* = nullptr>
  constexpr T binomial_coef_check(const T n, const T k) noexcept {
    return binomial_coef_recur(n, k);
  }

  template <typename T,
      typename std::enable_if<!std::is_integral<T>::value>::type* = nullptr>
  constexpr T binomial_coef_check(const T n, const T k) noexcept {
    return ( // NaN check; removed due to MSVC problems; template not being
             // ignored in <int> cases (is_nan(n) || is_nan(k)) ?
             // GCLIM<T>::quiet_NaN() :
             //
        static_cast<T>(binomial_coef_recur(
            static_cast<ullint_t>(n), static_cast<ullint_t>(k))));
  }

  template <typename T1, typename T2, typename TC = common_t<T1, T2>>
  constexpr TC binomial_coef_type_check(const T1 n, const T2 k) noexcept {
    return binomial_coef_check(static_cast<TC>(n), static_cast<TC>(k));
  }

}

/**
 * Compile-time binomial coefficient
 *
 * @param n integral-valued input.
 * @param k integral-valued input.
 * @return computes the Binomial coefficient
 * \f[ \binom{n}{k} = \frac{n!}{k!(n-k)!} \f]
 * also known as '\c n choose \c k '.
 */

template <typename T1, typename T2>
constexpr common_t<T1, T2> binomial_coef(const T1 n, const T2 k) noexcept {
  return internal::binomial_coef_type_check(n, k);
}

/*
 * compile-time log-gamma function
 *
 * for coefficient values, see:
 * http://my.fit.edu/~gabdo/gamma.txt
 */

namespace internal {

  // P. Godfrey's coefficients:
  //
  //  0.99999999999999709182
  //  57.156235665862923517
  // -59.597960355475491248
  //  14.136097974741747174
  //  -0.49191381609762019978
  //    .33994649984811888699e-4
  //    .46523628927048575665e-4
  //   -.98374475304879564677e-4
  //    .15808870322491248884e-3
  //   -.21026444172410488319e-3
  //    .21743961811521264320e-3
  //   -.16431810653676389022e-3
  //    .84418223983852743293e-4
  //   -.26190838401581408670e-4
  //    .36899182659531622704e-5

  constexpr long double lgamma_coef_term(const long double x) noexcept {
    return (0.99999999999999709182L + 57.156235665862923517L / (x + 1)
        - 59.597960355475491248L / (x + 2) + 14.136097974741747174L / (x + 3)
        - 0.49191381609762019978L / (x + 4)
        + .33994649984811888699e-4L / (x + 5)
        + .46523628927048575665e-4L / (x + 6)
        - .98374475304879564677e-4L / (x + 7)
        + .15808870322491248884e-3L / (x + 8)
        - .21026444172410488319e-3L / (x + 9)
        + .21743961811521264320e-3L / (x + 10)
        - .16431810653676389022e-3L / (x + 11)
        + .84418223983852743293e-4L / (x + 12)
        - .26190838401581408670e-4L / (x + 13)
        + .36899182659531622704e-5L / (x + 14));
  }

  template <typename T>
  constexpr T lgamma_term_2(const T x) noexcept { //
    return (T(GCEM_LOG_SQRT_2PI) + log(T(lgamma_coef_term(x))));
  }

  template <typename T>
  constexpr T lgamma_term_1(
      const T x) noexcept { // note: 607/128 + 0.5 = 5.2421875
    return ((x + T(0.5)) * log(x + T(5.2421875L)) - (x + T(5.2421875L)));
  }

  template <typename T>
  constexpr T lgamma_begin(const T x) noexcept { // returns lngamma(x+1)
    return (lgamma_term_1(x) + lgamma_term_2(x));
  }

  template <typename T>
  constexpr T lgamma_check(const T x) noexcept {
    return ( // NaN check
        is_nan(x) ? GCLIM<T>::quiet_NaN() :
                  // indistinguishable from one or <= zero
            GCLIM<T>::epsilon() > abs(x - T(1)) ? T(0)
            : GCLIM<T>::epsilon() > x           ? GCLIM<T>::infinity()
                                                :
                                                // else
            lgamma_begin(x - T(1)));
  }

}

/**
 * Compile-time log-gamma function
 *
 * @param x a real-valued input.
 * @return computes the log-gamma function
 * \f[ \ln \Gamma(x) = \ln \int_0^\infty y^{x-1} \exp(-y) dy \f]
 * using a polynomial form:
 * \f[ \Gamma(x+1) \approx (x+g+0.5)^{x+0.5} \exp(-x-g-0.5) \sqrt{2 \pi}
 * \left[ c_0 + \frac{c_1}{x+1} + \frac{c_2}{x+2} + \cdots + \frac{c_n}{x+n}
 * \right]
 * \f] where the value \f$ g \f$ and the coefficients \f$ (c_0, c_1, \ldots,
 * c_n) \f$ are taken from Paul Godfrey, whose note can be found here:
 * http://my.fit.edu/~gabdo/gamma.txt
 */

template <typename T>
constexpr return_t<T> lgamma(const T x) noexcept {
  return internal::lgamma_check(static_cast<return_t<T>>(x));
}

/*
 * the ('true') gamma function
 */

namespace internal {

  template <typename T>
  constexpr T tgamma_check(const T x) noexcept {
    return ( // NaN check
        is_nan(x) ? GCLIM<T>::quiet_NaN() :
                  // indistinguishable from one or zero
            GCLIM<T>::epsilon() > abs(x - T(1)) ? T(1)
            : GCLIM<T>::epsilon() > abs(x)      ? GCLIM<T>::infinity()
                                                :
                                                // negative numbers
            x < T(0) ? // check for integer
            GCLIM<T>::epsilon() > abs(x - find_whole(x)) ? GCLIM<T>::quiet_NaN()
                                                         :
                                                         // else
                tgamma_check(x + T(1)) / x
                     :

                     // else
            exp(lgamma(x)));
  }

}

/**
 * Compile-time gamma function
 *
 * @param x a real-valued input.
 * @return computes the `true' gamma function
 * \f[ \Gamma(x) = \int_0^\infty y^{x-1} \exp(-y) dy \f]
 * using a polynomial form:
 * \f[ \Gamma(x+1) \approx (x+g+0.5)^{x+0.5} \exp(-x-g-0.5) \sqrt{2 \pi}
 * \left[ c_0 + \frac{c_1}{x+1} + \frac{c_2}{x+2} + \cdots + \frac{c_n}{x+n}
 * \right]
 * \f] where the value \f$ g \f$ and the coefficients \f$ (c_0, c_1, \ldots,
 * c_n) \f$ are taken from Paul Godfrey, whose note can be found here:
 * http://my.fit.edu/~gabdo/gamma.txt
 */

template <typename T>
constexpr return_t<T> tgamma(const T x) noexcept {
  return internal::tgamma_check(static_cast<return_t<T>>(x));
}

/*
 * compile-time factorial function
 */

namespace internal {

  // T should be int, long int, unsigned int, etc.

  template <typename T>
  constexpr T factorial_table(
      const T x) noexcept { // table for x! when x = {2,...,16}
    return (x == T(2)   ? T(2)
            : x == T(3) ? T(6)
            : x == T(4) ? T(24)
            : x == T(5) ? T(120)
            : x == T(6) ? T(720)
            : x == T(7) ? T(5040)
            : x == T(8) ? T(40320)
            : x == T(9) ? T(362880)
                        :
                        //
            x == T(10)   ? T(3628800)
            : x == T(11) ? T(39916800)
            : x == T(12) ? T(479001600)
            : x == T(13) ? T(6227020800)
            : x == T(14) ? T(87178291200)
            : x == T(15) ? T(1307674368000)
                         : T(20922789888000));
  }

  template <typename T,
      typename std::enable_if<std::is_integral<T>::value>::type* = nullptr>
  constexpr T factorial_recur(const T x) noexcept {
    return (x == T(0)   ? T(1)
            : x == T(1) ? x
                        :
                        //
            x < T(17) ? // if
            factorial_table(x)
                      :
                      // else
            x * factorial_recur(x - 1));
  }

  template <typename T,
      typename std::enable_if<!std::is_integral<T>::value>::type* = nullptr>
  constexpr T factorial_recur(const T x) noexcept {
    return tgamma(x + 1);
  }

}

/**
 * Compile-time factorial function
 *
 * @param x a real-valued input.
 * @return Computes the factorial value \f$ x! \f$.
 * When \c x is an integral type (\c int, <tt>long int</tt>, etc.), a simple
 * recursion method is used, along with table values. When \c x is
 * real-valued, <tt>factorial(x) = tgamma(x+1)</tt>.
 */

template <typename T>
constexpr T factorial(const T x) noexcept {
  return internal::factorial_recur(x);
}

/**
 * Compile-time log-beta function
 *
 * @param a a real-valued input.
 * @param b a real-valued input.
 * @return the log-beta function using \f[ \ln \text{B}(\alpha,\beta) := \ln
 * \int_0^1 t^{\alpha - 1} (1-t)^{\beta - 1} dt = \ln \Gamma(\alpha) + \ln
 * \Gamma(\beta) - \ln \Gamma(\alpha + \beta) \f] where \f$ \Gamma \f$ denotes
 * the gamma function.
 */

template <typename T1, typename T2>
constexpr common_return_t<T1, T2> lbeta(const T1 a, const T2 b) noexcept {
  return ((lgamma(a) + lgamma(b)) - lgamma(a + b));
}

/**
 * Compile-time beta function
 *
 * @param a a real-valued input.
 * @param b a real-valued input.
 * @return the beta function using \f[ \text{B}(\alpha,\beta) := \int_0^1
 * t^{\alpha - 1} (1-t)^{\beta - 1} dt =
 * \frac{\Gamma(\alpha)\Gamma(\beta)}{\Gamma(\alpha + \beta)} \f] where \f$
 * \Gamma \f$ denotes the gamma function.
 */

template <typename T1, typename T2>
constexpr common_return_t<T1, T2> beta(const T1 a, const T2 b) noexcept {
  return exp(lbeta(a, b));
}

/*
 * log multivariate gamma function
 */

namespace internal {

  // see https://en.wikipedia.org/wiki/Multivariate_gamma_function

  template <typename T1, typename T2>
  constexpr T1 lmgamma_recur(const T1 a, const T2 p) noexcept {
    return ( // NaN check
        is_nan(a) ? GCLIM<T1>::quiet_NaN() :
                  //
            p == T2(1)  ? lgamma(a)
            : p < T2(1) ? GCLIM<T1>::quiet_NaN()
                        :
                        // else
            T1(GCEM_LOG_PI) * (p - T1(1)) / T1(2) + lgamma(a)
                + lmgamma_recur(a - T1(0.5), p - T2(1)));
  }

}

/**
 * Compile-time log multivariate gamma function
 *
 * @param a a real-valued input.
 * @param p integral-valued input.
 * @return computes log-multivariate gamma function via recursion
 * \f[ \Gamma_p(a) = \pi^{(p-1)/2} \Gamma(a) \Gamma_{p-1}(a-0.5) \f]
 * where \f$ \Gamma_1(a) = \Gamma(a) \f$.
 */

template <typename T1, typename T2>
constexpr return_t<T1> lmgamma(const T1 a, const T2 p) noexcept {
  return internal::lmgamma_recur(static_cast<return_t<T1>>(a), p);
}

namespace internal {

  template <typename T>
  constexpr T log_binomial_coef_compute(const T n, const T k) noexcept {
    return (lgamma(n + 1) - (lgamma(k + 1) + lgamma(n - k + 1)));
  }

  template <typename T1, typename T2, typename TC = common_return_t<T1, T2>>
  constexpr TC log_binomial_coef_type_check(const T1 n, const T2 k) noexcept {
    return log_binomial_coef_compute(static_cast<TC>(n), static_cast<TC>(k));
  }

}

/**
 * Compile-time log binomial coefficient
 *
 * @param n integral-valued input.
 * @param k integral-valued input.
 * @return computes the log Binomial coefficient
 * \f[ \ln \frac{n!}{k!(n-k)!} = \ln \Gamma(n+1) - [ \ln \Gamma(k+1) + \ln
 * \Gamma(n-k+1) ] \f]
 */

template <typename T1, typename T2>
constexpr common_return_t<T1, T2> log_binomial_coef(
    const T1 n, const T2 k) noexcept {
  return internal::log_binomial_coef_type_check(n, k);
}

/*################################################################################
\
## \
##   Copyright (C) 2016-2020 Keith O'Hara \
## \
##   This file is part of the GCE-Math C++ library. \
## \
##   Licensed under the Apache License, Version 2.0 (the "License"); \
##   you may not use this file except in compliance with the License. \
##   You may obtain a copy of the License at \
## \
##       http://www.apache.org/licenses/LICENSE-2.0 \
## \
##   Unless required by applicable law or agreed to in writing, software \
##   distributed under the License is distributed on an "AS IS" BASIS, \
##   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or \
implied. \
##   See the License for the specific language governing permissions and \
##   limitations under the License. \
## \
################################################################################*/

/*
 * compile-time error function
 */

namespace internal {

  // see
  // http://functions.wolfram.com/GammaBetaErf/Erf/10/01/0007/

  template <typename T>
  constexpr T erf_cf_large_recur(const T x, const int depth) noexcept {
    return (depth < GCEM_ERF_MAX_ITER ? // if
            x + 2 * depth / erf_cf_large_recur(x, depth + 1)
                                      :
                                      // else
            x);
  }

  template <typename T>
  constexpr T erf_cf_large_main(const T x) noexcept {
    return (T(1)
        - T(2) * (exp(-x * x) / T(GCEM_SQRT_PI))
            / erf_cf_large_recur(T(2) * x, 1));
  }

  // see
  // http://functions.wolfram.com/GammaBetaErf/Erf/10/01/0005/

  template <typename T>
  constexpr T erf_cf_small_recur(const T xx, const int depth) noexcept {
    return (depth < GCEM_ERF_MAX_ITER ? // if
            (2 * depth - 1) - 2 * xx
                + 4 * depth * xx / erf_cf_small_recur(xx, depth + 1)
                                      :
                                      // else
            (2 * depth - 1) - 2 * xx);
  }

  template <typename T>
  constexpr T erf_cf_small_main(const T x) noexcept {
    return (T(2) * x * (exp(-x * x) / T(GCEM_SQRT_PI))
        / erf_cf_small_recur(x * x, 1));
  }

  //

  template <typename T>
  constexpr T erf_begin(const T x) noexcept {
    return (x > T(2.1) ? // if
            erf_cf_large_main(x)
                       :
                       // else
            erf_cf_small_main(x));
  }

  template <typename T>
  constexpr T erf_check(const T x) noexcept {
    return ( // NaN check
        is_nan(x) ? GCLIM<T>::quiet_NaN() :
                  // +/-Inf
            is_posinf(x)   ? T(1)
            : is_neginf(x) ? -T(1)
                           :
                           // indistinguishable from zero
            GCLIM<T>::epsilon() > abs(x) ? T(0)
                                         :
                                         // else
            x < T(0) ? -erf_begin(-x)
                     : erf_begin(x));
  }

}

/**
 * Compile-time Gaussian error function
 *
 * @param x a real-valued input.
 * @return computes the Gaussian error function
 * \f[ \text{erf}(x) = \frac{2}{\sqrt{\pi}} \int_0^x \exp( - t^2) dt \f]
 * using a continued fraction representation:
 * \f[ \text{erf}(x) = \frac{2x}{\sqrt{\pi}} \exp(-x^2) \dfrac{1}{1 - 2x^2 +
 * \dfrac{4x^2}{3 - 2x^2 + \dfrac{8x^2}{5 - 2x^2 + \dfrac{12x^2}{7 - 2x^2 +
 * \ddots}}}} \f]
 */

template <typename T>
constexpr return_t<T> erf(const T x) noexcept {
  return internal::erf_check(static_cast<return_t<T>>(x));
}

/*
 * compile-time inverse error function
 *
 * Initial approximation based on:
 * 'Approximating the erfinv function' by Mike Giles
 */

namespace internal {

  template <typename T>
  constexpr T erf_inv_decision(
      const T value, const T p, const T direc, const int iter_count) noexcept;

  //
  // initial value

  // two cases: (1) a < 5; and (2) otherwise

  template <typename T>
  constexpr T erf_inv_initial_val_coef_2(
      const T a, const T p_term, const int order) noexcept {
    return (order == 1   ? T(-0.000200214257L)
            : order == 2 ? T(0.000100950558L) + a * p_term
            : order == 3 ? T(0.00134934322L) + a * p_term
            : order == 4 ? T(-0.003673428440L) + a * p_term
            : order == 5 ? T(0.005739507730L) + a * p_term
            : order == 6 ? T(-0.00762246130L) + a * p_term
            : order == 7 ? T(0.009438870470L) + a * p_term
            : order == 8 ? T(1.001674060000L) + a * p_term
            : order == 9 ? T(2.83297682000L) + a * p_term
                         : p_term);
  }

  template <typename T>
  constexpr T erf_inv_initial_val_case_2(
      const T a, const T p_term, const int order) noexcept {
    return (order == 9 ? // if
            erf_inv_initial_val_coef_2(a, p_term, order)
                       :
                       // else
            erf_inv_initial_val_case_2(
                a, erf_inv_initial_val_coef_2(a, p_term, order), order + 1));
  }

  template <typename T>
  constexpr T erf_inv_initial_val_coef_1(
      const T a, const T p_term, const int order) noexcept {
    return (order == 1   ? T(2.81022636e-08L)
            : order == 2 ? T(3.43273939e-07L) + a * p_term
            : order == 3 ? T(-3.5233877e-06L) + a * p_term
            : order == 4 ? T(-4.39150654e-06L) + a * p_term
            : order == 5 ? T(0.00021858087L) + a * p_term
            : order == 6 ? T(-0.00125372503L) + a * p_term
            : order == 7 ? T(-0.004177681640L) + a * p_term
            : order == 8 ? T(0.24664072700L) + a * p_term
            : order == 9 ? T(1.50140941000L) + a * p_term
                         : p_term);
  }

  template <typename T>
  constexpr T erf_inv_initial_val_case_1(
      const T a, const T p_term, const int order) noexcept {
    return (order == 9 ? // if
            erf_inv_initial_val_coef_1(a, p_term, order)
                       :
                       // else
            erf_inv_initial_val_case_1(
                a, erf_inv_initial_val_coef_1(a, p_term, order), order + 1));
  }

  template <typename T>
  constexpr T erf_inv_initial_val_int(const T a) noexcept {
    return (a < T(5) ? // if
            erf_inv_initial_val_case_1(a - T(2.5), T(0), 1)
                     :
                     // else
            erf_inv_initial_val_case_2(sqrt(a) - T(3), T(0), 1));
  }

  template <typename T>
  constexpr T erf_inv_initial_val(const T x) noexcept {
    return x * erf_inv_initial_val_int(-log((T(1) - x) * (T(1) + x)));
  }

  //
  // Halley recursion

  template <typename T>
  constexpr T erf_inv_err_val(
      const T value, const T p) noexcept { // err_val = f(x)
    return (erf(value) - p);
  }

  template <typename T>
  constexpr T erf_inv_deriv_1(
      const T value) noexcept { // derivative of the error function w.r.t. x
    return (exp(-value * value));
  }

  template <typename T>
  constexpr T erf_inv_deriv_2(
      const T value, const T deriv_1) noexcept { // second derivative of the
                                                 // error function w.r.t. x
    return (deriv_1 * (-T(2) * value));
  }

  template <typename T>
  constexpr T erf_inv_ratio_val_1(
      const T value, const T p, const T deriv_1) noexcept {
    return (erf_inv_err_val(value, p) / deriv_1);
  }

  template <typename T>
  constexpr T erf_inv_ratio_val_2(const T value, const T deriv_1) noexcept {
    return (erf_inv_deriv_2(value, deriv_1) / deriv_1);
  }

  template <typename T>
  constexpr T erf_inv_halley(
      const T ratio_val_1, const T ratio_val_2) noexcept {
    return (ratio_val_1
        / max(T(0.8), min(T(1.2), T(1) - T(0.5) * ratio_val_1 * ratio_val_2)));
  }

  template <typename T>
  constexpr T erf_inv_recur(const T value,
      const T p,
      const T deriv_1,
      const int iter_count) noexcept {
    return erf_inv_decision(value,
        p,
        erf_inv_halley(erf_inv_ratio_val_1(value, p, deriv_1),
            erf_inv_ratio_val_2(value, deriv_1)),
        iter_count);
  }

  template <typename T>
  constexpr T erf_inv_decision(
      const T value, const T p, const T direc, const int iter_count) noexcept {
    return (iter_count < GCEM_ERF_INV_MAX_ITER ? // if
            erf_inv_recur(
                value - direc, p, erf_inv_deriv_1(value), iter_count + 1)
                                               :
                                               // else
            value - direc);
  }

  template <typename T>
  constexpr T erf_inv_recur_begin(const T initial_val, const T p) noexcept {
    return erf_inv_recur(initial_val, p, erf_inv_deriv_1(initial_val), 1);
  }

  template <typename T>
  constexpr T erf_inv_begin(const T p) noexcept {
    return ( // NaN check
        is_nan(p) ? GCLIM<T>::quiet_NaN() :
                  // bad values
            abs(p) > T(1) ? GCLIM<T>::quiet_NaN()
                          :
                          // indistinguishable from 1
            GCLIM<T>::epsilon() > abs(T(1) - p) ? GCLIM<T>::infinity()
                                                :
                                                // indistinguishable from - 1
            GCLIM<T>::epsilon() > abs(T(1) + p) ? -GCLIM<T>::infinity()
                                                :
                                                // else
            erf_inv_recur_begin(erf_inv_initial_val(p), p));
  }

}

/**
 * Compile-time inverse Gaussian error function
 *
 * @param p a real-valued input with values in the unit-interval.
 * @return Computes the inverse Gaussian error function, a value \f$ x \f$
 * such that
 * \f[ f(x) := \text{erf}(x) - p \f]
 * is equal to zero, for a given \c p.
 * GCE-Math finds this root using Halley's method:
 * \f[ x_{n+1} = x_n - \frac{f(x_n)/f'(x_n)}{1 - 0.5 \frac{f(x_n)}{f'(x_n)}
 * \frac{f''(x_n)}{f'(x_n)} } \f] where
 * \f[ \frac{\partial}{\partial x} \text{erf}(x) = \exp(-x^2),
 * \ \ \frac{\partial^2}{\partial x^2} \text{erf}(x) = -2x\exp(-x^2) \f]
 */

template <typename T>
constexpr return_t<T> erf_inv(const T p) noexcept {
  return internal::erf_inv_begin(static_cast<return_t<T>>(p));
}

/*
 * compile-time incomplete beta function
 *
 * see eq. 18.5.17a in the Handbook of Continued Fractions for Special
 * Functions
 */

namespace internal {

  template <typename T>
  constexpr T incomplete_beta_cf(const T a,
      const T b,
      const T z,
      const T c_j,
      const T d_j,
      const T f_j,
      const int depth) noexcept;

  //
  // coefficients; see eq. 18.5.17b

  template <typename T>
  constexpr T incomplete_beta_coef_even(
      const T a, const T b, const T z, const int k) noexcept {
    return (-z * (a + k) * (a + b + k) / ((a + 2 * k) * (a + 2 * k + T(1))));
  }

  template <typename T>
  constexpr T incomplete_beta_coef_odd(
      const T a, const T b, const T z, const int k) noexcept {
    return (z * k * (b - k) / ((a + 2 * k - T(1)) * (a + 2 * k)));
  }

  template <typename T>
  constexpr T incomplete_beta_coef(
      const T a, const T b, const T z, const int depth) noexcept {
    return (!is_odd(depth)
            ? incomplete_beta_coef_even(a, b, z, depth / 2)
            : incomplete_beta_coef_odd(a, b, z, (depth + 1) / 2));
  }

  //
  // update formulae for the modified Lentz method

  template <typename T>
  constexpr T incomplete_beta_c_update(
      const T a, const T b, const T z, const T c_j, const int depth) noexcept {
    return (T(1) + incomplete_beta_coef(a, b, z, depth) / c_j);
  }

  template <typename T>
  constexpr T incomplete_beta_d_update(
      const T a, const T b, const T z, const T d_j, const int depth) noexcept {
    return (T(1) / (T(1) + incomplete_beta_coef(a, b, z, depth) * d_j));
  }

  //
  // convergence-type condition

  template <typename T>
  constexpr T incomplete_beta_decision(const T a,
      const T b,
      const T z,
      const T c_j,
      const T d_j,
      const T f_j,
      const int depth) noexcept {
    return ( // tolerance check
        abs(c_j * d_j - T(1)) < GCEM_INCML_BETA_TOL ? f_j * c_j * d_j :
                                                    // max_iter check
            depth < GCEM_INCML_BETA_MAX_ITER ? // if
            incomplete_beta_cf(a, b, z, c_j, d_j, f_j * c_j * d_j, depth + 1)
                                             :
                                             // else
            f_j * c_j * d_j);
  }

  template <typename T>
  constexpr T incomplete_beta_cf(const T a,
      const T b,
      const T z,
      const T c_j,
      const T d_j,
      const T f_j,
      const int depth) noexcept {
    return incomplete_beta_decision(a,
        b,
        z,
        incomplete_beta_c_update(a, b, z, c_j, depth),
        incomplete_beta_d_update(a, b, z, d_j, depth),
        f_j,
        depth);
  }

  //
  // x^a (1-x)^{b} / (a beta(a,b)) * cf

  template <typename T>
  constexpr T incomplete_beta_begin(const T a, const T b, const T z) noexcept {
    return ((exp(a * log(z) + b * log(T(1) - z) - lbeta(a, b)) / a)
        * incomplete_beta_cf(a,
            b,
            z,
            T(1),
            incomplete_beta_d_update(a, b, z, T(1), 0),
            incomplete_beta_d_update(a, b, z, T(1), 0),
            1));
  }

  template <typename T>
  constexpr T incomplete_beta_check(const T a, const T b, const T z) noexcept {
    return ( // NaN check
        any_nan(a, b, z) ? GCLIM<T>::quiet_NaN() :
                         // indistinguishable from zero
            GCLIM<T>::epsilon() > z ? T(0)
                                    :
                                    // parameter check for performance
            (a + T(1)) / (a + b + T(2)) > z
            ? incomplete_beta_begin(a, b, z)
            : T(1) - incomplete_beta_begin(b, a, T(1) - z));
  }

  template <typename T1,
      typename T2,
      typename T3,
      typename TC = common_return_t<T1, T2, T3>>
  constexpr TC incomplete_beta_type_check(
      const T1 a, const T2 b, const T3 p) noexcept {
    return incomplete_beta_check(
        static_cast<TC>(a), static_cast<TC>(b), static_cast<TC>(p));
  }

}

/**
 * Compile-time regularized incomplete beta function
 *
 * @param a a real-valued, non-negative input.
 * @param b a real-valued, non-negative input.
 * @param z a real-valued, non-negative input.
 *
 * @return computes the regularized incomplete beta function,
 * \f[ \frac{\text{B}(z;\alpha,\beta)}{\text{B}(\alpha,\beta)} =
 * \frac{1}{\text{B}(\alpha,\beta)}\int_0^z t^{a-1} (1-t)^{\beta-1} dt \f]
 * using a continued fraction representation, found in the Handbook of
 * Continued Fractions for Special Functions, and a modified Lentz method.
 * \f[ \frac{\text{B}(z;\alpha,\beta)}{\text{B}(\alpha,\beta)} =
 * \frac{z^{\alpha} (1-t)^{\beta}}{\alpha \text{B}(\alpha,\beta)}
 * \dfrac{a_1}{1
 * + \dfrac{a_2}{1 + \dfrac{a_3}{1 + \dfrac{a_4}{1 + \ddots}}}} \f] where \f$
 * a_1 = 1 \f$ and
 * \f[ a_{2m+2} = - \frac{(\alpha + m)(\alpha + \beta + m)}{(\alpha +
 * 2m)(\alpha
 * + 2m + 1)}, \ m \geq 0 \f]
 * \f[ a_{2m+1} = \frac{m(\beta - m)}{(\alpha + 2m - 1)(\alpha + 2m)}, \ m
 * \geq 1 \f] The Lentz method works as follows: let \f$ f_j \f$ denote the
 * value of the continued fraction up to the first \f$ j \f$ terms; \f$ f_j
 * \f$ is updated as follows:
 * \f[ c_j = 1 + a_j / c_{j-1}, \ \ d_j = 1 / (1 + a_j d_{j-1}) \f]
 * \f[ f_j = c_j d_j f_{j-1} \f]
 */

template <typename T1, typename T2, typename T3>
constexpr common_return_t<T1, T2, T3> incomplete_beta(
    const T1 a, const T2 b, const T3 z) noexcept {
  return internal::incomplete_beta_type_check(a, b, z);
}

/*
 * inverse of the incomplete beta function
 */

namespace internal {

  template <typename T>
  constexpr T incomplete_beta_inv_decision(const T value,
      const T alpha_par,
      const T beta_par,
      const T p,
      const T direc,
      const T lb_val,
      const int iter_count) noexcept;

  //
  // initial value for Halley

  //
  // a,b > 1 case

  template <typename T>
  constexpr T incomplete_beta_inv_initial_val_1_tval(
      const T p) noexcept { // a > 1.0
    return (p > T(0.5) ?    // if
            sqrt(-T(2) * log(T(1) - p))
                       :
                       // else
            sqrt(-T(2) * log(p)));
  }

  template <typename T>
  constexpr T incomplete_beta_inv_initial_val_1_int_begin(
      const T t_val) noexcept { // internal for a > 1.0
    return (t_val
        - (T(2.515517) + T(0.802853) * t_val + T(0.010328) * t_val * t_val)
            / (T(1) + T(1.432788) * t_val + T(0.189269) * t_val * t_val
                + T(0.001308) * t_val * t_val * t_val));
  }

  template <typename T>
  constexpr T incomplete_beta_inv_initial_val_1_int_ab1(
      const T alpha_par, const T beta_par) noexcept {
    return (T(1) / (2 * alpha_par - T(1)) + T(1) / (2 * beta_par - T(1)));
  }

  template <typename T>
  constexpr T incomplete_beta_inv_initial_val_1_int_ab2(
      const T alpha_par, const T beta_par) noexcept {
    return (T(1) / (2 * beta_par - T(1)) - T(1) / (2 * alpha_par - T(1)));
  }

  template <typename T>
  constexpr T incomplete_beta_inv_initial_val_1_int_h(
      const T ab_term_1) noexcept {
    return (T(2) / ab_term_1);
  }

  template <typename T>
  constexpr T incomplete_beta_inv_initial_val_1_int_w(
      const T value, const T ab_term_2, const T h_term) noexcept {
    // return( value * sqrt(h_term + lambda)/h_term - ab_term_2*(lambda
    // + 5.0/6.0 -2.0/(3.0*h_term)) );
    return (value * sqrt(h_term + (value * value - T(3)) / T(6)) / h_term
        - ab_term_2
            * ((value * value - T(3)) / T(6) + T(5) / T(6)
                - T(2) / (T(3) * h_term)));
  }

  template <typename T>
  constexpr T incomplete_beta_inv_initial_val_1_int_end(
      const T alpha_par, const T beta_par, const T w_term) noexcept {
    return (alpha_par / (alpha_par + beta_par * exp(2 * w_term)));
  }

  template <typename T>
  constexpr T incomplete_beta_inv_initial_val_1(const T alpha_par,
      const T beta_par,
      const T t_val,
      const T sgn_term) noexcept { // a > 1.0
    return incomplete_beta_inv_initial_val_1_int_end(alpha_par,
        beta_par,
        incomplete_beta_inv_initial_val_1_int_w(
            sgn_term * incomplete_beta_inv_initial_val_1_int_begin(t_val),
            incomplete_beta_inv_initial_val_1_int_ab2(alpha_par, beta_par),
            incomplete_beta_inv_initial_val_1_int_h(
                incomplete_beta_inv_initial_val_1_int_ab1(
                    alpha_par, beta_par))));
  }

  //
  // a,b else

  template <typename T>
  constexpr T incomplete_beta_inv_initial_val_2_s1(
      const T alpha_par, const T beta_par) noexcept {
    return (pow(alpha_par / (alpha_par + beta_par), alpha_par) / alpha_par);
  }

  template <typename T>
  constexpr T incomplete_beta_inv_initial_val_2_s2(
      const T alpha_par, const T beta_par) noexcept {
    return (pow(beta_par / (alpha_par + beta_par), beta_par) / beta_par);
  }

  template <typename T>
  constexpr T incomplete_beta_inv_initial_val_2(const T alpha_par,
      const T beta_par,
      const T p,
      const T s_1,
      const T s_2) noexcept {
    return (p <= s_1 / (s_1 + s_2)
            ? pow(p * (s_1 + s_2) * alpha_par, T(1) / alpha_par)
            : T(1) - pow(p * (s_1 + s_2) * beta_par, T(1) / beta_par));
  }

  // initial value

  template <typename T>
  constexpr T incomplete_beta_inv_initial_val(
      const T alpha_par, const T beta_par, const T p) noexcept {
    return ((alpha_par > T(1) && beta_par > T(1)) ?
                                                  // if
            incomplete_beta_inv_initial_val_1(alpha_par,
                beta_par,
                incomplete_beta_inv_initial_val_1_tval(p),
                p < T(0.5) ? T(1) : T(-1))
                                                  :
                                                  // else
            p > T(0.5) ?
                       // if
                T(1)
                    - incomplete_beta_inv_initial_val_2(beta_par,
                        alpha_par,
                        T(1) - p,
                        incomplete_beta_inv_initial_val_2_s1(
                            beta_par, alpha_par),
                        incomplete_beta_inv_initial_val_2_s2(
                            beta_par, alpha_par))
                       :
                       // else
                incomplete_beta_inv_initial_val_2(alpha_par,
                    beta_par,
                    p,
                    incomplete_beta_inv_initial_val_2_s1(alpha_par, beta_par),
                    incomplete_beta_inv_initial_val_2_s2(alpha_par, beta_par)));
  }

  //
  // Halley recursion

  template <typename T>
  constexpr T incomplete_beta_inv_err_val(const T value,
      const T alpha_par,
      const T beta_par,
      const T p) noexcept { // err_val = f(x)
    return (incomplete_beta(alpha_par, beta_par, value) - p);
  }

  template <typename T>
  constexpr T incomplete_beta_inv_deriv_1(const T value,
      const T alpha_par,
      const T beta_par,
      const T lb_val) noexcept { // derivative of the incomplete beta function
                                 // w.r.t. x
    return (                     // indistinguishable from zero or one
        GCLIM<T>::epsilon() > abs(value)              ? T(0)
            : GCLIM<T>::epsilon() > abs(T(1) - value) ? T(0)
                                                      :
                                                      // else
            exp((alpha_par - T(1)) * log(value)
                + (beta_par - T(1)) * log(T(1) - value) - lb_val));
  }

  template <typename T>
  constexpr T incomplete_beta_inv_deriv_2(const T value,
      const T alpha_par,
      const T beta_par,
      const T deriv_1) noexcept { // second derivative of the incomplete beta
                                  // function w.r.t. x
    return (deriv_1
        * ((alpha_par - T(1)) / value - (beta_par - T(1)) / (T(1) - value)));
  }

  template <typename T>
  constexpr T incomplete_beta_inv_ratio_val_1(const T value,
      const T alpha_par,
      const T beta_par,
      const T p,
      const T deriv_1) noexcept {
    return (
        incomplete_beta_inv_err_val(value, alpha_par, beta_par, p) / deriv_1);
  }

  template <typename T>
  constexpr T incomplete_beta_inv_ratio_val_2(const T value,
      const T alpha_par,
      const T beta_par,
      const T deriv_1) noexcept {
    return (incomplete_beta_inv_deriv_2(value, alpha_par, beta_par, deriv_1)
        / deriv_1);
  }

  template <typename T>
  constexpr T incomplete_beta_inv_halley(
      const T ratio_val_1, const T ratio_val_2) noexcept {
    return (ratio_val_1
        / max(T(0.8), min(T(1.2), T(1) - T(0.5) * ratio_val_1 * ratio_val_2)));
  }

  template <typename T>
  constexpr T incomplete_beta_inv_recur(const T value,
      const T alpha_par,
      const T beta_par,
      const T p,
      const T deriv_1,
      const T lb_val,
      const int iter_count) noexcept {
    return ( // derivative = 0
        GCLIM<T>::epsilon() > abs(deriv_1)
            ? incomplete_beta_inv_decision(value,
                  alpha_par,
                  beta_par,
                  p,
                  T(0),
                  lb_val,
                  GCEM_INCML_BETA_INV_MAX_ITER + 1)
            :
            // else
            incomplete_beta_inv_decision(value,
                alpha_par,
                beta_par,
                p,
                incomplete_beta_inv_halley(
                    incomplete_beta_inv_ratio_val_1(
                        value, alpha_par, beta_par, p, deriv_1),
                    incomplete_beta_inv_ratio_val_2(
                        value, alpha_par, beta_par, deriv_1)),
                lb_val,
                iter_count));
  }

  template <typename T>
  constexpr T incomplete_beta_inv_decision(const T value,
      const T alpha_par,
      const T beta_par,
      const T p,
      const T direc,
      const T lb_val,
      const int iter_count) noexcept {
    return (iter_count <= GCEM_INCML_BETA_INV_MAX_ITER ?
                                                       // if
            incomplete_beta_inv_recur(value - direc,
                alpha_par,
                beta_par,
                p,
                incomplete_beta_inv_deriv_1(value, alpha_par, beta_par, lb_val),
                lb_val,
                iter_count + 1)
                                                       :
                                                       // else
            value - direc);
  }

  template <typename T>
  constexpr T incomplete_beta_inv_begin(const T initial_val,
      const T alpha_par,
      const T beta_par,
      const T p,
      const T lb_val) noexcept {
    return incomplete_beta_inv_recur(initial_val,
        alpha_par,
        beta_par,
        p,
        incomplete_beta_inv_deriv_1(initial_val, alpha_par, beta_par, lb_val),
        lb_val,
        1);
  }

  template <typename T>
  constexpr T incomplete_beta_inv_check(
      const T alpha_par, const T beta_par, const T p) noexcept {
    return ( // NaN check
        any_nan(alpha_par, beta_par, p) ? GCLIM<T>::quiet_NaN() :
                                        // indistinguishable from zero or one
            GCLIM<T>::epsilon() > p               ? T(0)
            : GCLIM<T>::epsilon() > abs(T(1) - p) ? T(1)
                                                  :
                                                  // else
            incomplete_beta_inv_begin(
                incomplete_beta_inv_initial_val(alpha_par, beta_par, p),
                alpha_par,
                beta_par,
                p,
                lbeta(alpha_par, beta_par)));
  }

  template <typename T1,
      typename T2,
      typename T3,
      typename TC = common_t<T1, T2, T3>>
  constexpr TC incomplete_beta_inv_type_check(
      const T1 a, const T2 b, const T3 p) noexcept {
    return incomplete_beta_inv_check(
        static_cast<TC>(a), static_cast<TC>(b), static_cast<TC>(p));
  }

}

/**
 * Compile-time inverse incomplete beta function
 *
 * @param a a real-valued, non-negative input.
 * @param b a real-valued, non-negative input.
 * @param p a real-valued input with values in the unit-interval.
 *
 * @return Computes the inverse incomplete beta function, a value \f$ x \f$
 * such that
 * \f[ f(x) := \frac{\text{B}(x;\alpha,\beta)}{\text{B}(\alpha,\beta)} - p \f]
 * equal to zero, for a given \c p.
 * GCE-Math finds this root using Halley's method:
 * \f[ x_{n+1} = x_n - \frac{f(x_n)/f'(x_n)}{1 - 0.5 \frac{f(x_n)}{f'(x_n)}
 * \frac{f''(x_n)}{f'(x_n)} } \f] where
 * \f[ \frac{\partial}{\partial x}
 * \left(\frac{\text{B}(x;\alpha,\beta)}{\text{B}(\alpha,\beta)}\right) =
 * \frac{1}{\text{B}(\alpha,\beta)} x^{\alpha-1} (1-x)^{\beta-1} \f]
 * \f[ \frac{\partial^2}{\partial x^2}
 * \left(\frac{\text{B}(x;\alpha,\beta)}{\text{B}(\alpha,\beta)}\right) =
 * \frac{1}{\text{B}(\alpha,\beta)} x^{\alpha-1} (1-x)^{\beta-1} \left(
 * \frac{\alpha-1}{x} - \frac{\beta-1}{1 - x} \right) \f]
 */

template <typename T1, typename T2, typename T3>
constexpr common_t<T1, T2, T3> incomplete_beta_inv(
    const T1 a, const T2 b, const T3 p) noexcept {
  return internal::incomplete_beta_inv_type_check(a, b, p);
}

/*
 * compile-time (regularized) incomplete gamma function
 */

namespace internal {

  // 50 point Gauss-Legendre quadrature

  template <typename T>
  constexpr T incomplete_gamma_quad_inp_vals(
      const T lb, const T ub, const int counter) noexcept {
    return (ub - lb) * gauss_legendre_50_points[counter] / T(2)
        + (ub + lb) / T(2);
  }

  template <typename T>
  constexpr T incomplete_gamma_quad_weight_vals(
      const T lb, const T ub, const int counter) noexcept {
    return (ub - lb) * gauss_legendre_50_weights[counter] / T(2);
  }

  template <typename T>
  constexpr T incomplete_gamma_quad_fn(
      const T x, const T a, const T lg_term) noexcept {
    return exp(-x + (a - T(1)) * log(x) - lg_term);
  }

  template <typename T>
  constexpr T incomplete_gamma_quad_recur(const T lb,
      const T ub,
      const T a,
      const T lg_term,
      const int counter) noexcept {
    return (counter < 49 ? // if
            incomplete_gamma_quad_fn(
                incomplete_gamma_quad_inp_vals(lb, ub, counter), a, lg_term)
                    * incomplete_gamma_quad_weight_vals(lb, ub, counter)
                + incomplete_gamma_quad_recur(lb, ub, a, lg_term, counter + 1)
                         :
                         // else
            incomplete_gamma_quad_fn(
                incomplete_gamma_quad_inp_vals(lb, ub, counter), a, lg_term)
                * incomplete_gamma_quad_weight_vals(lb, ub, counter));
  }

  template <typename T>
  constexpr T incomplete_gamma_quad_lb(const T a, const T z) noexcept {
    return (a > T(1000) ? max(T(0), min(z, a) - 11 * sqrt(a))
                        : // break integration into ranges
            a > T(800)   ? max(T(0), min(z, a) - 11 * sqrt(a))
            : a > T(500) ? max(T(0), min(z, a) - 10 * sqrt(a))
            : a > T(300) ? max(T(0), min(z, a) - 10 * sqrt(a))
            : a > T(100) ? max(T(0), min(z, a) - 9 * sqrt(a))
            : a > T(90)  ? max(T(0), min(z, a) - 9 * sqrt(a))
            : a > T(70)  ? max(T(0), min(z, a) - 8 * sqrt(a))
            : a > T(50)  ? max(T(0), min(z, a) - 7 * sqrt(a))
            : a > T(40)  ? max(T(0), min(z, a) - 6 * sqrt(a))
            : a > T(30)  ? max(T(0), min(z, a) - 5 * sqrt(a))
                         :
                         // else
            max(T(0), min(z, a) - 4 * sqrt(a)));
  }

  template <typename T>
  constexpr T incomplete_gamma_quad_ub(const T a, const T z) noexcept {
    return (a > T(1000)  ? min(z, a + 10 * sqrt(a))
            : a > T(800) ? min(z, a + 10 * sqrt(a))
            : a > T(500) ? min(z, a + 9 * sqrt(a))
            : a > T(300) ? min(z, a + 9 * sqrt(a))
            : a > T(100) ? min(z, a + 8 * sqrt(a))
            : a > T(90)  ? min(z, a + 8 * sqrt(a))
            : a > T(70)  ? min(z, a + 7 * sqrt(a))
            : a > T(50)  ? min(z, a + 6 * sqrt(a))
                         :
                         // else
            min(z, a + 5 * sqrt(a)));
  }

  template <typename T>
  constexpr T incomplete_gamma_quad(const T a, const T z) noexcept {
    return incomplete_gamma_quad_recur(incomplete_gamma_quad_lb(a, z),
        incomplete_gamma_quad_ub(a, z),
        a,
        lgamma(a),
        0);
  }

  // reverse cf expansion
  // see: https://functions.wolfram.com/GammaBetaErf/Gamma2/10/0003/

  template <typename T>
  constexpr T incomplete_gamma_cf_2_recur(
      const T a, const T z, const int depth) noexcept {
    return (depth < 100 ? // if
            (1 + (depth - 1) * 2 - a + z)
                + depth * (a - depth)
                    / incomplete_gamma_cf_2_recur(a, z, depth + 1)
                        :
                        // else
            (1 + (depth - 1) * 2 - a + z));
  }

  template <typename T>
  constexpr T incomplete_gamma_cf_2(const T a,
      const T z) noexcept { // lower (regularized) incomplete gamma function
    return (T(1.0)
        - exp(a * log(z) - z - lgamma(a))
            / incomplete_gamma_cf_2_recur(a, z, 1));
  }

  // cf expansion
  // see: http://functions.wolfram.com/GammaBetaErf/Gamma2/10/0009/

  template <typename T>
  constexpr T incomplete_gamma_cf_1_coef(
      const T a, const T z, const int depth) noexcept {
    return (is_odd(depth) ? -(a - 1 + T(depth + 1) / T(2)) * z
                          : T(depth) / T(2) * z);
  }

  template <typename T>
  constexpr T incomplete_gamma_cf_1_recur(
      const T a, const T z, const int depth) noexcept {
    return (depth < GCEM_INCML_GAMMA_MAX_ITER ? // if
            (a + depth - 1)
                + incomplete_gamma_cf_1_coef(a, z, depth)
                    / incomplete_gamma_cf_1_recur(a, z, depth + 1)
                                              :
                                              // else
            (a + depth - 1));
  }

  template <typename T>
  constexpr T incomplete_gamma_cf_1(const T a,
      const T z) noexcept { // lower (regularized) incomplete gamma function
    return (
        exp(a * log(z) - z - lgamma(a)) / incomplete_gamma_cf_1_recur(a, z, 1));
  }

  //

  template <typename T>
  constexpr T incomplete_gamma_check(const T a, const T z) noexcept {
    return ( // NaN check
        any_nan(a, z) ? GCLIM<T>::quiet_NaN() :
                      //
            a < T(0) ? GCLIM<T>::quiet_NaN()
                     :
                     //
            GCLIM<T>::epsilon() > z ? T(0)
                                    :
                                    //
            GCLIM<T>::epsilon() > a ? T(1)
                                    :
                                    // cf or quadrature
            (a < T(10)) && (z - a < T(10))  ? incomplete_gamma_cf_1(a, z)
            : (a < T(10)) || (z / a > T(3)) ? incomplete_gamma_cf_2(a, z)
                                            :
                                            // else
            incomplete_gamma_quad(a, z));
  }

  template <typename T1, typename T2, typename TC = common_return_t<T1, T2>>
  constexpr TC incomplete_gamma_type_check(const T1 a, const T2 p) noexcept {
    return incomplete_gamma_check(static_cast<TC>(a), static_cast<TC>(p));
  }

}

/**
 * Compile-time regularized lower incomplete gamma function
 *
 * @param a a real-valued, non-negative input.
 * @param x a real-valued, non-negative input.
 *
 * @return the regularized lower incomplete gamma function evaluated at (\c a,
 * \c x),
 * \f[ \frac{\gamma(a,x)}{\Gamma(a)} = \frac{1}{\Gamma(a)} \int_0^x t^{a-1}
 * \exp(-t) dt \f] When \c a is not too large, the value is computed using the
 * continued fraction representation of the upper incomplete gamma function,
 * \f$
 * \Gamma(a,x) \f$, using
 * \f[ \Gamma(a,x) = \Gamma(a) - \dfrac{x^a\exp(-x)}{a - \dfrac{ax}{a + 1 +
 * \dfrac{x}{a + 2 - \dfrac{(a+1)x}{a + 3 + \dfrac{2x}{a + 4 - \ddots}}}}} \f]
 * where \f$ \gamma(a,x) \f$ and \f$ \Gamma(a,x) \f$ are connected via
 * \f[ \frac{\gamma(a,x)}{\Gamma(a)} + \frac{\Gamma(a,x)}{\Gamma(a)} = 1 \f]
 * When \f$ a > 10 \f$, a 50-point Gauss-Legendre quadrature scheme is
 * employed.
 */

template <typename T1, typename T2>
constexpr common_return_t<T1, T2> incomplete_gamma(
    const T1 a, const T2 x) noexcept {
  return internal::incomplete_gamma_type_check(a, x);
}

/*
 * inverse of the incomplete gamma function
 */

namespace internal {

  template <typename T>
  constexpr T incomplete_gamma_inv_decision(const T value,
      const T a,
      const T p,
      const T direc,
      const T lg_val,
      const int iter_count) noexcept;

  //
  // initial value for Halley

  template <typename T>
  constexpr T incomplete_gamma_inv_t_val_1(const T p) noexcept { // a > 1.0
    return (p > T(0.5) ? sqrt(-2 * log(T(1) - p)) : sqrt(-2 * log(p)));
  }

  template <typename T>
  constexpr T incomplete_gamma_inv_t_val_2(const T a) noexcept { // a <= 1.0
    return (T(1) - T(0.253) * a - T(0.12) * a * a);
  }

  //

  template <typename T>
  constexpr T incomplete_gamma_inv_initial_val_1_int_begin(
      const T t_val) noexcept { // internal for a > 1.0
    return (t_val
        - (T(2.515517L) + T(0.802853L) * t_val + T(0.010328L) * t_val * t_val)
            / (T(1) + T(1.432788L) * t_val + T(0.189269L) * t_val * t_val
                + T(0.001308L) * t_val * t_val * t_val));
  }

  template <typename T>
  constexpr T incomplete_gamma_inv_initial_val_1_int_end(
      const T value_inp, const T a) noexcept { // internal for a > 1.0
    return max(T(1E-04),
        a * pow(T(1) - T(1) / (9 * a) - value_inp / (3 * sqrt(a)), 3));
  }

  template <typename T>
  constexpr T incomplete_gamma_inv_initial_val_1(
      const T a, const T t_val, const T sgn_term) noexcept { // a > 1.0
    return incomplete_gamma_inv_initial_val_1_int_end(
        sgn_term * incomplete_gamma_inv_initial_val_1_int_begin(t_val), a);
  }

  template <typename T>
  constexpr T incomplete_gamma_inv_initial_val_2(
      const T a, const T p, const T t_val) noexcept { // a <= 1.0
    return (p < t_val ?                               // if
            pow(p / t_val, T(1) / a)
                      :
                      // else
            T(1) - log(T(1) - (p - t_val) / (T(1) - t_val)));
  }

  // initial value

  template <typename T>
  constexpr T incomplete_gamma_inv_initial_val(const T a, const T p) noexcept {
    return (a > T(1) ? // if
            incomplete_gamma_inv_initial_val_1(
                a, incomplete_gamma_inv_t_val_1(p), p > T(0.5) ? T(-1) : T(1))
                     :
                     // else
            incomplete_gamma_inv_initial_val_2(
                a, p, incomplete_gamma_inv_t_val_2(a)));
  }

  //
  // Halley recursion

  template <typename T>
  constexpr T incomplete_gamma_inv_err_val(
      const T value, const T a, const T p) noexcept { // err_val = f(x)
    return (incomplete_gamma(a, value) - p);
  }

  template <typename T>
  constexpr T incomplete_gamma_inv_deriv_1(const T value,
      const T a,
      const T lg_val) noexcept { // derivative of the incomplete gamma
                                 // function w.r.t. x
    return (exp(-value + (a - T(1)) * log(value) - lg_val));
  }

  template <typename T>
  constexpr T incomplete_gamma_inv_deriv_2(const T value,
      const T a,
      const T deriv_1) noexcept { // second derivative of the incomplete gamma
                                  // function w.r.t. x
    return (deriv_1 * ((a - T(1)) / value - T(1)));
  }

  template <typename T>
  constexpr T incomplete_gamma_inv_ratio_val_1(
      const T value, const T a, const T p, const T deriv_1) noexcept {
    return (incomplete_gamma_inv_err_val(value, a, p) / deriv_1);
  }

  template <typename T>
  constexpr T incomplete_gamma_inv_ratio_val_2(
      const T value, const T a, const T deriv_1) noexcept {
    return (incomplete_gamma_inv_deriv_2(value, a, deriv_1) / deriv_1);
  }

  template <typename T>
  constexpr T incomplete_gamma_inv_halley(
      const T ratio_val_1, const T ratio_val_2) noexcept {
    return (ratio_val_1
        / max(T(0.8), min(T(1.2), T(1) - T(0.5) * ratio_val_1 * ratio_val_2)));
  }

  template <typename T>
  constexpr T incomplete_gamma_inv_recur(const T value,
      const T a,
      const T p,
      const T deriv_1,
      const T lg_val,
      const int iter_count) noexcept {
    return incomplete_gamma_inv_decision(value,
        a,
        p,
        incomplete_gamma_inv_halley(
            incomplete_gamma_inv_ratio_val_1(value, a, p, deriv_1),
            incomplete_gamma_inv_ratio_val_2(value, a, deriv_1)),
        lg_val,
        iter_count);
  }

  template <typename T>
  constexpr T incomplete_gamma_inv_decision(const T value,
      const T a,
      const T p,
      const T direc,
      const T lg_val,
      const int iter_count) noexcept {
    // return( abs(direc) > GCEM_INCML_GAMMA_INV_TOL ?
    // incomplete_gamma_inv_recur(value - direc, a, p,
    // incomplete_gamma_inv_deriv_1(value,a,lg_val), lg_val) : value - direc
    // );
    return (iter_count <= GCEM_INCML_GAMMA_INV_MAX_ITER ? // if
            incomplete_gamma_inv_recur(value - direc,
                a,
                p,
                incomplete_gamma_inv_deriv_1(value, a, lg_val),
                lg_val,
                iter_count + 1)
                                                        :
                                                        // else
            value - direc);
  }

  template <typename T>
  constexpr T incomplete_gamma_inv_begin(
      const T initial_val, const T a, const T p, const T lg_val) noexcept {
    return incomplete_gamma_inv_recur(initial_val,
        a,
        p,
        incomplete_gamma_inv_deriv_1(initial_val, a, lg_val),
        lg_val,
        1);
  }

  template <typename T>
  constexpr T incomplete_gamma_inv_check(const T a, const T p) noexcept {
    return ( // NaN check
        any_nan(a, p) ? GCLIM<T>::quiet_NaN() :
                      //
            GCLIM<T>::epsilon() > p               ? T(0)
            : p > T(1)                            ? GCLIM<T>::quiet_NaN()
            : GCLIM<T>::epsilon() > abs(T(1) - p) ? GCLIM<T>::infinity()
                                                  :
                                                  //
            GCLIM<T>::epsilon() > a ? T(0)
                                    :
                                    // else
            incomplete_gamma_inv_begin(
                incomplete_gamma_inv_initial_val(a, p), a, p, lgamma(a)));
  }

  template <typename T1, typename T2, typename TC = common_return_t<T1, T2>>
  constexpr TC incomplete_gamma_inv_type_check(
      const T1 a, const T2 p) noexcept {
    return incomplete_gamma_inv_check(static_cast<TC>(a), static_cast<TC>(p));
  }

}

/**
 * Compile-time inverse incomplete gamma function
 *
 * @param a a real-valued, non-negative input.
 * @param p a real-valued input with values in the unit-interval.
 *
 * @return Computes the inverse incomplete gamma function, a value \f$ x \f$
 * such that
 * \f[ f(x) := \frac{\gamma(a,x)}{\Gamma(a)} - p \f]
 * equal to zero, for a given \c p.
 * GCE-Math finds this root using Halley's method:
 * \f[ x_{n+1} = x_n - \frac{f(x_n)/f'(x_n)}{1 - 0.5 \frac{f(x_n)}{f'(x_n)}
 * \frac{f''(x_n)}{f'(x_n)} } \f] where
 * \f[ \frac{\partial}{\partial x} \left(\frac{\gamma(a,x)}{\Gamma(a)}\right)
 * =
 * \frac{1}{\Gamma(a)} x^{a-1} \exp(-x) \f]
 * \f[ \frac{\partial^2}{\partial x^2}
 * \left(\frac{\gamma(a,x)}{\Gamma(a)}\right) = \frac{1}{\Gamma(a)} x^{a-1}
 * \exp(-x) \left( \frac{a-1}{x} - 1 \right) \f]
 */

template <typename T1, typename T2>
constexpr common_return_t<T1, T2> incomplete_gamma_inv(
    const T1 a, const T2 p) noexcept {
  return internal::incomplete_gamma_inv_type_check(a, p);
}
}
