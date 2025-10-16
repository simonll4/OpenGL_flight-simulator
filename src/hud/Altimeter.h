#pragma once
#include "Instrument.h"

namespace hud
{
    /**
     * @class Altimeter
     * @brief Instrumento que muestra la altitud del avión
     *
     * Hereda de Instrument y proporciona:
     * - Tape vertical con escala de altitud móvil
     * - Caja de lectura digital con display de 7 segmentos
     * - Indicador chevron para referencia visual
     */
    class Altimeter : public Instrument
    {
    public:
        Altimeter();

        /**
         * @brief Renderiza el altímetro con los datos de vuelo actuales
         * @param renderer Renderer 2D compartido
         * @param flightData Datos del vuelo (especialmente altitude)
         */
        void render(gfx::Renderer2D &renderer, const flight::FlightData &flightData) override;

    private:
        // Métodos específicos del altímetro
        void drawBackground(gfx::Renderer2D &renderer);
        void drawAltitudeTape(gfx::Renderer2D &renderer, float altitude);
        void drawCurrentAltitudeBox(gfx::Renderer2D &renderer, float altitude);
    };


} // namespace hud
