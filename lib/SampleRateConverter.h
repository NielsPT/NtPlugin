#pragma once

#include "Plugin.h"
#include "Stereo.h"
#include "lib/windowFunctions.h"
#include <algorithm>
#include <array>
#include <cstddef>

namespace NtFx {
namespace Src {
  constexpr int nDelayLine            = 192;
  constexpr int oversamplingFirMultHq = 24;
  constexpr int oversamplingFirMultLq = 12;
  constexpr int fc                    = 22e3;
  enum oversamplingMode : int {
    disable = 1,
    // iir_2x,
    // iir_4x,
    // iir_8x,
    fir_2x_lq,
    fir_4x_lq,
    fir_8x_lq,
    fir_2x_hq,
    fir_4x_hq,
    fir_8x_hq,
    end = fir_8x_hq
  };
  template <typename signal_t>
  struct State {
    size_t iStoreIn;
    size_t iStoreOut;
    std::array<Stereo<signal_t>, nDelayLine * 2> dlInterpolation;
    std::array<Stereo<signal_t>, nDelayLine * 2> dlAntialiasing;
  };
  template <typename signal_t>
  struct Coeffs {
    bool disable        = false;
    size_t osFactor     = 1;
    size_t osFirLenMult = 12;
    size_t n            = 12;
    signal_t fsHi       = 48000;
    std::array<signal_t, nDelayLine> b;
  };
  template <typename signal_t>
  Stereo<signal_t> processSample(NtPlugin<signal_t>& plug,
      State<signal_t>& state,
      const Coeffs<signal_t>& coeffs,
      Stereo<signal_t> x) {
    if (coeffs.disable) { return plug.processSample(x); }
    state.dlInterpolation[state.iStoreIn]              = x;
    state.dlInterpolation[state.iStoreIn + nDelayLine] = x;
    if (++state.iStoreIn >= nDelayLine) { state.iStoreIn = 0; }
    auto iReadIn = state.iStoreIn + nDelayLine;
    for (size_t i = 0; i < coeffs.osFactor; i++) {
      Stereo<signal_t> accum;
      for (size_t j = 0; j < coeffs.osFirLenMult; j++) {
        accum += coeffs.b[j * coeffs.osFactor + i]
            * state.dlInterpolation[iReadIn - j];
      }
      auto xProc                            = accum * coeffs.osFactor;
      auto yProc                            = plug.processSample(xProc);
      state.dlAntialiasing[state.iStoreOut] = yProc;
      state.dlAntialiasing[state.iStoreOut + nDelayLine] = yProc;
      if (++state.iStoreOut >= nDelayLine) { state.iStoreOut = 0; }
    }
    auto iReadOut =
        state.iStoreOut + nDelayLine - coeffs.osFactor * coeffs.osFirLenMult;
    Stereo<signal_t> accum;
    for (size_t i = 0; i < coeffs.osFactor * coeffs.osFirLenMult; i++) {
      accum += coeffs.b[i] * state.dlAntialiasing[iReadOut + i];
    }
    return accum;
  }
  // TODO: All of this only toucheds coeffs. Should we have a class?
  template <typename signal_t>
  static inline void update(
      oversamplingMode mode, signal_t fs, Coeffs<signal_t>& coeffs) {
    switch (mode) {

      // TODO: IIR oversampling
      // case iir_2x:
      //   coeffs.osFactor = 2;
      //   break;
      // case iir_4x:
      //   coeffs.osFactor = 4;
      //   break;
      // case iir_8x:
      //   coeffs.osFactor = 8;
      //   break;
    case fir_2x_lq:
      coeffs.osFactor     = 2;
      coeffs.osFirLenMult = oversamplingFirMultLq;
      coeffs.disable      = false;
      break;
    case fir_4x_lq:
      coeffs.osFactor     = 4;
      coeffs.osFirLenMult = oversamplingFirMultLq;
      coeffs.disable      = false;
      break;
    case fir_8x_lq:
      coeffs.osFactor     = 8;
      coeffs.osFirLenMult = oversamplingFirMultLq;
      coeffs.disable      = false;
      break;
    case fir_2x_hq:
      coeffs.osFactor     = 2;
      coeffs.osFirLenMult = oversamplingFirMultHq;
      coeffs.disable      = false;
      break;
    case fir_4x_hq:
      coeffs.osFactor     = 4;
      coeffs.osFirLenMult = oversamplingFirMultHq;
      coeffs.disable      = false;
      break;
    case fir_8x_hq:
      coeffs.osFactor     = 8;
      coeffs.osFirLenMult = oversamplingFirMultHq;
      coeffs.disable      = false;
      break;
    default:
    case disable:
      coeffs.osFactor     = 1;
      coeffs.osFirLenMult = 1;
      coeffs.disable      = true;
    }
    coeffs.fsHi = fs * coeffs.osFactor;
    coeffs.n    = coeffs.osFactor * coeffs.osFirLenMult;
    // TODO: Does it make sense to replace the coeffs in place instead?
    // These vectors do NOT improve stability anyway,
    if (!coeffs.disable) {
      auto b = windowMethod<signal_t>(fc, coeffs.n, coeffs.fsHi);
      std::fill(coeffs.b.begin(), coeffs.b.end(), 0.0);
      for (size_t i = 0; i < coeffs.n; i++) { coeffs.b[i] = b[i]; }
    }
  }
  template <typename signal_t>
  static inline void reset(State<signal_t>& state) {
    state.iStoreIn  = 0;
    state.iStoreOut = 0;
    std::fill(state.dlAntialiasing.begin(), state.dlAntialiasing.end(), 0.0);
    std::fill(state.dlInterpolation.begin(), state.dlInterpolation.end(), 0.0);
  }
}
}