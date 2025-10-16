#include "Instrument.h"

namespace hud
{
    Instrument::Instrument()
        : position_(0.0f, 0.0f),
          size_(100.0f, 100.0f),
          color_(0.0f, 1.0f, 0.4f, 0.95f),
          enabled_(true)
    {
    }

} // namespace hud
