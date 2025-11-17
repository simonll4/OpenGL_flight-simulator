#pragma once
#include "../../core/Instrument.h"

namespace hud
{
    /**
     * @class Altimeter
     * @brief Tape vertical + display digital para la altitud barométrica.
     *
     * Extiende `Instrument` para dibujar:
     *  - Una escala móvil con marcas cada 100 ft.
     *  - Una caja central con lectura numérica en 7 segmentos.
     *  - Un chevron que conecta el tape con la lectura digital.
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
