/**
 * @file WaypointIndicator.h
 * @brief HSI (Horizontal Situation Indicator) for waypoint navigation.
 */

#pragma once
#include "../../core/Instrument.h"

namespace hud
{
    /**
     * @class WaypointIndicator
     * @brief HSI (Horizontal Situation Indicator) navigation display.
     *
     * Implementation of a navigation indicator inspired by Garmin HSI.
     * Provides comprehensive visual information for waypoint navigation using:
     *
     * - Fixed compass rose with marks every 5 and 10 degrees.
     * - Labels at cardinal points (N, E, S, W).
     * - Heading numbers every 30 degrees (000, 030, 060, etc.).
     * - Magenta arrow indicating direction to the waypoint.
     * - Vertical indicator for altitude difference.
     *
     * The HSI combines compass functionality with a navigation indicator,
     * allowing the pilot to simultaneously visualize current heading and
     * direction to the target waypoint without mental calculations.
     */
    class WaypointIndicator : public Instrument
    {
    public:
        WaypointIndicator();

        /**
         * @brief Renders the HSI indicator with current flight data.
         * @param renderer Shared 2D renderer.
         * @param flightData Flight data (includes position, heading, active waypoint).
         */
        void render(gfx::Renderer2D &renderer, const flight::FlightData &flightData) override;

    private:
        /**
         * @struct NavSnapshot
         * @brief Snapshot of processed navigation data.
         *
         * Structure containing all calculated navigation data at a given moment,
         * facilitating data passing between render methods and improving code readability.
         */
        struct NavSnapshot
        {
            float heading = 0.0f;            ///< Aircraft current heading (0-360 degrees).
            float relativeAngle = 0.0f;      ///< Angle relative to waypoint (-180 to +180 degrees).
            float altitudeDifference = 0.0f; ///< Altitude difference with waypoint (meters).
        };

        /**
         * @brief Builds a snapshot with current navigation data.
         * @param flightData Simulator flight data.
         * @return NavSnapshot with processed data ready for rendering.
         */
        NavSnapshot buildNavSnapshot(const flight::FlightData &flightData) const;

        // Component render methods
        /// Draws the compass rose, marks, and magenta arrow.
        void drawCompassRose(gfx::Renderer2D &renderer, const NavSnapshot &nav);
        /// Renders the altitude difference indicator (UP/DN/LVL).
        void drawVerticalIndicator(gfx::Renderer2D &renderer, const NavSnapshot &nav);
    };

} // namespace hud
