#include "WaypointIndicator.h"
#include "../../../gfx/rendering/TextRenderer.h"
#include <cmath>
#include <iomanip>
#include <sstream>

namespace hud
{
    // ============================================================================
    // CONFIGURACIÓN VISUAL DEL INDICADOR - ESTILO PROFESIONAL
    // ============================================================================

    static const float PANEL_WIDTH = 280.0f;
    static const float PANEL_HEIGHT = 140.0f;
    static const float COMPASS_RADIUS = 45.0f;
    static const float ARROW_LENGTH = 35.0f;
    static const float ARROW_WIDTH = 8.0f;
    static const float INFO_SPACING = 10.0f;

    WaypointIndicator::WaypointIndicator() : Instrument()
    {
        // Panel más grande y profesional
        size_ = glm::vec2(PANEL_WIDTH, PANEL_HEIGHT);
        color_ = glm::vec4(0.0f, 1.0f, 0.4f, 0.95f); // Verde HUD
    }

    // ============================================================================
    // FUNCIÓN PRINCIPAL DE RENDERIZADO
    // ============================================================================

    void WaypointIndicator::render(gfx::Renderer2D &renderer, const flight::FlightData &flightData)
    {
        // Si no hay waypoint activo, mostrar mensaje
        if (!flightData.hasActiveWaypoint)
        {
            drawNoWaypointMessage(renderer);
            return;
        }

        // Calcular ángulo relativo
        float relativeAngle = flightData.waypointBearing - flightData.heading;
        while (relativeAngle > 180.0f) relativeAngle -= 360.0f;
        while (relativeAngle < -180.0f) relativeAngle += 360.0f;

        // Renderizar componentes del panel profesional
        drawMainPanel(renderer);
        drawCompass(renderer, relativeAngle);
        drawInfoDisplay(renderer, flightData.waypointDistance, relativeAngle, flightData.waypointBearing);
    }

    // ============================================================================
    // PANEL PRINCIPAL CON FONDO SEMI-TRANSPARENTE
    // ============================================================================

    void WaypointIndicator::drawMainPanel(gfx::Renderer2D &renderer)
    {
        // Fondo semi-transparente oscuro para mejor contraste
        glm::vec4 bgColor = glm::vec4(0.0f, 0.0f, 0.0f, 0.6f);
        renderer.drawRect(position_, size_, bgColor, true);

        // Bordes del panel
        renderer.drawRect(position_, size_, color_, false);

        // Línea divisoria vertical entre brújula e info
        float dividerX = position_.x + PANEL_WIDTH * 0.45f;
        renderer.drawLine(
            glm::vec2(dividerX, position_.y + 5.0f),
            glm::vec2(dividerX, position_.y + size_.y - 5.0f),
            glm::vec4(color_.r, color_.g, color_.b, 0.4f),
            2.0f);

        // Etiqueta "NAV" en la esquina superior izquierda
        glm::vec2 labelPos = position_ + glm::vec2(8.0f, 10.0f);
        gfx::TextRenderer::drawString(renderer, "NAV", labelPos, 
            glm::vec2(7.0f, 10.0f), color_, 8.0f);
    }

    // ============================================================================
    // BRÚJULA CIRCULAR CON FLECHA DIRECCIONAL (Estilo HSI)
    // ============================================================================

