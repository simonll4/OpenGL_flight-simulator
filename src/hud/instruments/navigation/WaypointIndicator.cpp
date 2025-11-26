/**
 * @file WaypointIndicator.cpp
 * @brief Implementation of the waypoint navigation indicator.
 *
 * This file contains the complete implementation of a navigation indicator
 * inspired by the Garmin HSI. It provides an intuitive visual interface for
 * waypoint navigation with a fixed compass and a vertical indicator.
 *
 * Key features:
 * - Compass rose with marks every 5 degrees and labels every 30 degrees.
 * - Magenta arrow indicating direction to the waypoint.
 * - Vertical altitude difference indicator.
 *
 * @author Flight Simulator Team
 * @date 2024
 */

#include "WaypointIndicator.h"
#include "../../../gfx/rendering/TextRenderer.h"
#include <glm/glm.hpp>
#include <cmath>
#include <iomanip>
#include <sstream>

namespace hud
{
    namespace
    {
        // ====================================================================
        // INDICATOR CONFIGURATION CONSTANTS
        // ====================================================================

        constexpr float kRoseRadius = 55.0f;              ///< Compass rose radius
        constexpr float kMajorTickLength = 10.0f;         ///< Major tick length (every 10 degrees)
        constexpr float kMinorTickLength = 5.0f;          ///< Minor tick length (every 5 degrees)
        constexpr float kVerticalIndicatorOffset = 25.0f; ///< Vertical indicator separation
        constexpr float kVerticalIndicatorHeight = 60.0f; ///< Vertical indicator height
        constexpr float kMaxAltitudeDiff = 500.0f;        ///< Maximum displayed altitude difference
        constexpr float kPanelMarginLeft = 12.0f;
        constexpr float kPanelMarginRight = 12.0f;
        constexpr float kPanelMarginTop = 12.0f;
        constexpr float kPanelMarginBottom = 12.0f;
        constexpr float kPanelWidth = kPanelMarginLeft + kVerticalIndicatorOffset + kRoseRadius * 2.0f + kPanelMarginRight;
        constexpr float kPanelHeight = kPanelMarginTop + kRoseRadius * 2.0f + kPanelMarginBottom;

        float normalizeRelativeAngle(float angle)
        {
            while (angle > 180.0f)
                angle -= 360.0f;
            while (angle < -180.0f)
                angle += 360.0f;
            return angle;
        }

        glm::vec2 compassCenter(const glm::vec2 &panelPos)
        {
            float centerX = panelPos.x + kPanelMarginLeft + kVerticalIndicatorOffset + kRoseRadius;
            float centerY = panelPos.y + kPanelMarginTop + kRoseRadius;
            return glm::vec2(centerX, centerY);
        }

        void drawCompassTicks(gfx::Renderer2D &renderer, const glm::vec2 &center, const glm::vec4 &color)
        {
            for (int i = 0; i < 72; ++i)
            {
                float angleDeg = -90.0f + i * 5.0f;
                float angleRad = glm::radians(angleDeg);
                bool isMajor = (i % 2 == 0);
                bool isNumeric = (i % 6 == 0);

                float innerRadius = kRoseRadius - (isMajor ? kMajorTickLength : kMinorTickLength);
                glm::vec2 startPos = center + glm::vec2(std::cos(angleRad), std::sin(angleRad)) * innerRadius;
                glm::vec2 endPos = center + glm::vec2(std::cos(angleRad), std::sin(angleRad)) * kRoseRadius;
                glm::vec4 tickColor = glm::vec4(color.r, color.g, color.b, isMajor ? 0.8f : 0.4f);
                renderer.drawLine(startPos, endPos, tickColor, isMajor ? 2.0f : 1.0f);

                if (isNumeric)
                {
                    int labelDeg = (i * 5) % 360;
                    std::ostringstream ss;
                    ss << std::setfill('0') << std::setw(3) << labelDeg;
                    float textRadius = kRoseRadius + 12.0f;
                    glm::vec2 textPos = center + glm::vec2(std::cos(angleRad), std::sin(angleRad)) * textRadius - glm::vec2(8.0f, 4.0f);
                    gfx::TextRenderer::drawString(renderer, ss.str(), textPos, glm::vec2(5.0f, 8.0f), glm::vec4(color.r, color.g, color.b, 0.8f), 6.0f);
                }
            }
        }

