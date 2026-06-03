#include "lib/Audio.h"
#include "lib/Component.h"
#include "lib/utils.h"

namespace NtFx {
namespace Generator {
  struct Noise : public ComponentBase<Audio> {
    virtual Audio process(Audio x) noexcept override {
      return { rand<signal_t>(), rand<signal_t>() };
    }
  };
  // TODO: Sine, square, saw and so on.
}
}