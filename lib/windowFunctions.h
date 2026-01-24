#pragma once
#include <cassert>
#include <cmath>
#include <complex>
#include <vector>

namespace NtFx {

template <typename T>
inline static std::vector<T> cosine_window(
    size_t n, const T* coeff, size_t ncoeff, bool sflag) {
  std::vector<T> w;
  if (n == 1) {
    w.push_back(1.0);
  } else {
    const size_t wlength = sflag ? (n - 1) : n;
    for (size_t i = 0; i < n; ++i) {
      T wi = 0.0;
      for (size_t j = 0; j < ncoeff; ++j) {
        wi += coeff[j] * cos(i * j * 2.0 * M_PI / wlength);
      }
      w.push_back(wi);
    }
  }
  return w;
}

template <typename T>
inline static std::vector<T> rectwin(size_t n) noexcept {
  std::vector<T> w;
  for (size_t i = 0; i < n; ++i) { w.push_back(1.0); }
  return w;
}

template <typename T>
inline static std::vector<T> hanning(size_t n, bool sflag = false) noexcept {
  const T coeff[2] = { 0.5, -0.5 };
  return cosine_window<T>(n, coeff, sizeof(coeff) / sizeof(T), sflag);
}

template <typename T>
inline static std::vector<T> hamming(size_t n, bool sflag = false) noexcept {
  const T coeff[2] = { 0.54, -0.46 };
  return cosine_window<T>(n, coeff, sizeof(coeff) / sizeof(T), sflag);
}

template <typename T>
inline static std::vector<T> blackman(size_t n, bool sflag = false) noexcept {
  const T coeff[3] = { 0.42, -0.5, 0.08 };
  return cosine_window<T>(n, coeff, sizeof(coeff) / sizeof(T), sflag);
}

template <typename T>
inline static std::vector<T> blackmanharris(
    size_t n, bool sflag = false) noexcept {
  const T coeff[4] = { 0.35875, -0.48829, 0.14128, -0.01168 };
  return cosine_window<T>(n, coeff, sizeof(coeff) / sizeof(T), sflag);
}

template <typename T>
inline static std::vector<T> nuttallwin(size_t n, bool sflag = false) noexcept {
  const T coeff[4] = { 0.3635819, -0.4891775, 0.1365995, -0.0106411 };
  return cosine_window<T>(n, coeff, sizeof(coeff) / sizeof(T), sflag);
}

template <typename T>
inline static std::vector<T> nuttallwin_octave(
    size_t n, bool sflag = false) noexcept {
  const T coeff[4] = { 0.355768, -0.487396, 0.144232, -0.012604 };
  return cosine_window<T>(n, coeff, sizeof(coeff) / sizeof(T), sflag);
}

template <typename T>
inline static std::vector<T> flattopwin(size_t n, bool sflag = false) noexcept {
  const T coeff[5] = {
    0.21557895, -0.41663158, 0.277263158, -0.083578947, 0.006947368
  };

  return cosine_window<T>(n, coeff, sizeof(coeff) / sizeof(T), sflag);
}

template <typename T>
inline static std::vector<T> flattopwin_octave(
    size_t n, bool sflag = false) noexcept {

  const T coeff[5] = { 1.0 / 4.6402,
    -1.93 / 4.6402,
    1.29 / 4.6402,
    -0.388 / 4.6402,
    0.0322 / 4.6402 };

  return cosine_window<T>(n, coeff, sizeof(coeff) / sizeof(T), sflag);
}

template <typename T>
inline static std::vector<T> triang(size_t n) noexcept {
  std::vector<T> w;
  const size_t denominator = (n % 2 != 0) ? (n + 1) : n;
  for (size_t i = 0; i < n; ++i) {
    w.push_back(1.0 - std::abs(2.0 * i - (n - 1)) / denominator);
  }
  return w;
}

template <typename T>
inline static std::vector<T> bartlett(size_t n) noexcept {
  std::vector<T> w;
  if (n == 1) {
    w.push_back(1.0);
  } else {
    const size_t denominator = (n - 1);
    for (size_t i = 0; i < n; ++i) {
      w.push_back(1.0 - std::abs(2.0 * i - (n - 1)) / denominator);
    }
  }
  return w;
}

template <typename T>
inline static std::vector<T> barthannwin(size_t n) noexcept {
  std::vector<T> w;
  if (n == 1) {
    w.push_back(1.0);
  } else {
    for (size_t i = 0; i < n; ++i) {
      const T x = std::abs(i / (n - 1.0) - 0.5);
      w.push_back(0.62 - 0.48 * x + 0.38 * cos(2.0 * M_PI * x));
    }
  }
  return w;
}

template <typename T>
inline static std::vector<T> bohmanwin(size_t n) noexcept {
  std::vector<T> w;
  if (n == 1) {
    w.push_back(1.0);
  } else {
    for (size_t i = 0; i < n; ++i) {
      const T x = std::abs(2.0 * i - (n - 1)) / (n - 1);
      w.push_back((1.0 - x) * cos(M_PI * x) + sin(M_PI * x) / M_PI);
    }
  }
  return w;
}

template <typename T>
inline static std::vector<T> parzenwin(size_t n) noexcept {
  std::vector<T> w;
  if (n == 1) {
    w.push_back(1.0);
  } else {
    for (size_t i = 0; i < n; ++i) {
      const T x = std::abs(2.0 * i - (n - 1)) / n;
      const T y = 1.0 - x;
      w.push_back(
          std::min(1.0 - 6.0 * x * x + 6.0 * x * x * x, 2.0 * y * y * y));
    }
  }
  return w;
}

template <typename T>
inline static std::vector<T> gausswin(size_t n, T alpha) noexcept {
  std::vector<T> w;
  if (n == 1) {
    w.push_back(1.0);
  } else {
    for (size_t i = 0; i < n; ++i) {
      const T x          = std::abs(2.0 * i - (n - 1)) / (n - 1);
      const T ax         = alpha * x;
      const T ax_squared = ax * ax;
      w.push_back(std::exp(-0.5 * ax_squared));
    }
  }
  return w;
}

template <typename T>
inline static std::vector<T> tukeywin(size_t n, T r) noexcept {
  std::vector<T> w;
  if (n == 1) {
    w.push_back(1.0);
  } else {
    r = std::max(0.0, std::min(1.0, r));
    for (size_t i = 0; i < n; ++i) {
      w.push_back(
          (cos(std::max(std::abs((T)i - (n - 1) / 2.0) * (2.0 / (n - 1) / r)
                       - (1.0 / r - 1.0),
                   0.0)
               * M_PI)
              + 1.0)
          / 2.0);
    }
  }
  return w;
}

template <typename T>
static inline T sq(T x) noexcept {
  return x * x;
}

template <typename T>
inline static std::vector<T> taylorwin(size_t n, size_t nbar, T sll) noexcept {
  std::vector<T> w;
  w.reserve(n);
  const T amplification = pow(10.0, -sll / 20.0);
  const T a             = acosh(amplification) / M_PI;
  const T a2            = sq(a);
  const T sp2           = sq(nbar) / (a2 + sq(nbar - 0.5));
  for (size_t i = 0; i < n; ++i) { w.push_back(1.0); }
  for (size_t m = 1; m < nbar; ++m) {
    T numerator   = 1.0;
    T denominator = 1.0;
    for (size_t i = 1; i < nbar; ++i) {
      numerator *= (1.0 - sq(m) / (sp2 * (a2 + sq(i - 0.5))));
      if (i != m) { denominator *= (1.0 - static_cast<T>(sq(m)) / sq(i)); }
    }
    const T Fm = -(numerator / denominator);
    for (size_t i = 0; i < n; ++i) {
      const T x = 2 * M_PI * (i + 0.5) / n;
      w[i] += Fm * cos(m * x);
    }
  }
  return w;
}

template <typename T>
static T chbevl(T x, const T* coeff, size_t n) {
  T b0 = 0.0;
  T b1 = 0.0;
  T b2;
  for (size_t i = 0; i < n; ++i) {
    b2 = b1;
    b1 = b0;
    b0 = x * b1 - b2 + coeff[i];
  }
  return 0.5 * (b0 - b2);
}

template <typename T>
static T bessel_i0(T x) {
  const T A[30] = { -4.41534164647933937950e-18,
    3.33079451882223809783e-17,
    -2.43127984654795469359e-16,
    1.71539128555513303061e-15,
    -1.16853328779934516808e-14,
    7.67618549860493561688e-14,
    -4.85644678311192946090e-13,
    2.95505266312963983461e-12,
    -1.72682629144155570723e-11,
    9.67580903537323691224e-11,
    -5.18979560163526290666e-10,
    2.65982372468238665035e-9,
    -1.30002500998624804212e-8,
    6.04699502254191894932e-8,
    -2.67079385394061173391e-7,
    1.11738753912010371815e-6,
    -4.41673835845875056359e-6,
    1.64484480707288970893e-5,
    -5.75419501008210370398e-5,
    1.88502885095841655729e-4,
    -5.76375574538582365885e-4,
    1.63947561694133579842e-3,
    -4.32430999505057594430e-3,
    1.05464603945949983183e-2,
    -2.37374148058994688156e-2,
    4.93052842396707084878e-2,
    -9.49010970480476444210e-2,
    1.71620901522208775349e-1,
    -3.04682672343198398683e-1,
    6.76795274409476084995e-1 };

  const T B[30] = { -7.23318048787475395456e-18,
    -4.83050448594418207126e-18,
    4.46562142029675999901e-17,
    3.46122286769746109310e-17,
    -2.82762398051658348494e-16,
    -3.42548561967721913462e-16,
    1.77256013305652638360e-15,
    3.81168066935262242075e-15,
    -9.55484669882830764870e-15,
    -4.15056934728722208663e-14,
    1.54008621752140982691e-14,
    3.85277838274214270114e-13,
    7.18012445138366623367e-13,
    -1.79417853150680611778e-12,
    -1.32158118404477131188e-11,
    -3.14991652796324136454e-11,
    1.18891471078464383424e-11,
    4.94060238822496958910e-10,
    3.39623202570838634515e-9,
    2.26666899049817806459e-8,
    2.04891858946906374183e-7,
    2.89137052083475648297e-6,
    6.88975834691682398426e-5,
    3.36911647825569408990e-3,
    8.04490411014108831608e-1 };

  x = std::abs(x);
  if (x <= 8.0) {
    return std::exp(x) * chbevl<T>(x / 2.0 - 2.0, A, 30);
  } else {
    return std::exp(x) * chbevl<T>(32.0 / x - 2.0, B, 25) / sqrt(x);
  }
}

template <typename T>
inline static std::vector<T> kaiser(size_t n, T beta) noexcept {
  std::vector<T> w;
  if (n == 1) {
    w.push_back(1.0);
  } else {
    for (size_t i = 0; i < n; ++i) {
      const T x = (2.0 * i - (n - 1)) / (n - 1);
      w.push_back(bessel_i0(beta * sqrt(1.0 - x * x)) / bessel_i0(beta));
    }
  }
  return w;
}

inline static size_t bitreverse(size_t n, size_t size) noexcept {
  size_t ri = 0;
  while (size != 1) {
    ri *= 2;
    ri |= (n & 1);
    n >>= 1;
    size /= 2;
  }
  return ri;
}

template <typename T>
inline static void fft_radix2(std::complex<T>* z, size_t size) noexcept {
  size_t num_subffts = size / 2;
  size_t size_subfft = 2;
  std::complex<T> ww[size / 2];

  for (size_t i = 0; i < size / 2; ++i) {
    ww[i] = std::exp(static_cast<T>(-2.0) * static_cast<T>(M_PI)
        * std::complex<T>(0.0, 1.0) * static_cast<T>(i) / static_cast<T>(size));
  }

  for (size_t i = 0; i < size; ++i) {
    const size_t ri = bitreverse(i, size);
    if (i < ri) {
      const std::complex<T> temp = z[i];
      z[i]                       = z[ri];
      z[ri]                      = temp;
    }
  }

  while (num_subffts != 0) {
    for (size_t i = 0; i < num_subffts; ++i) {
      size_t subfft_offset = size_subfft * i;
      for (size_t j = 0; j < size_subfft / 2; ++j) {
        size_t target1                 = subfft_offset + j;
        size_t target2                 = subfft_offset + j + size_subfft / 2;
        size_t left                    = target1;
        size_t right                   = target2;
        const size_t ww_index          = (j * num_subffts);
        const std::complex<T> w        = ww[ww_index];
        const std::complex<T> zleft    = z[left];
        const std::complex<T> w_zright = w * z[right];
        z[target1]                     = zleft + w_zright;
        z[target2]                     = zleft - w_zright;
      }
    }
    num_subffts /= 2;
    size_subfft *= 2;
  }
}

template <typename T>
inline static void czt(std::complex<T>* z,
    size_t n,
    std::complex<T>* ztrans,
    size_t m,
    std::complex<T> w,
    std::complex<T> a) noexcept {
  size_t fft_size = 1;
  while (fft_size < n + m - 1) { fft_size *= 2; }
  std::complex<T> zz[fft_size];
  for (size_t k = 0; k < fft_size; ++k) {
    if (k < n) {
      auto w1 =
          std::pow(w, static_cast<std::complex<T>>(0.5) * static_cast<T>(k * k))
          / std::pow(a, static_cast<T>(k));
      zz[k] = w1 * z[k];
    } else {
      zz[k] = 0;
    }
  }
  fft_radix2(zz, fft_size);
  std::complex<T> w2[fft_size];
  for (size_t k = 0; k < fft_size; ++k) {
    if (k < n + m - 1) {
      const int kshift = k - (n - 1);
      w2[k]            = std::pow(w,
          static_cast<std::complex<T>>(-0.5) * static_cast<T>(kshift * kshift));
    } else {
      w2[k] = 0;
    }
  }

  fft_radix2(w2, fft_size);
  for (size_t k = 0; k < fft_size; ++k) { zz[k] *= w2[k]; }
  fft_radix2(zz, fft_size);
  for (size_t k = 0; k < fft_size; ++k) { zz[k] /= fft_size; }
  for (size_t k = 1; k < fft_size - k; ++k) {
    const size_t kswap         = fft_size - k;
    const std::complex<T> temp = zz[k];
    zz[k]                      = zz[kswap];
    zz[kswap]                  = temp;
  }

  for (size_t k = 0; k < m; ++k) {
    const std::complex<T> w3 =
        std::pow(w, static_cast<std::complex<T>>(0.5) * static_cast<T>(k * k));
    ztrans[k] = w3 * zz[n - 1 + k];
  }
}

template <typename T>
inline static void czt_fft(std::complex<T>* z, size_t size) noexcept {
  if (size == 0) { return; }
  size_t sz = size;
  while (sz % 2 == 0) { sz /= 2; }
  if (sz == 1) {
    fft_radix2(z, size);
  } else {
    const std::complex<T> w =
        std::exp(static_cast<T>(-2.0) * static_cast<T>(M_PI)
            * std::complex<T>(0.0, 1.0) / static_cast<std::complex<T>>(size));
    const std::complex<T> a = 1;
    czt(z, size, z, size, w, a);
  }
}

template <typename T>
inline static void fft(
    std::complex<T>* z, size_t size, bool inv = false) noexcept {
  czt_fft(z, size);
  if (inv) {
    for (size_t k = 0; k < size; ++k) { z[k] /= size; }
    for (size_t k = 1; k < size - k; ++k) {
      const size_t kswap         = size - k;
      const std::complex<T> temp = z[k];
      z[k]                       = z[kswap];
      z[kswap]                   = temp;
    }
  }
}

template <typename T>
inline static std::vector<std::complex<T>> fft(
    const std::vector<T> x, bool invert = false) {
  auto n = x.size();
  std::complex<T> z[n];
  for (size_t i = 0; i < n; i++) { z[i] = x[i]; }
  fft(z, n, invert);
  std::vector<std::complex<T>> y;
  for (size_t i = 0; i < n; i++) { y.push_back(z[i]); }
  return y;
}

template <typename T>
inline static std::vector<T> chebwin(size_t n, T r) noexcept {
  std::vector<T> w;
  w.reserve(n);
  if (n == 1) {
    w[0] = 1.0;
  } else {
    const size_t order    = n - 1;
    const T amplification = pow(10.0, std::abs(r) / 20.0);
    const T beta          = cosh(acosh(amplification) / order);
    std::complex<T> p[n];
    if (n % 2 != 0) {
      for (size_t i = 0; i < n; ++i) {
        const T x = beta * cos(M_PI * i / n);
        if (x > 1.0) {
          p[i] = cosh(order * acosh(x));
        } else if (x < -1.0) {
          p[i] = cosh(order * acosh(-x));
        } else {
          p[i] = cos(order * acos(x));
        }
      }
      czt_fft(p, n);
      const size_t h = (n - 1) / 2;
      for (size_t i = 0; i < n; ++i) {
        const size_t j = (i <= h) ? (h - i) : (i - h);
        w[i]           = std::real(p[j]);
      }
    } else {
      for (size_t i = 0; i < n; ++i) {
        const T x               = beta * cos(M_PI * i / n);
        const std::complex<T> z = std::exp(M_PI * std::complex<T>(0.0, 1.0)
            * static_cast<T>(i) / static_cast<T>(n));
        if (x > 1) {
          p[i] = z * cosh(order * acosh(x));
        } else if (x < -1) {
          p[i] = -z * cosh(order * acosh(-x));
        } else {
          p[i] = z * cos(order * acos(x));
        }
      }
      czt_fft(p, n);
      const size_t h = n / 2;
      for (size_t i = 0; i < n; ++i) {
        const size_t j = (i < h) ? (h - i) : (i - h + 1);
        w[i]           = std::real(p[j]);
      }
    }
    T maxw = w[0];
    for (size_t i = 1; i < n; ++i) { maxw = std::max(maxw, w[i]); }
    for (size_t i = 0; i < n; ++i) { w[i] /= maxw; }
  }
  return w;
}

template <typename T>
static inline std::vector<T> generateFreqRespFft(T fc, size_t n, T fs) {
  std::vector<T> ft(n, 0.0);
  auto hzPrBin = fs / static_cast<T>(n);
  std::fill(ft.begin(), ft.begin() + std::ceil(fc / hzPrBin), 1);
  std::fill(ft.begin() + std::ceil((fs - fc) / hzPrBin) /* +1 ?*/, ft.end(), 1);
  return ft;
}

template <typename T>
inline static std::vector<T> windowMethod(T fc, size_t n, T fs) {
  auto fResp = generateFreqRespFft(fc, n, fs);
  auto tmp   = fft<T>(fResp);
  std::vector<T> tResp;
  for (auto v : tmp) { tResp.push_back(std::real(v) / static_cast<T>(n)); }
  std::vector<T> firstHalf(tResp.begin(), tResp.begin() + tResp.size() / 2);
  std::vector<T> secondHalf(tResp.begin() + tResp.size() / 2, tResp.end());
  secondHalf.insert(secondHalf.end(), firstHalf.begin(), firstHalf.end());
  auto bFull = secondHalf;
  assert(bFull.size() == n);
  auto hanningWindow = hanning<T>(n);
  for (size_t i = 0; i < n; i++) { bFull[i] *= hanningWindow[i]; }
  return bFull;
}
} // namespace NtFx