        void drawCompassCardinals(gfx::Renderer2D &renderer, const glm::vec2 &center, const glm::vec4 &color)
        {
            static const char *cardinals[4] = {"N", "E", "S", "W"};
            for (int i = 0; i < 4; ++i)
            {
                float angleDeg = -90.0f + i * 90.0f;
                float angleRad = glm::radians(angleDeg);
                float labelRadius = kRoseRadius + 20.0f;
                glm::vec2 pos = center + glm::vec2(std::cos(angleRad), std::sin(angleRad)) * labelRadius - glm::vec2(4.0f, 5.0f);
                gfx::TextRenderer::drawString(renderer, cardinals[i], pos, glm::vec2(7.0f, 10.0f), color, 8.0f);
            }
        }

        void drawWaypointPointer(gfx::Renderer2D &renderer, const glm::vec2 &center, float relativeAngle, const glm::vec4 &color)
        {
            float pointerRad = glm::radians(-90.0f + relativeAngle);
            float pointerLength = kRoseRadius * 0.8f;
            glm::vec2 tip = center + glm::vec2(std::cos(pointerRad), std::sin(pointerRad)) * pointerLength;

            renderer.drawLine(center, tip, color, 4.0f);

            float headSize = 15.0f;
            float leftAng = pointerRad + glm::radians(135.0f);
            float rightAng = pointerRad - glm::radians(135.0f);
            glm::vec2 left = tip + glm::vec2(std::cos(leftAng), std::sin(leftAng)) * headSize;
            glm::vec2 right = tip + glm::vec2(std::cos(rightAng), std::sin(rightAng)) * headSize;
            renderer.drawLine(tip, left, color, 4.0f);
            renderer.drawLine(tip, right, color, 4.0f);
            renderer.drawLine(left, right, color, 4.0f);

            int segments = 10;
            float segmentLength = pointerLength / static_cast<float>(segments);
            for (int i = 0; i < segments - 1; i += 2)
            {
                float startOffset = (i + 0.2f) * segmentLength;
                float endOffset = (i + 1.0f) * segmentLength;
                glm::vec2 startDash = center + glm::vec2(std::cos(pointerRad), std::sin(pointerRad)) * startOffset;
                glm::vec2 endDash = center + glm::vec2(std::cos(pointerRad), std::sin(pointerRad)) * endOffset;
                renderer.drawLine(startDash, endDash, glm::vec4(color.r, color.g, color.b, 0.5f), 2.0f);
            }
        }

    } // namespace

    ////////////////////////////////////////////////////////////////////////////
    //  Lifecycle
    ////////////////////////////////////////////////////////////////////////////

    WaypointIndicator::WaypointIndicator()
        : Instrument()
    {
        size_ = glm::vec2(kPanelWidth, kPanelHeight);
        color_ = glm::vec4(1.0f, 0.2f, 0.7f, 0.9f);
    }

    ////////////////////////////////////////////////////////////////////////////
    //  Main Render
    ////////////////////////////////////////////////////////////////////////////

    void WaypointIndicator::render(gfx::Renderer2D &renderer, const flight::FlightData &flightData)
    {
        if (!enabled_)
            return;

        if (!flightData.hasActiveWaypoint)
        {
            return;
        }

        NavSnapshot nav = buildNavSnapshot(flightData);
        drawCompassRose(renderer, nav);
        drawVerticalIndicator(renderer, nav);
    }

    ////////////////////////////////////////////////////////////////////////////
    //  Data Preparation
    ////////////////////////////////////////////////////////////////////////////

    WaypointIndicator::NavSnapshot WaypointIndicator::buildNavSnapshot(const flight::FlightData &flightData) const
    {
        NavSnapshot nav;
        nav.heading = flightData.heading;
        nav.relativeAngle = normalizeRelativeAngle(flightData.waypointBearing - nav.heading);
        nav.altitudeDifference = flightData.targetWaypoint.y - flightData.position.y;
        return nav;
    }

    ////////////////////////////////////////////////////////////////////////////
    //  Compass Rose + Magenta Arrow
    ////////////////////////////////////////////////////////////////////////////

    void WaypointIndicator::drawCompassRose(gfx::Renderer2D &renderer, const NavSnapshot &nav)
    {
        glm::vec2 center = compassCenter(position_);
        renderer.drawCircle(center, kRoseRadius, glm::vec4(color_.r, color_.g, color_.b, 0.5f), 64, false);
        renderer.drawCircle(center, kRoseRadius * 0.92f, glm::vec4(color_.r, color_.g, color_.b, 0.2f), 64, false);

        drawCompassTicks(renderer, center, color_);
        drawCompassCardinals(renderer, center, color_);
        drawWaypointPointer(renderer, center, nav.relativeAngle, color_);
    }