    void WaypointIndicator::drawCompass(gfx::Renderer2D &renderer, float relativeAngle)
    {
        // Centro de la brújula (lado izquierdo del panel)
        glm::vec2 center = position_ + glm::vec2(PANEL_WIDTH * 0.22f, size_.y * 0.55f);

        // Círculo exterior de la brújula
        renderer.drawCircle(center, COMPASS_RADIUS, color_, 32, false);
        
        // Círculo interior más pequeño
        renderer.drawCircle(center, COMPASS_RADIUS * 0.75f, 
            glm::vec4(color_.r, color_.g, color_.b, 0.3f), 32, false);

        // Marcas cardinales (N, S, E, W)
        float markRadius = COMPASS_RADIUS * 0.85f;
        
        // Norte (arriba)
        glm::vec2 northPos = center + glm::vec2(0.0f, -markRadius - 10.0f);
        gfx::TextRenderer::drawString(renderer, "N", northPos, 
            glm::vec2(7.0f, 10.0f), color_, 8.0f);
        
        // Marcas de tick en los puntos cardinales
        for (int i = 0; i < 4; i++)
        {
            float angle = i * 90.0f;
            float rad = glm::radians(angle - 90.0f);
            glm::vec2 start = center + glm::vec2(std::cos(rad), std::sin(rad)) * (COMPASS_RADIUS - 5.0f);
            glm::vec2 end = center + glm::vec2(std::cos(rad), std::sin(rad)) * COMPASS_RADIUS;
            renderer.drawLine(start, end, color_, 2.0f);
        }

        // FLECHA DIRECCIONAL GRANDE Y VISIBLE
        float angleRad = glm::radians(relativeAngle - 90.0f);
        glm::vec2 arrowTip = center + glm::vec2(
            std::cos(angleRad) * ARROW_LENGTH,
            std::sin(angleRad) * ARROW_LENGTH
        );

        // Línea principal de la flecha (más gruesa)
        renderer.drawLine(center, arrowTip, color_, 4.0f);

        // Punta de flecha sólida (triángulo)
        float headSize = 14.0f;
        float headAngle1 = angleRad + glm::radians(140.0f);
        float headAngle2 = angleRad - glm::radians(140.0f);

        glm::vec2 arrowHead1 = arrowTip + glm::vec2(
            std::cos(headAngle1) * headSize,
            std::sin(headAngle1) * headSize
        );
        glm::vec2 arrowHead2 = arrowTip + glm::vec2(
            std::cos(headAngle2) * headSize,
            std::sin(headAngle2) * headSize
        );

        // Dibujar triángulo de la punta
        renderer.drawLine(arrowTip, arrowHead1, color_, 4.0f);
        renderer.drawLine(arrowTip, arrowHead2, color_, 4.0f);
        renderer.drawLine(arrowHead1, arrowHead2, color_, 4.0f);

        // Indicador de rumbo actual (tick arriba fijo)
        glm::vec2 tickTop = center + glm::vec2(0.0f, -COMPASS_RADIUS - 3.0f);
        glm::vec2 tickBottom = center + glm::vec2(0.0f, -COMPASS_RADIUS + 5.0f);
        renderer.drawLine(tickTop, tickBottom, 
            glm::vec4(1.0f, 0.8f, 0.0f, 1.0f), 3.0f); // Amarillo para destacar
    }

    // ============================================================================
    // PANEL DE INFORMACIÓN DIGITAL (Lado derecho)
    // ============================================================================

