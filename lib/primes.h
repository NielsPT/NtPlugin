#pragma once

/**
 * @file primes.h
 * @author Niels Thøgersen (niels.thoegersen@gmail.com)
 * @brief Prime numbers.
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

#include <array>
#include <cstddef>

namespace NtFx {
constexpr static std::array<size_t, 168> primesTo1000 {
  2,
  3,
  5,
  7,
  11,
  13,
  17,
  19,
  23,
  29,
  31,
  37,
  41,
  43,
  47,
  53,
  59,
  61,
  67,
  71,
  73,
  79,
  83,
  89,
  97,
  101,
  103,
  107,
  109,
  113,
  127,
  131,
  137,
  139,
  149,
  151,
  157,
  163,
  167,
  173,
  179,
  181,
  191,
  193,
  197,
  199,
  211,
  223,
  227,
  229,
  233,
  239,
  241,
  251,
  257,
  263,
  269,
  271,
  277,
  281,
  283,
  293,
  307,
  311,
  313,
  317,
  331,
  337,
  347,
  349,
  353,
  359,
  367,
  373,
  379,
  383,
  389,
  397,
  401,
  409,
  419,
  421,
  431,
  433,
  439,
  443,
  449,
  457,
  461,
  463,
  467,
  479,
  487,
  491,
  499,
  503,
  509,
  521,
  523,
  541,
  547,
  557,
  563,
  569,
  571,
  577,
  587,
  593,
  599,
  601,
  607,
  613,
  617,
  619,
  631,
  641,
  643,
  647,
  653,
  659,
  661,
  673,
  677,
  683,
  691,
  701,
  709,
  719,
  727,
  733,
  739,
  743,
  751,
  757,
  761,
  769,
  773,
  787,
  797,
  809,
  811,
  821,
  823,
  827,
  829,
  839,
  853,
  857,
  859,
  863,
  877,
  881,
  883,
  887,
  907,
  911,
  919,
  929,
  937,
  941,
  947,
  953,
  967,
  971,
  977,
  983,
  991,
  997,
};

inline consteval std::array<size_t, 1000> fillPrimesOrZeros() {
  std::array<size_t, 1000> x { };
  for (size_t i = 0; i < 1000; i++) { x[i] = 0; }
  for (size_t i = 0; i < 168; i++) { x[primesTo1000[i]] = primesTo1000[i]; }
  return x;
}

constexpr std::array<size_t, 1000> primeOrZeros = fillPrimesOrZeros();

constexpr static inline size_t nextPrime(size_t x) {
  for (size_t i = x + 1; i < 1000; i++) {
    if (primeOrZeros[i]) { return i; }
  }
  return 997;
}

static inline bool isPrime(size_t x) {
  for (size_t i = 2; i <= x / 2; ++i) {
    if (x % i == 0) { return false; }
  }
  return true;
}
}