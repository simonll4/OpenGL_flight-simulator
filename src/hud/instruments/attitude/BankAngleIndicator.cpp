#include "BankAngleIndicator.h"
#include <cmath>
#include <string>

namespace hud
{

    BankAngleIndicator::BankAngleIndicator() : Instrument()
    {
        // Base color slightly more turquoise to stand out on the HUD
        color_ = glm::vec4(0.0f, 1.0f, 0.4f, 0.95f);
    }

    void BankAngleIndicator::render(gfx::Renderer2D &renderer, const flight::FlightData &flightData)
    {
        if (!enabled_)
            return;

        // Normalize roll angle to avoid jumps when it exceeds 360/-360 degrees
        float rollAngle = normalizeRoll(flightData.roll);

        // Convert instrument center position from NDC to pixels
        glm::vec2 centerPx = ndcToPixels(0.0f, NDC_CENTER_Y);
        float centerX = centerPx.x;
        float centerY = centerPx.y;

        // Calculate endpoints of the inclined base line based on the viewport
        glm::vec2 leftPx = ndcToPixels(-NDC_LINE_WIDTH * 0.5f,
                                       NDC_CENTER_Y - NDC_LINE_SLOPE * NDC_LINE_WIDTH * 0.5f);
        glm::vec2 rightPx = ndcToPixels(NDC_LINE_WIDTH * 0.5f,
                                        NDC_CENTER_Y + NDC_LINE_SLOPE * NDC_LINE_WIDTH * 0.5f);

        // Draw indicator components
        // Note: We do not draw the base line to keep the HUD clean
        // drawBaseLine(renderer, centerX, centerY);

        drawGraduations(renderer, centerX, centerY,
                        leftPx.x, leftPx.y, rightPx.x, rightPx.y,
                        rollAngle);

        drawNeedle(renderer, centerX, centerY);
    }

    void BankAngleIndicator::drawBaseLine(gfx::Renderer2D &renderer, float centerX, float centerY)
    {
        // Convert line endpoints to pixels
        glm::vec2 leftPx = ndcToPixels(-NDC_LINE_WIDTH * 0.5f,
                                       NDC_CENTER_Y - NDC_LINE_SLOPE * NDC_LINE_WIDTH * 0.5f);
        glm::vec2 rightPx = ndcToPixels(NDC_LINE_WIDTH * 0.5f,
                                        NDC_CENTER_Y + NDC_LINE_SLOPE * NDC_LINE_WIDTH * 0.5f);

        renderer.drawLine(leftPx, rightPx, color_, 1.5f);
    }

    void BankAngleIndicator::drawGraduations(gfx::Renderer2D &renderer, float centerX, float centerY,
                                             float leftX, float leftY, float rightX, float rightY,
                                             float rollAngle)
    {
        // Index of the mark aligned with the central triangle
        int centerLineIndex = static_cast<int>(std::round(rollAngle / DEGREES_PER_LINE));

        // Draw only 5 lines: 2 to the left, 1 central, 2 to the right
        int visibleLines = 0;
        for (int i = centerLineIndex - 2; i <= centerLineIndex + 2; ++i)
        {
            float lineAngle = i * DEGREES_PER_LINE;

            // Calculate angular difference and normalized position (t) on the line
            float angleDiff = lineAngle - rollAngle;
            float t = 0.5f + (angleDiff / DEGREES_PER_LINE) * (NDC_LINE_SPACING / NDC_LINE_WIDTH);

            // Only draw lines within the visible range
            if (t >= 0.0f && t <= 1.0f)
            {
                // Interpolate position on the inclined line
                float lineX = leftX + t * (rightX - leftX);
                float lineY = leftY + t * (rightY - leftY);

                // Determine mark height (major every 30 degrees, minor every 10 degrees)
                bool isMajor = (i % 3 == 0);
                float markHeightNDC = isMajor ? NDC_MARK_HEIGHT_MAJOR : NDC_MARK_HEIGHT_MINOR;

                // Convert height from NDC to pixels (dimension only, not position)
                float markHeightPx = markHeightNDC * size_.y * 0.5f;

                // Draw vertical graduation line
                glm::vec2 markTop(lineX, lineY - markHeightPx * 0.5f);
                glm::vec2 markBot(lineX, lineY + markHeightPx * 0.5f);
                renderer.drawLine(markTop, markBot, color_, 1.5f);

                // Show text every 20 degrees (multiples of 2 lines of 10 degrees), excluding 0 degrees
                if (i % 2 == 0 && lineAngle != 0.0f)
                {
                    float textOffsetNDC = 0.035f;
                    float textOffsetPx = textOffsetNDC * size_.y * 0.5f;
                    glm::vec2 textPos(lineX, lineY + markHeightPx * 0.5f + textOffsetPx);

                    // Render integer value of the angle using the global Renderer2D
                    // Normalize the display value to be in [-180, 180]
                    int displayValue = static_cast<int>(lineAngle);
                    while (displayValue > 180) displayValue -= 360;
                    while (displayValue < -180) displayValue += 360;

                    gfx::TextRenderer::drawString(
                        renderer,
                        std::to_string(displayValue),
                        textPos,
                        glm::vec2(DIGIT_WIDTH, DIGIT_HEIGHT),
                        color_,
                        10.0f);
                }

                visibleLines++;
                if (visibleLines >= 5)
                    break;
            }
        }
    }

    void BankAngleIndicator::drawNeedle(gfx::Renderer2D &renderer, float centerX, float centerY)
    {
        // Place the needle slightly below the center
        float needleOffsetPx = NDC_NEEDLE_OFFSET * size_.y * 0.5f;
        float needleY = centerY - needleOffsetPx;

        // Triangle size in pixels
        float triangleSizePx = NDC_TRIANGLE_SIZE * size_.y * 0.5f;

        // Triangle vertices pointing upwards
        glm::vec2 tip(centerX, needleY + triangleSizePx);                                      // Tip
        glm::vec2 baseLeft(centerX - triangleSizePx * 0.6f, needleY - triangleSizePx * 0.3f);  // Left base
        glm::vec2 baseRight(centerX + triangleSizePx * 0.6f, needleY - triangleSizePx * 0.3f); // Right base

        // Brighter color for the needle
        glm::vec4 needleColor = glm::vec4(0.0f, 1.0f, 0.2f, 1.0f);

        // Draw triangle (outline)
        renderer.drawTriangle(tip, baseLeft, baseRight, needleColor, false);
    }

    glm::vec2 BankAngleIndicator::ndcToPixels(float ndcX, float ndcY) const
    {
        // NDC: -1.0 to 1.0, origin at center
        // Pixels: 0 to width/height, origin at top-left corner

        float pixelX = position_.x + (ndcX + 1.0f) * size_.x * 0.5f;
        float pixelY = position_.y + (1.0f - ndcY) * size_.y * 0.5f; // Invert Y

        return glm::vec2(pixelX, pixelY);
    }

    float BankAngleIndicator::normalizeRoll(float roll) const
    {
        // Normalize to range -180 to 180 degrees
        float normalized = roll;
        while (normalized > 180.0f)
            normalized -= 360.0f;
        while (normalized < -180.0f)
            normalized += 360.0f;
        return normalized;
    }

} // namespace hud
