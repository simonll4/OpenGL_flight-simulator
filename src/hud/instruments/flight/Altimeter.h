/**
 * @file Altimeter.h
 * @brief Altimeter instrument with vertical tape and digital display.
 */

#pragma once
#include "../../core/Instrument.h"

namespace hud
{
    /**
     * @class Altimeter
     * @brief Vertical tape + digital display for barometric altitude.
     *
     * Extends `Instrument` to draw:
     *  - A moving scale with marks every 100 ft.
     *  - A central box with a 7-segment numeric readout.
     *  - A chevron connecting the tape to the digital readout.
     */
    class Altimeter : public Instrument
    {
    public:
        Altimeter();

        /**
         * @brief Renders the altimeter with current flight data.
         * @param renderer Shared 2D renderer.
         * @param flightData Flight data (especially altitude).
         */
        void render(gfx::Renderer2D &renderer, const flight::FlightData &flightData) override;

    private:
        // Altimeter-specific methods
        void drawBackground(gfx::Renderer2D &renderer);
        void drawAltitudeTape(gfx::Renderer2D &renderer, float altitude);
        void drawCurrentAltitudeBox(gfx::Renderer2D &renderer, float altitude);
    };

} // namespace hud
