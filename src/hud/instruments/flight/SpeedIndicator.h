#pragma once
#include "../../core/Instrument.h"

namespace hud
{
    /**
     * @class SpeedIndicator
     * @brief Tape vertical izquierdo que muestra velocidad indicada (IAS).
     *
     * Replica la ergonomía del altímetro: escala móvil, lectura digital centrada
     * y chevron conector. La escala marca cada 10 nudos y enumera cada 20 para
     * evitar saturación visual.
     */
    class SpeedIndicator : public Instrument
    {
    public:
        SpeedIndicator();

        /**
         * @brief Renderiza el indicador de velocidad
         * @param renderer Renderer 2D compartido
         * @param flightData Datos del vuelo (especialmente airspeed)
         */
        void render(gfx::Renderer2D &renderer, const flight::FlightData &flightData) override;

    private:
        void drawSpeedTape(gfx::Renderer2D &renderer, float airspeed);
        void drawCurrentSpeedBox(gfx::Renderer2D &renderer, float airspeed);
        void drawSpeedNumber(gfx::Renderer2D &renderer, int speed, const glm::vec2 &position);
    };

} // namespace hud
