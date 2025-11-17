#include "Instrument.h"

namespace hud
{
    Instrument::Instrument()
        : position_(0.0f, 0.0f),
          size_(100.0f, 100.0f),
          color_(0.0f, 1.0f, 0.4f, 0.95f),
          enabled_(true)
    {
        // Valores por defecto razonables para que un instrumento recién creado
        // pueda renderizarse incluso antes de que FlightHUD le asigne layout:
        // - posición (0,0) evita lecturas de memoria inválidas
        // - tamaño 100x100 proporciona un área mínima visible
        // - color HUD verde mantiene consistencia visual
        // - enabled_ true permite que se muestre sin pasos adicionales
    }

} // namespace hud
