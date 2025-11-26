/**
 * @file SpeedIndicator.h
 * @brief Airspeed indicator with vertical tape display.
 */

#pragma once
#include "../../core/Instrument.h"

namespace hud
{
    /**
     * @class SpeedIndicator
     * @brief Left vertical tape showing indicated airspeed (IAS).
     *
     * Replicates altimeter ergonomics: moving scale, centered digital readout,
     * and connecting chevron. The scale marks every 10 knots and numbers every 20
     * to avoid visual clutter.
     */
    class SpeedIndicator : public Instrument
    {
    public:
        SpeedIndicator();

        /**
         * @brief Renders the speed indicator.
         * @param renderer Shared 2D renderer.
         * @param flightData Flight data (especially airspeed).
         */
        void render(gfx::Renderer2D &renderer, const flight::FlightData &flightData) override;

    private:
        void drawSpeedTape(gfx::Renderer2D &renderer, float airspeed);
        void drawCurrentSpeedBox(gfx::Renderer2D &renderer, float airspeed);
        void drawSpeedNumber(gfx::Renderer2D &renderer, int speed, const glm::vec2 &position);
    };

} // namespace hud
