#pragma once
#include "Instrument.h"

namespace hud
{
    /**
     * @class SpeedIndicator
     * @brief Indicador de velocidad del aire - tape vertical izquierdo
     * 
     * EJEMPLO de cómo heredar de Instrument y crear un nuevo instrumento.
     * Este instrumento muestra la velocidad del aire similar al altímetro.
     * 
     * Hereda de Instrument y proporciona:
     * - Tape vertical con escala de velocidad móvil
     * - Caja de lectura digital central
     * - Marcas cada 10 nudos con números cada 20 nudos
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
        // Métodos específicos del indicador de velocidad
        void drawSpeedTape(gfx::Renderer2D &renderer, float airspeed);
        void drawCurrentSpeedBox(gfx::Renderer2D &renderer, float airspeed);
        void drawSpeedNumber(gfx::Renderer2D &renderer, int speed, const glm::vec2 &position);
    };

} // namespace hud
