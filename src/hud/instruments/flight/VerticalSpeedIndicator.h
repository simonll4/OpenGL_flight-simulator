#pragma once
#include "../../core/Instrument.h"

namespace hud
{
    /**
     * @class VerticalSpeedIndicator
     * @brief Instrumento compacto que muestra la velocidad vertical (VSI)
     *
     * UNIDADES Y FORMATO (ESTÁNDAR MILITAR):
     * - Entrada: ft/min (pies por minuto) desde FlightData
     * - Display: ft/min dividido por 100 (formato compacto estándar)
     * - Ejemplo: 4000 ft/min → muestra "+40"
     * - Ejemplo: -1200 ft/min → muestra "-12"
     *
     * Diseño:
     * - Escala vertical FIJA (no scrolling) de ±60 (representa ±6000 ft/min)
     * - Indicador triangular móvil que señala el valor actual
     * - Número digital centrado con formato +/-XX (entero, sin decimales)
     * - Marcas cada 10 unidades (1000 ft/min)
     *
     * Ubicación típica en HUD militar:
     * - Entre el FPV (centro) y el altímetro (derecha)
     * - Altura ~50% de los tapes principales
     * - No obstruye el horizonte sintético
     *
     * CONSISTENCIA CON OTROS INSTRUMENTOS:
     * - Airspeed: kt (nudos) - formato entero directo
     * - Altitude: ft (pies) - formato entero directo
     * - VSI: ft/min/100 - formato entero compacto (/100 es estándar militar)
     */
    class VerticalSpeedIndicator : public Instrument
    {
    public:
        VerticalSpeedIndicator();

        /**
         * @brief Renderiza el VSI con los datos de vuelo actuales
         * @param renderer Renderer 2D compartido
         * @param flightData Datos del vuelo (especialmente verticalSpeed en ft/min)
         */
        void render(gfx::Renderer2D &renderer, const flight::FlightData &flightData) override;

    private:
        // Métodos de renderizado
        void drawScale(gfx::Renderer2D &renderer);
        void drawIndicator(gfx::Renderer2D &renderer, float verticalSpeed);
        void drawDigitalReadout(gfx::Renderer2D &renderer, float verticalSpeed);

        // Configuración de la escala (valores en ft/min)
        static constexpr float MAX_VSI = 6000.0f;       // ft/min (se muestra como 60)
        static constexpr float MIN_VSI = -6000.0f;      // ft/min (se muestra como -60)
        static constexpr float MARK_INTERVAL = 1000.0f; // Marcas cada 1000 ft/min (10 en display)
        static constexpr float DISPLAY_SCALE = 100.0f;  // Divisor para display compacto
    };

} // namespace hud
