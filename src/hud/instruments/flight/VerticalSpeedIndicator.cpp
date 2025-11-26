#include "VerticalSpeedIndicator.h"
#include "../../../gfx/rendering/TextRenderer.h"
#include <cmath>
#include <algorithm>

namespace hud
{
    // ============================================================================
    // VISUAL CONFIGURATION
    // ============================================================================

    // Scale dimensions (compact and professional)
    static const float SCALE_WIDTH = 54.0f;       // Vertical scale width
    static const float SCALE_HEIGHT = 225.0f;     // Total scale height (50% of tapes)
    static const float TICK_LENGTH = 8.0f;        // Length of lateral marks (reduced)
    static const float MAJOR_TICK_LENGTH = 12.0f; // Major marks (0, +/-20, +/-40, +/-60)

    // Triangular indicator (compact)
    static const float INDICATOR_WIDTH = 10.0f;
    static const float INDICATOR_HEIGHT = 8.0f;

    // Digital readout box (consistent with other instruments)
    static const float READOUT_BOX_WIDTH = 48.0f; // Narrower (shorter numbers)
    static const float READOUT_BOX_HEIGHT = 24.0f;

    VerticalSpeedIndicator::VerticalSpeedIndicator() : Instrument()
    {
        // VSI specific configuration
        size_ = glm::vec2(SCALE_WIDTH, SCALE_HEIGHT);
        color_ = glm::vec4(0.0f, 1.0f, 0.4f, 0.95f); // HUD Green
    }

    // ============================================================================
    // MAIN RENDER FUNCTION
    // ============================================================================

    void VerticalSpeedIndicator::render(gfx::Renderer2D &renderer, const flight::FlightData &flightData)
    {
        if (!enabled_)
            return;

        float verticalSpeed = flightData.verticalSpeed; // ft/min

        // Clamping to prevent indicator from leaving the scale
        // (extreme values shown at limit)
        float clampedVS = std::clamp(verticalSpeed, MIN_VSI, MAX_VSI);

        drawScale(renderer);
        drawIndicator(renderer, clampedVS);
        drawDigitalReadout(renderer, verticalSpeed); // Show real value (not clamped)
    }

    // ============================================================================
    // FIXED SCALE RENDERING
    // ============================================================================

    void VerticalSpeedIndicator::drawScale(gfx::Renderer2D &renderer)
    {
        float centerX = position_.x + size_.x * 0.5f;
        float centerY = position_.y + size_.y * 0.5f;

        // Scale vertical line (backbone)
        float lineX = centerX + 5.0f; // Offset to right of center
        renderer.drawLine(
            glm::vec2(lineX, position_.y),
            glm::vec2(lineX, position_.y + size_.y),
            color_, 1.0f);

        // Draw scale marks
        int numMarks = static_cast<int>((MAX_VSI - MIN_VSI) / MARK_INTERVAL) + 1;

        for (int i = 0; i < numMarks; ++i)
        {
            float vsi = MIN_VSI + i * MARK_INTERVAL; // In ft/min

            // Calculate mark Y position (inverted: +up, -down)
            float normalizedVSI = vsi / MAX_VSI; // -1.0 to +1.0
            float markY = centerY - normalizedVSI * (SCALE_HEIGHT * 0.5f);

            // Determine if major mark (every 2000 ft/min = 20 on display)
            bool isMajorMark = (std::fmod(std::abs(vsi), 2000.0f) < 0.1f);

            float tickLen = isMajorMark ? MAJOR_TICK_LENGTH : TICK_LENGTH;

            // Draw horizontal tick
            renderer.drawLine(
                glm::vec2(lineX - tickLen, markY),
                glm::vec2(lineX, markY),
                color_, isMajorMark ? 1.25f : 0.8f);

            // Draw numbers only on major marks
            if (isMajorMark)
            {
                // Format number in military standard: ft/min / 100
                // Example: 4000 ft/min -> "+40"
                int displayValue = static_cast<int>(vsi / DISPLAY_SCALE);

                std::string label;
                if (displayValue == 0)
                {
                    label = "0";
                }
                else
                {
                    // Add explicit sign
                    label = (displayValue > 0 ? "+" : "") + std::to_string(displayValue);
                }

                // Text position (left of ticks)
                float textX = lineX - tickLen - 18.0f;
                glm::vec2 textPos = glm::vec2(textX, markY);
                gfx::TextRenderer::drawString(renderer, label, textPos,
                                              glm::vec2(5.0f, 8.0f), color_, 7.0f);
            }
        }

        // Reference line at 0 (thicker and longer)
        renderer.drawLine(
            glm::vec2(lineX - (MAJOR_TICK_LENGTH + 12.0f), centerY),
            glm::vec2(lineX + 6.0f, centerY),
            color_, 3.0f);
    }

