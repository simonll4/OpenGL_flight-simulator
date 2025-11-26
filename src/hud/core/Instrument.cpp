#include "Instrument.h"

namespace hud
{
    Instrument::Instrument()
        : position_(0.0f, 0.0f),
          size_(100.0f, 100.0f),
          color_(0.0f, 1.0f, 0.4f, 0.95f),
          enabled_(true)
    {
        // Reasonable default values so a newly created instrument
        // can render even before FlightHUD assigns it a layout:
        // - position (0,0) avoids invalid memory reads
        // - size 100x100 provides a minimum visible area
        // - HUD green color maintains visual consistency
        // - enabled_ true allows it to be shown without extra steps
    }

} // namespace hud
