/**
 * @file BankAngleIndicator.h
 * @brief Bank angle (roll) indicator with moving graduations.
 */

#pragma once
#include "../../core/Instrument.h"
#include "../../../gfx/rendering/Renderer2D.h"
#include "../../../gfx/rendering/TextRenderer.h"
#include "../../../flight/data/FlightData.h"
#include <glm/glm.hpp>

namespace hud
{
    /**
     * @class BankAngleIndicator
     * @brief Bank angle (roll) indicator.
     *
     * Displays the aircraft's lateral inclination angle using:
     * - Fixed inclined base line (slight upward slope).
     * - Fixed indicator triangle (needle) at the center.
     * - 5 moving graduated marks (every 10 degrees).
     * - Numbers every 20 degrees for reference.
     *
     * Based on UI::BankAngleIndicator from the computacion_grafica/simulador project,
     * adapted to use Renderer2D and pixel coordinates.
     */
    class BankAngleIndicator : public Instrument
    {
    public:
        BankAngleIndicator();
        ~BankAngleIndicator() = default;

        /**
         * @brief Renders the bank angle indicator.
         * @param renderer Shared Renderer2D.
         * @param flightData Flight data (uses flightData.roll).
         */
        void render(gfx::Renderer2D &renderer, const flight::FlightData &flightData) override;

    private:
        // ====================================================================
        // CONFIGURATION CONSTANTS
        // ====================================================================

        // Center line of the semicircle in NDC (upper HUD area)
        static constexpr float NDC_CENTER_Y = -0.85f;

        // Base bar length and fixed slope to give an arc sensation
        static constexpr float NDC_LINE_WIDTH = 0.45f; // Slightly wider
        static constexpr float NDC_LINE_SLOPE = 0.15f; // Steeper slope (15%) for less "flattening"

        // Separation between marks (10 degrees) and differentiated heights for major/minor
        static constexpr float NDC_LINE_SPACING = 0.05f; // Slightly more space
        static constexpr float DEGREES_PER_LINE = 10.0f;
        static constexpr float NDC_MARK_HEIGHT_MAJOR = 0.06f; // Taller marks
        static constexpr float NDC_MARK_HEIGHT_MINOR = 0.04f; // More visible secondary marks

        // Indicator triangle (needle)
        static constexpr float NDC_NEEDLE_OFFSET = 0.03f;
        static constexpr float NDC_TRIANGLE_SIZE = 0.020f;

        // Size in pixels for rendering +/-20 degree numbers
        static constexpr float DIGIT_WIDTH = 8.0f;
        static constexpr float DIGIT_HEIGHT = 12.0f;

        // ====================================================================
        // PRIVATE DRAWING METHODS
        // ====================================================================

        /**
         * @brief Draws the fixed inclined base line.
         *
         * The bar acts as a slightly inclined static horizon.
         * It is left disabled by default for a cleaner HUD.
         */
        void drawBaseLine(gfx::Renderer2D &renderer, float centerX, float centerY);

        /**
         * @brief Draws the 5 moving graduated marks and their numbers.
         *
         * Calculates the relative position of each mark with respect to the current roll
         * and limits visibility to two marks per side to reduce noise.
         */
        void drawGraduations(gfx::Renderer2D &renderer, float centerX, float centerY,
                             float leftX, float leftY, float rightX, float rightY,
                             float rollAngle);

        /**
         * @brief Draws the fixed indicator triangle (needle).
         *
         * Highlights the static reference point with a brighter color.
         */
        void drawNeedle(gfx::Renderer2D &renderer, float centerX, float centerY);

        /**
         * @brief Converts NDC coordinates to pixels within the assigned viewport.
         * @param ndcX X coordinate in NDC (-1.0 to 1.0).
         * @param ndcY Y coordinate in NDC (-1.0 to 1.0).
         * @return Coordinate in pixels relative to position_/size_.
         *
         * Leverages the box assigned to the instrument to scale values
         * -1..1 to screen offsets respecting the top-left origin.
         */
        glm::vec2 ndcToPixels(float ndcX, float ndcY) const;

        /**
         * @brief Normalizes roll angle to range -180 to 180 degrees.
         *
         * Prevents sudden jumps when the simulator reports values > 360 degrees.
         */
        float normalizeRoll(float roll) const;
    };

} // namespace hud
