/**
 * @file VerticalSpeedIndicator.h
 * @brief Vertical speed indicator (VSI) with compact display format.
 */

#pragma once
#include "../../core/Instrument.h"

namespace hud
{
    /**
     * @class VerticalSpeedIndicator
     * @brief Compact instrument showing vertical speed (VSI).
     *
     * UNITS AND FORMAT (MILITARY STANDARD):
     * - Input: ft/min (feet per minute) from FlightData.
     * - Display: ft/min divided by 100 (standard compact format).
     * - Example: 4000 ft/min -> shows "+40".
     * - Example: -1200 ft/min -> shows "-12".
     *
     * Design:
     * - FIXED vertical scale (no scrolling) of +/- 60 (represents +/- 6000 ft/min).
     * - Moving triangular indicator pointing to current value.
     * - Centered digital number with +/-XX format (integer, no decimals).
     * - Marks every 10 units (1000 ft/min).
     *
     * Typical location in military HUD:
     * - Between FPV (center) and altimeter (right).
     * - Height ~50% of main tapes.
     * - Does not obstruct synthetic horizon.
     *
     * CONSISTENCY WITH OTHER INSTRUMENTS:
     * - Airspeed: kt (knots) - direct integer format.
     * - Altitude: ft (feet) - direct integer format.
     * - VSI: ft/min/100 - compact integer format (/100 is military standard).
     */
    class VerticalSpeedIndicator : public Instrument
    {
    public:
        VerticalSpeedIndicator();

        /**
         * @brief Renders the VSI with current flight data.
         * @param renderer Shared 2D renderer.
         * @param flightData Flight data (especially verticalSpeed in ft/min).
         */
        void render(gfx::Renderer2D &renderer, const flight::FlightData &flightData) override;

    private:
        // Render methods
        void drawScale(gfx::Renderer2D &renderer);
        void drawIndicator(gfx::Renderer2D &renderer, float verticalSpeed);
        void drawDigitalReadout(gfx::Renderer2D &renderer, float verticalSpeed);

        // Scale configuration (values in ft/min)
        static constexpr float MAX_VSI = 6000.0f;       // ft/min (shows as 60)
        static constexpr float MIN_VSI = -6000.0f;      // ft/min (shows as -60)
        static constexpr float MARK_INTERVAL = 1000.0f; // Marks every 1000 ft/min (10 on display)
        static constexpr float DISPLAY_SCALE = 100.0f;  // Divisor for compact display
    };

} // namespace hud