    ////////////////////////////////////////////////////////////////////////////
    //  Vertical Relative Altitude Indicator
    ////////////////////////////////////////////////////////////////////////////

    void WaypointIndicator::drawVerticalIndicator(gfx::Renderer2D &renderer, const NavSnapshot &nav)
    {
        glm::vec2 center = compassCenter(position_);
        float indicatorX = center.x - kRoseRadius - kVerticalIndicatorOffset;
        float indicatorY = center.y;

        glm::vec2 lineTop = glm::vec2(indicatorX, indicatorY - kVerticalIndicatorHeight * 0.5f);
        glm::vec2 lineBottom = glm::vec2(indicatorX, indicatorY + kVerticalIndicatorHeight * 0.5f);
        renderer.drawLine(lineTop, lineBottom, glm::vec4(color_.r, color_.g, color_.b, 0.4f), 2.0f);

        float markSize = 5.0f;
        renderer.drawLine(glm::vec2(indicatorX - markSize, indicatorY), glm::vec2(indicatorX + markSize, indicatorY), glm::vec4(color_.r, color_.g, color_.b, 0.6f), 2.0f);

        float clampedDiff = glm::clamp(nav.altitudeDifference, -kMaxAltitudeDiff, kMaxAltitudeDiff);
        float normalizedPos = clampedDiff / kMaxAltitudeDiff;
        float arrowY = indicatorY - normalizedPos * (kVerticalIndicatorHeight * 0.4f);

        glm::vec4 arrowColor;
        if (std::abs(nav.altitudeDifference) < 50.0f)
            arrowColor = color_;
        else if (nav.altitudeDifference > 0.0f)
            arrowColor = glm::vec4(1.0f, 0.9f, 0.1f, 1.0f);
        else
            arrowColor = glm::vec4(0.3f, 0.7f, 1.0f, 1.0f);

        float arrowSize = 8.0f;
        if (nav.altitudeDifference > 50.0f)
        {
            glm::vec2 tip = glm::vec2(indicatorX, arrowY - arrowSize);
            glm::vec2 left = glm::vec2(indicatorX - arrowSize * 0.7f, arrowY);
            glm::vec2 right = glm::vec2(indicatorX + arrowSize * 0.7f, arrowY);
            renderer.drawLine(tip, left, arrowColor, 3.0f);
            renderer.drawLine(tip, right, arrowColor, 3.0f);
            renderer.drawLine(left, right, arrowColor, 3.0f);
            gfx::TextRenderer::drawString(renderer, "UP", glm::vec2(indicatorX - 10.0f, arrowY + arrowSize + 2.0f), glm::vec2(5.0f, 7.0f), arrowColor, 6.0f);
        }
        else if (nav.altitudeDifference < -50.0f)
        {
            glm::vec2 tip = glm::vec2(indicatorX, arrowY + arrowSize);
            glm::vec2 left = glm::vec2(indicatorX - arrowSize * 0.7f, arrowY);
            glm::vec2 right = glm::vec2(indicatorX + arrowSize * 0.7f, arrowY);
            renderer.drawLine(tip, left, arrowColor, 3.0f);
            renderer.drawLine(tip, right, arrowColor, 3.0f);
            renderer.drawLine(left, right, arrowColor, 3.0f);
            gfx::TextRenderer::drawString(renderer, "DN", glm::vec2(indicatorX - 8.0f, arrowY - arrowSize - 10.0f), glm::vec2(5.0f, 7.0f), arrowColor, 6.0f);
        }
        else
        {
            renderer.drawCircle(glm::vec2(indicatorX, arrowY), 5.0f, arrowColor, 16, true);
            gfx::TextRenderer::drawString(renderer, "LVL", glm::vec2(indicatorX - 10.0f, arrowY + 8.0f), glm::vec2(5.0f, 7.0f), arrowColor, 6.0f);
        }

        std::ostringstream ss;
        if (std::abs(nav.altitudeDifference) >= 1000.0f)
            ss << std::fixed << std::setprecision(1) << (nav.altitudeDifference / 1000.0f) << "km";
        else
            ss << static_cast<int>(std::round(nav.altitudeDifference)) << "m";

        gfx::TextRenderer::drawString(renderer, ss.str(), glm::vec2(indicatorX - 15.0f, lineBottom.y + 5.0f), glm::vec2(5.0f, 7.0f), glm::vec4(color_.r, color_.g, color_.b, 0.8f), 6.0f);
    }

} // namespace hud