    // ============================================================================
    // MOVING TRIANGULAR INDICATOR
    // ============================================================================

    void VerticalSpeedIndicator::drawIndicator(gfx::Renderer2D &renderer, float verticalSpeed)
    {
        float centerX = position_.x + size_.x * 0.5f;
        float centerY = position_.y + size_.y * 0.5f;

        // Calculate indicator Y position based on VSI
        float normalizedVSI = verticalSpeed / MAX_VSI; // -1.0 to +1.0
        float indicatorY = centerY - normalizedVSI * (SCALE_HEIGHT * 0.5f);

        // Triangle pointing to scale (tip to right)
        float lineX = centerX + 5.0f;
        float triLeft = lineX + 2.0f;
        float triRight = triLeft + INDICATOR_WIDTH;
        float triTop = indicatorY - INDICATOR_HEIGHT * 0.5f;
        float triBot = indicatorY + INDICATOR_HEIGHT * 0.5f;

        // Draw filled triangle
        renderer.drawTriangle(
            glm::vec2(triLeft, triTop),
            glm::vec2(triLeft, triBot),
            glm::vec2(triRight, indicatorY),
            color_);

        // Triangle outline (thinner)
        renderer.drawLine(glm::vec2(triLeft, triTop), glm::vec2(triRight, indicatorY), color_, 1.2f);
        renderer.drawLine(glm::vec2(triRight, indicatorY), glm::vec2(triLeft, triBot), color_, 1.2f);
        renderer.drawLine(glm::vec2(triLeft, triBot), glm::vec2(triLeft, triTop), color_, 1.2f);
    }

    // ============================================================================
    // DIGITAL READOUT (MILITARY STANDARD FORMAT)
    // ============================================================================

    void VerticalSpeedIndicator::drawDigitalReadout(gfx::Renderer2D &renderer, float verticalSpeed)
    {
        float centerX = position_.x + size_.x * 0.5f;
        float centerY = position_.y + size_.y * 0.5f;

        // Align digital box with 0 line and LEFT of scale
        float lineX = centerX + 5.0f;                  // Must match drawScale
        float boxX = lineX - 6.0f - READOUT_BOX_WIDTH; // Minimum separation from scale
        float boxY = centerY - READOUT_BOX_HEIGHT * 0.5f;

        // Semi-transparent background for readability
        glm::vec4 boxBgColor = glm::vec4(0.0f, 0.0f, 0.0f, 0.7f);
        renderer.drawRect(
            glm::vec2(boxX, boxY),
            glm::vec2(READOUT_BOX_WIDTH, READOUT_BOX_HEIGHT),
            boxBgColor, true);

        // Box outline
        renderer.drawRect(
            glm::vec2(boxX, boxY),
            glm::vec2(READOUT_BOX_WIDTH, READOUT_BOX_HEIGHT),
            color_, false);

        // MILITARY STANDARD FORMAT: ft/min / 100
        // Example: 4000 ft/min -> "+40"
        // Example: -1200 ft/min -> "-12"
        // Example: +500 ft/min -> "+5"
        int displayValue = static_cast<int>(std::round(verticalSpeed / DISPLAY_SCALE));

        std::string vsiText;
        if (displayValue == 0)
        {
            vsiText = "0";
        }
        else
        {
            // Explicit sign + absolute value (no decimals, compact integer format)
            vsiText = (displayValue > 0 ? "+" : "") + std::to_string(displayValue);
        }

        // Draw centered number in box (consistent with other instruments)
        glm::vec2 textPos = glm::vec2(boxX + READOUT_BOX_WIDTH * 0.5f, centerY);
        gfx::TextRenderer::drawString(renderer, vsiText, textPos,
                                      glm::vec2(6.0f, 10.0f), color_, 8.0f);
    }

} // namespace hud
