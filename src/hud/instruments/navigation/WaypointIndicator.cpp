/**
 * @file WaypointIndicator.cpp
 * @brief Implementación del indicador de navegación HSI (Horizontal Situation Indicator)
 *
 * Este archivo contiene la implementación completa de un indicador de navegación
 * inspirado en el HSI de Garmin. Proporciona una interfaz visual intuitiva para
 * la navegación por waypoints, combinando una brújula rotativa con información
 * digital de navegación.
 *
 * Características principales:
 * - Rosa de los vientos con marcas cada 5° y etiquetas cada 30°
 * - Flecha magenta indicando dirección al waypoint
 * - Panel de información digital (distancia, ángulo de giro, bearing)
 * - Indicador vertical de diferencia de altura
 * - Barra de proximidad al waypoint
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
#include <utility>

namespace hud
{
    namespace
    {
        // ====================================================================
        // CONSTANTES DE CONFIGURACIÓN DEL HSI
        // ====================================================================
        
        constexpr float kPanelWidth = 340.0f;              ///< Ancho del panel principal
        constexpr float kPanelHeight = 180.0f;             ///< Alto del panel principal
        constexpr float kRoseRadius = 55.0f;               ///< Radio de la rosa de los vientos
        constexpr float kMajorTickLength = 10.0f;          ///< Longitud de marcas principales (cada 10°)
        constexpr float kMinorTickLength = 5.0f;           ///< Longitud de marcas menores (cada 5°)
        constexpr float kVerticalIndicatorOffset = 25.0f;  ///< Separación del indicador vertical
        constexpr float kVerticalIndicatorHeight = 60.0f;  ///< Altura del indicador vertical
        constexpr float kMaxAltitudeDiff = 500.0f;         ///< Máxima diferencia de altura mostrada

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
            return panelPos + glm::vec2(kPanelWidth * 0.26f, kPanelHeight * 0.52f);
        }

        glm::vec2 infoPanelOrigin(const glm::vec2 &panelPos)
        {
            return panelPos + glm::vec2(kPanelWidth * 0.55f, 18.0f);
        }

        std::pair<std::string, std::string> formatDistance(float distance)
        {
            if (distance >= 1000.0f)
            {
                float km = distance / 1000.0f;
                std::ostringstream ss;
                ss << std::fixed << std::setprecision(1) << km;
                return {ss.str(), "KM"};
            }

            int meters = static_cast<int>(std::round(distance));
            return {std::to_string(meters), "M"};
        }

        std::string formatBearing(float bearing)
        {
            int brg = static_cast<int>(std::round(bearing));
            if (brg < 0)
                brg += 360;
            if (brg >= 360)
                brg -= 360;

            std::ostringstream ss;
            ss << std::setfill('0') << std::setw(3) << brg << "°";
            return ss.str();
        }

        std::string formatTurn(float relativeAngle)
        {
            int turnInt = static_cast<int>(std::round(relativeAngle));
            std::string prefix = (turnInt > 0) ? "+" : "";
            return prefix + std::to_string(turnInt) + "°";
        }

        glm::vec4 turnColor(float relativeAngle, const glm::vec4 &baseColor)
        {
            float absTurn = std::abs(relativeAngle);
            if (absTurn < 5.0f)
                return baseColor;
            if (absTurn < 20.0f)
                return glm::vec4(1.0f, 0.8f, 0.0f, 1.0f);
            return glm::vec4(1.0f, 0.3f, 0.0f, 1.0f);
        }

        void drawCompassTicks(gfx::Renderer2D &renderer, const glm::vec2 &center, float heading, const glm::vec4 &color)
        {
            for (int i = 0; i < 72; ++i)
            {
                float angleDeg = -90.0f + i * 5.0f;
                float angleRad = glm::radians(angleDeg - heading);
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

        void drawCompassCardinals(gfx::Renderer2D &renderer, const glm::vec2 &center, float heading, const glm::vec4 &color)
        {
            static const char *cardinals[4] = {"N", "E", "S", "W"};
            for (int i = 0; i < 4; ++i)
            {
                float angleDeg = -90.0f + i * 90.0f;
                float angleRad = glm::radians(angleDeg - heading);
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

        void drawHeadingBug(gfx::Renderer2D &renderer, const glm::vec2 &center)
        {
            glm::vec4 bugColor = glm::vec4(1.0f, 0.9f, 0.1f, 1.0f);
            float bugRadius = kRoseRadius + 14.0f;
            glm::vec2 bugTip = center + glm::vec2(0.0f, -bugRadius);
            glm::vec2 bugLeft = bugTip + glm::vec2(-6.0f, 10.0f);
            glm::vec2 bugRight = bugTip + glm::vec2(6.0f, 10.0f);
            renderer.drawLine(bugTip, bugLeft, bugColor, 3.0f);
            renderer.drawLine(bugTip, bugRight, bugColor, 3.0f);
            renderer.drawLine(bugLeft, bugRight, bugColor, 3.0f);
        }
    } // namespace

    WaypointIndicator::WaypointIndicator()
        : Instrument()
    {
        size_ = glm::vec2(kPanelWidth, kPanelHeight);
        color_ = glm::vec4(1.0f, 0.2f, 0.7f, 0.9f);
    }

    void WaypointIndicator::render(gfx::Renderer2D &renderer, const flight::FlightData &flightData)
    {
        if (!flightData.hasActiveWaypoint)
        {
            drawNoWaypointMessage(renderer);
            return;
        }

        NavSnapshot nav = buildNavSnapshot(flightData);
        drawMainPanel(renderer);
        drawCompassRose(renderer, nav);
        drawInfoPanel(renderer, nav);
        drawVerticalIndicator(renderer, nav);
    }

    WaypointIndicator::NavSnapshot WaypointIndicator::buildNavSnapshot(const flight::FlightData &flightData) const
    {
        NavSnapshot nav;
        nav.heading = flightData.heading;
        nav.distance = flightData.waypointDistance;
        nav.bearing = flightData.waypointBearing;
        nav.relativeAngle = normalizeRelativeAngle(nav.bearing - nav.heading);
        nav.altitudeDifference = flightData.targetWaypoint.y - flightData.position.y;
        return nav;
    }

    void WaypointIndicator::drawMainPanel(gfx::Renderer2D &renderer)
    {
        glm::vec4 bgColor = glm::vec4(0.0f, 0.0f, 0.0f, 0.6f);
        renderer.drawRect(position_, size_, bgColor, true);
        renderer.drawRect(position_, size_, glm::vec4(color_.r, color_.g, color_.b, 0.6f), false);
        glm::vec2 labelPos = position_ + glm::vec2(8.0f, 10.0f);
        gfx::TextRenderer::drawString(renderer, "HSI", labelPos, glm::vec2(7.0f, 10.0f), color_, 8.0f);
    }

    void WaypointIndicator::drawCompassRose(gfx::Renderer2D &renderer, const NavSnapshot &nav)
    {
        glm::vec2 center = compassCenter(position_);
        renderer.drawCircle(center, kRoseRadius, glm::vec4(color_.r, color_.g, color_.b, 0.5f), 64, false);
        renderer.drawCircle(center, kRoseRadius * 0.92f, glm::vec4(color_.r, color_.g, color_.b, 0.2f), 64, false);

        drawCompassTicks(renderer, center, nav.heading, color_);
        drawCompassCardinals(renderer, center, nav.heading, color_);
        drawWaypointPointer(renderer, center, nav.relativeAngle, color_);
        drawHeadingBug(renderer, center);
    }

    void WaypointIndicator::drawInfoPanel(gfx::Renderer2D &renderer, const NavSnapshot &nav)
    {
        glm::vec2 infoOrigin = infoPanelOrigin(position_);
        float panelX = infoOrigin.x;
        float startY = infoOrigin.y;

        auto [distVal, distUnit] = formatDistance(nav.distance);
        gfx::TextRenderer::drawString(renderer, "DIST", glm::vec2(panelX, startY), glm::vec2(6.0f, 9.0f), glm::vec4(color_.r, color_.g, color_.b, 0.7f), 7.0f);
        gfx::TextRenderer::drawString(renderer, distVal, glm::vec2(panelX, startY + 12.0f), glm::vec2(12.0f, 17.0f), color_, 14.0f);
        gfx::TextRenderer::drawString(renderer, distUnit, glm::vec2(panelX + distVal.length() * 12.0f + 4.0f, startY + 18.0f), glm::vec2(6.0f, 9.0f), glm::vec4(color_.r, color_.g, color_.b, 0.7f), 8.0f);

        float turnY = startY + 42.0f;
        gfx::TextRenderer::drawString(renderer, "TURN", glm::vec2(panelX, turnY), glm::vec2(6.0f, 9.0f), glm::vec4(color_.r, color_.g, color_.b, 0.7f), 7.0f);
        glm::vec4 turnClr = turnColor(nav.relativeAngle, color_);
        gfx::TextRenderer::drawString(renderer, formatTurn(nav.relativeAngle), glm::vec2(panelX, turnY + 12.0f), glm::vec2(10.0f, 15.0f), turnClr, 12.0f);

        float brgY = startY + 78.0f;
        gfx::TextRenderer::drawString(renderer, "BRG", glm::vec2(panelX, brgY), glm::vec2(6.0f, 9.0f), glm::vec4(color_.r, color_.g, color_.b, 0.7f), 7.0f);
        gfx::TextRenderer::drawString(renderer, formatBearing(nav.bearing), glm::vec2(panelX, brgY + 12.0f), glm::vec2(9.0f, 13.0f), color_, 11.0f);

        float barY = startY + 115.0f;
        float barWidth = 110.0f;
        float barHeight = 7.0f;
        renderer.drawRect(glm::vec2(panelX, barY), glm::vec2(barWidth, barHeight), glm::vec4(color_.r, color_.g, color_.b, 0.2f), true);
        float proximity = 1.0f - glm::clamp(nav.distance / 2000.0f, 0.0f, 1.0f);
        renderer.drawRect(glm::vec2(panelX, barY), glm::vec2(barWidth * proximity, barHeight), glm::vec4(color_.r, color_.g, color_.b, 0.8f), true);
    }

    void WaypointIndicator::drawNoWaypointMessage(gfx::Renderer2D &renderer)
    {
        glm::vec4 bgColor = glm::vec4(0.0f, 0.0f, 0.0f, 0.6f);
        renderer.drawRect(position_, size_, bgColor, true);
        renderer.drawRect(position_, size_, glm::vec4(color_.r, color_.g, color_.b, 0.5f), false);
        glm::vec2 center = position_ + size_ * 0.5f;
        glm::vec2 textPos = center - glm::vec2(60.0f, 0.0f);
        gfx::TextRenderer::drawString(renderer, "NO WAYPOINT", textPos, glm::vec2(10.0f, 14.0f), glm::vec4(color_.r, color_.g, color_.b, 0.7f), 13.0f);
        renderer.drawCircle(center - glm::vec2(0.0f, 40.0f), 30.0f, glm::vec4(color_.r, color_.g, color_.b, 0.2f), 32, false);
    }

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
