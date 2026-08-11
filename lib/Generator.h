#include "lib/Audio.h"
#include "lib/Component.h"
#include "lib/utils.h"

namespace NtFx {
namespace Generator {
  struct Noise final : public ComponentBase<Audio> {
    Audio process(Audio) noexcept override {
      return { rand<signal_t>(), rand<signal_t>() };
    }
  };
}
}