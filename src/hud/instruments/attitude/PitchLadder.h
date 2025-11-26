/**
 * @file PitchLadder.h
 * @brief Pitch ladder indicator with dynamic horizontal lines.
 */

#pragma once
#include "../../core/Instrument.h"
#include "../../../gfx/rendering/Renderer2D.h"
#include "../../../flight/data/FlightData.h"
#include <glm/glm.hpp>

namespace hud
{
    /**
     * @class PitchLadder
     * @brief Pitch ladder indicator.
     *
     * Displays the aircraft's pitch angle using:
     * - Fixed central crosshair (circle + lateral lines).
     * - 5 dynamic horizontal lines (every 10 degrees).
     * - Lines cut in the center with a gap.
     * - Vertical markers at the ends (up = +pitch, down = -pitch).
     *
     * Based on UI::PitchLadder from the computacion_grafica/simulador project,
     * adapted to use Renderer2D and pixel coordinates.
     */
    class PitchLadder : public Instrument
    {
    public:
        PitchLadder();
        ~PitchLadder() = default;

        /**
         * @brief Renders the pitch ladder.
         * @param renderer Shared Renderer2D.
         * @param flightData Flight data (uses flightData.pitch).
         */
        void render(gfx::Renderer2D &renderer, const flight::FlightData &flightData) override;

    private:
        // ====================================================================
        // CONFIGURATION CONSTANTS
        // ====================================================================

        // Pitch line configuration
        static constexpr float PITCH_STEP = 10.0f;        // Each line represents 10 degrees
        static constexpr float MAX_PITCH_DISPLAY = 40.0f; // Show up to +/- 40 degrees

        // Central crosshair (circle + lateral lines) in NDC
        static constexpr float NDC_CIRCLE_RADIUS = 0.01f;       // Radius of the aimed circle
        static constexpr float NDC_LATERAL_LINE_LENGTH = 0.04f; // Extension of guide lines
        static constexpr int CIRCLE_SEGMENTS = 16;              // Segments to approximate the circle

        // Pitch lines in NDC
        static constexpr float NDC_LINE_WIDTH_NORMAL = 0.1f; // Length for +/- 10, +/- 20, etc.
        static constexpr float NDC_LINE_WIDTH_ZERO = 0.15f;  // 0 degree line is longer
        static constexpr float NDC_GAP = 0.03f;              // Separation in the center
        static constexpr float NDC_MARKER_SIZE = 0.02f;      // End markers

        // Conversion factor: 1% NDC per degree of pitch
        static constexpr float NDC_PER_DEGREE = 0.01f;

        // Visibility limit in NDC (+/- 0.8)
        static constexpr float NDC_VISIBILITY_LIMIT = 0.8f;

        // ====================================================================
        // PRIVATE DRAWING METHODS
        // ====================================================================

        /**
         * @brief Draws the fixed central crosshair (circle + lateral lines).
         *
         * This crosshair serves as an absolute reference; it does not depend on pitch.
         */
        void drawCrosshair(gfx::Renderer2D &renderer, float centerX, float centerY);

        /**
         * @brief Draws the 5 dynamic pitch lines.
         *
         * Calculates visible lines around the current pitch and limits
         * output to +/- 2 divisions to avoid cluttering the screen.
         */
        void drawPitchLines(gfx::Renderer2D &renderer, float centerX, float centerY, float pitchAngle, float rollAngle);

        /**
         * @brief Draws a single pitch line.
         * @param pitchLineAngle Angle this line represents (e.g., 10, 20, etc.).
         * @param currentPitch Current aircraft pitch.
         * @param rollAngle Current roll angle for rotation.
         *
         * Responsible for positioning, clipping with central gap, and placing
         * vertical markers that distinguish between positive/negative pitch.
         */
        void drawSinglePitchLine(gfx::Renderer2D &renderer, float centerX, float centerY,
                                 float pitchLineAngle, float currentPitch, float rollAngle);

        /**
         * @brief Converts NDC dimension to pixels (without Y inversion).
         *
         * Uses viewport size on each axis to scale relative magnitudes
         * (-1..1) to concrete screen lengths.
         */
        float ndcDimensionToPixels(float ndcDim, float axisLength) const;
    };

} // namespace hud
