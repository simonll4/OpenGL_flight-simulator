#include "PitchLadder.h"
#include <cmath>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace hud
{

    PitchLadder::PitchLadder() : Instrument()
    {
        // Use the same translucent green as the rest of the HUD
        color_ = glm::vec4(0.0f, 1.0f, 0.4f, 0.95f);
    }

    void PitchLadder::render(gfx::Renderer2D &renderer, const flight::FlightData &flightData)
    {
        if (!enabled_)
            return;

        // Viewport dimensions
        float viewportWidth = size_.x;
        float viewportHeight = size_.y;

        // Geometric center of the instrument box
        float centerX = position_.x + viewportWidth * 0.5f;
        float centerY = position_.y + viewportHeight * 0.5f;

        // Draw central crosshair (fixed)
        drawCrosshair(renderer, centerX, centerY);

        // Draw pitch lines (dynamic)
        drawPitchLines(renderer, centerX, centerY, flightData.pitch, flightData.roll);
    }

    void PitchLadder::drawCrosshair(gfx::Renderer2D &renderer, float centerX, float centerY)
    {
        // Convert circle radius to pixels
        float radiusPx = ndcDimensionToPixels(NDC_CIRCLE_RADIUS, size_.y);

        // Draw central circle (outline) serving as aimpoint
        renderer.drawCircle(glm::vec2(centerX, centerY), radiusPx, color_, CIRCLE_SEGMENTS, false);

        // Lateral lines extending from the circle (represent local horizon line)
        float lineLengthPx = ndcDimensionToPixels(NDC_LATERAL_LINE_LENGTH, size_.x);

        // Left lateral line
        glm::vec2 leftStart(centerX - radiusPx, centerY);
        glm::vec2 leftEnd(centerX - radiusPx - lineLengthPx, centerY);
        renderer.drawLine(leftStart, leftEnd, color_, 2.0f);

        // Right lateral line
        glm::vec2 rightStart(centerX + radiusPx, centerY);
        glm::vec2 rightEnd(centerX + radiusPx + lineLengthPx, centerY);
        renderer.drawLine(rightStart, rightEnd, color_, 2.0f);
    }

    void PitchLadder::drawPitchLines(gfx::Renderer2D &renderer, float centerX, float centerY, float pitchAngle, float rollAngle)
    {
        // Calculate center line index based on current pitch
        int centerLineIndex = static_cast<int>(std::round(pitchAngle / PITCH_STEP));

        // Draw 5 lines: 2 above, 1 center, 2 below current pitch to maintain clarity
        for (int i = centerLineIndex - 2; i <= centerLineIndex + 2; ++i)
        {
            float pitchLineAngle = i * PITCH_STEP;

            // Only generate if within reasonable visual range
            if (pitchLineAngle >= -MAX_PITCH_DISPLAY && pitchLineAngle <= MAX_PITCH_DISPLAY)
            {
                drawSinglePitchLine(renderer, centerX, centerY, pitchLineAngle, pitchAngle, rollAngle);
            }
        }
    }

    void PitchLadder::drawSinglePitchLine(gfx::Renderer2D &renderer, float centerX, float centerY,
                                          float pitchLineAngle, float currentPitch, float rollAngle)
    {
        // Difference between desired division and current pitch
        float pitchDiff = pitchLineAngle - currentPitch;
        float lineYNDC = pitchDiff * NDC_PER_DEGREE; // 1% NDC per degree

        // Only show lines within visible range
        if (std::abs(lineYNDC) > NDC_VISIBILITY_LIMIT)
            return;

        // Convert Y position from NDC to pixels (relative to center)
        float lineY = centerY - (lineYNDC * size_.y * 0.5f); // Invert because Y grows downwards

        // Determine line width (0 degrees is longer)
        float lineWidthNDC = (pitchLineAngle == 0.0f) ? NDC_LINE_WIDTH_ZERO : NDC_LINE_WIDTH_NORMAL;
        float lineWidthPx = ndcDimensionToPixels(lineWidthNDC, size_.x);
        float gapPx = ndcDimensionToPixels(NDC_GAP, size_.x);

        // Precalculate sine and cosine for rotation
        // Convert roll to radians (negative because screen rotation is clockwise for positive roll)
        // Note: In simulators, positive roll is banking right. Visually the horizon must rotate LEFT.
        float rollRad = glm::radians(-rollAngle);
        float cosRoll = std::cos(rollRad);
        float sinRoll = std::sin(rollRad);

        // Lambda function to rotate a point around (centerX, centerY)
        auto rotatePoint = [&](glm::vec2 p) -> glm::vec2 {
            float dx = p.x - centerX;
            float dy = p.y - centerY;
            return glm::vec2(
                centerX + dx * cosRoll - dy * sinRoll,
                centerY + dx * sinRoll + dy * cosRoll
            );
        };

        // Calculate endpoints of the two parts of the line (cut to leave crosshair visible)
        // Left part: from -lineWidth to -gap
        glm::vec2 leftStartRaw(centerX - lineWidthPx, lineY);
        glm::vec2 leftEndRaw(centerX - gapPx, lineY);

        // Right part: from gap to lineWidth
        glm::vec2 rightStartRaw(centerX + gapPx, lineY);
        glm::vec2 rightEndRaw(centerX + lineWidthPx, lineY);

        // Apply rotation
        glm::vec2 leftStart = rotatePoint(leftStartRaw);
        glm::vec2 leftEnd = rotatePoint(leftEndRaw);
        glm::vec2 rightStart = rotatePoint(rightStartRaw);
        glm::vec2 rightEnd = rotatePoint(rightEndRaw);

        // Draw the two parts of the horizontal line
        renderer.drawLine(leftStart, leftEnd, color_, 2.0f);
        renderer.drawLine(rightStart, rightEnd, color_, 2.0f);

        // If not the center line (0 degrees), add markers at the ends
        if (pitchLineAngle != 0.0f)
        {
            float markerSizePx = ndcDimensionToPixels(NDC_MARKER_SIZE, size_.y);
            bool isPositive = (pitchLineAngle > 0.0f);

            // Left marker (vertical)
            glm::vec2 leftMarkerStartRaw(centerX - lineWidthPx, lineY);
            glm::vec2 leftMarkerEndRaw(centerX - lineWidthPx,
                                    lineY + (isPositive ? markerSizePx : -markerSizePx));
            
            // Right marker (vertical)
            glm::vec2 rightMarkerStartRaw(centerX + lineWidthPx, lineY);
            glm::vec2 rightMarkerEndRaw(centerX + lineWidthPx,
                                     lineY + (isPositive ? markerSizePx : -markerSizePx));

            // Apply rotation to markers
            renderer.drawLine(rotatePoint(leftMarkerStartRaw), rotatePoint(leftMarkerEndRaw), color_, 2.0f);
            renderer.drawLine(rotatePoint(rightMarkerStartRaw), rotatePoint(rightMarkerEndRaw), color_, 2.0f);

            // HUD convention: markers point up for positive pitch (sky)
            // and down for negative pitch (ground) to convey visual sense.
        }
    }

    float PitchLadder::ndcDimensionToPixels(float ndcDim, float axisLength) const
    {
        // Scales an NDC percentage to the equivalent amount in pixels
        return ndcDim * axisLength * 0.5f;
    }

} // namespace hud