    void WaypointIndicator::drawInfoDisplay(gfx::Renderer2D &renderer, float distance, 
                                             float relativeAngle, float bearing)
    {
        float infoX = position_.x + PANEL_WIDTH * 0.52f;
        float startY = position_.y + 15.0f;

        // === DISTANCIA (Display principal - números grandes) ===
        std::string distValue, distUnit;
        if (distance >= 1000.0f)
        {
            float distKm = distance / 1000.0f;
            std::ostringstream ss;
            ss << std::fixed << std::setprecision(1) << distKm;
            distValue = ss.str();
            distUnit = "KM";
        }
        else
        {
            distValue = std::to_string(static_cast<int>(std::round(distance)));
            distUnit = "M";
        }

        // Etiqueta "DIST"
        gfx::TextRenderer::drawString(renderer, "DIST", 
            glm::vec2(infoX, startY), 
            glm::vec2(6.0f, 9.0f), 
            glm::vec4(color_.r, color_.g, color_.b, 0.7f), 7.0f);

        // Valor de distancia (números GRANDES)
        gfx::TextRenderer::drawString(renderer, distValue, 
            glm::vec2(infoX, startY + 12.0f), 
            glm::vec2(11.0f, 16.0f), color_, 14.0f);

        // Unidad
        gfx::TextRenderer::drawString(renderer, distUnit, 
            glm::vec2(infoX + distValue.length() * 11.0f + 3.0f, startY + 18.0f), 
            glm::vec2(6.0f, 9.0f), 
            glm::vec4(color_.r, color_.g, color_.b, 0.7f), 8.0f);

        // === ÁNGULO DE DESVIACIÓN ===
        float angleY = startY + 40.0f;
        
        // Etiqueta "TURN"
        gfx::TextRenderer::drawString(renderer, "TURN", 
            glm::vec2(infoX, angleY), 
            glm::vec2(6.0f, 9.0f), 
            glm::vec4(color_.r, color_.g, color_.b, 0.7f), 7.0f);

        // Valor del ángulo con signo
        int angleInt = static_cast<int>(std::round(relativeAngle));
        std::string angleStr = (angleInt > 0 ? "+" : "") + std::to_string(angleInt) + "°";
        
        // Color según desviación (verde si está cerca, amarillo si no)
        glm::vec4 angleColor = (std::abs(angleInt) < 10.0f) 
            ? color_ 
            : glm::vec4(1.0f, 0.8f, 0.0f, 1.0f);

        gfx::TextRenderer::drawString(renderer, angleStr, 
            glm::vec2(infoX, angleY + 12.0f), 
            glm::vec2(9.0f, 14.0f), angleColor, 12.0f);

        // === BEARING (Rumbo al waypoint) ===
        float bearingY = startY + 75.0f;
        
        // Etiqueta "BRG"
        gfx::TextRenderer::drawString(renderer, "BRG", 
            glm::vec2(infoX, bearingY), 
            glm::vec2(6.0f, 9.0f), 
            glm::vec4(color_.r, color_.g, color_.b, 0.7f), 7.0f);

        // Valor del bearing (000-360)
        int bearingInt = static_cast<int>(std::round(bearing));
        if (bearingInt < 0) bearingInt += 360;
        if (bearingInt >= 360) bearingInt -= 360;
        
        std::ostringstream bearingSS;
        bearingSS << std::setfill('0') << std::setw(3) << bearingInt << "°";
        
        gfx::TextRenderer::drawString(renderer, bearingSS.str(), 
            glm::vec2(infoX, bearingY + 12.0f), 
            glm::vec2(8.0f, 12.0f), color_, 10.0f);

        // Barra de progreso visual (indicador de cercanía)
        float barY = startY + 110.0f;
        float barWidth = 100.0f;
        float barHeight = 6.0f;
        
        // Fondo de la barra
        renderer.drawRect(
            glm::vec2(infoX, barY),
            glm::vec2(barWidth, barHeight),
            glm::vec4(color_.r, color_.g, color_.b, 0.2f),
            true);
        
        // Relleno de la barra (más lleno = más cerca)
        float proximityPercent = 1.0f - glm::clamp(distance / 2000.0f, 0.0f, 1.0f);
        renderer.drawRect(
            glm::vec2(infoX, barY),
            glm::vec2(barWidth * proximityPercent, barHeight),
            color_,
            true);
    }

    // ============================================================================
    // MENSAJE CUANDO NO HAY WAYPOINT ACTIVO
    // ============================================================================

    void WaypointIndicator::drawNoWaypointMessage(gfx::Renderer2D &renderer)
    {
        // Fondo semi-transparente
        glm::vec4 bgColor = glm::vec4(0.0f, 0.0f, 0.0f, 0.6f);
        renderer.drawRect(position_, size_, bgColor, true);
        renderer.drawRect(position_, size_, 
            glm::vec4(color_.r, color_.g, color_.b, 0.4f), false);

        glm::vec2 center = position_ + size_ * 0.5f;

        // Mensaje "NO WAYPOINT"
        glm::vec2 textPos = center - glm::vec2(50.0f, 0.0f);
        gfx::TextRenderer::drawString(renderer, "NO WAYPOINT", textPos, 
            glm::vec2(9.0f, 13.0f), 
            glm::vec4(color_.r, color_.g, color_.b, 0.6f), 
            12.0f);
        
        // Indicador visual tenue
        renderer.drawCircle(center - glm::vec2(0.0f, 35.0f), 25.0f, 
            glm::vec4(color_.r, color_.g, color_.b, 0.2f), 32, false);
    }

} // namespace hud
