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
        // Usar el mismo verde translúcido que el resto del HUD
        color_ = glm::vec4(0.0f, 1.0f, 0.4f, 0.95f);
    }

    void PitchLadder::render(gfx::Renderer2D &renderer, const flight::FlightData &flightData)
    {
        if (!enabled_)
            return;

        // Dimensiones del viewport asignado
        float viewportWidth = size_.x;
        float viewportHeight = size_.y;

        // Centro geométrico de la caja del instrumento
        float centerX = position_.x + viewportWidth * 0.5f;
        float centerY = position_.y + viewportHeight * 0.5f;

        // Dibujar mira central (fija)
        drawCrosshair(renderer, centerX, centerY);

        // Dibujar líneas de pitch (dinámicas)
        drawPitchLines(renderer, centerX, centerY, flightData.pitch, flightData.roll);
    }

    void PitchLadder::drawCrosshair(gfx::Renderer2D &renderer, float centerX, float centerY)
    {
        // Convertir radio del círculo a píxeles
        float radiusPx = ndcDimensionToPixels(NDC_CIRCLE_RADIUS, size_.y);

        // Dibujar círculo central (outline) que sirve como aimpoint
        renderer.drawCircle(glm::vec2(centerX, centerY), radiusPx, color_, CIRCLE_SEGMENTS, false);

        // Líneas laterales saliendo del círculo (representan línea de horizonte local)
        float lineLengthPx = ndcDimensionToPixels(NDC_LATERAL_LINE_LENGTH, size_.x);

        // Línea lateral izquierda
        glm::vec2 leftStart(centerX - radiusPx, centerY);
        glm::vec2 leftEnd(centerX - radiusPx - lineLengthPx, centerY);
        renderer.drawLine(leftStart, leftEnd, color_, 2.0f);

        // Línea lateral derecha
        glm::vec2 rightStart(centerX + radiusPx, centerY);
        glm::vec2 rightEnd(centerX + radiusPx + lineLengthPx, centerY);
        renderer.drawLine(rightStart, rightEnd, color_, 2.0f);
    }

    void PitchLadder::drawPitchLines(gfx::Renderer2D &renderer, float centerX, float centerY, float pitchAngle, float rollAngle)
    {
        // Calcular índice de línea central basado en el pitch actual
        int centerLineIndex = static_cast<int>(std::round(pitchAngle / PITCH_STEP));

        // Dibujar 5 líneas: 2 arriba, 1 central, 2 abajo del pitch actual para mantener claridad
        for (int i = centerLineIndex - 2; i <= centerLineIndex + 2; ++i)
        {
            float pitchLineAngle = i * PITCH_STEP;

            // Solo generar si está dentro del rango visual razonable
            if (pitchLineAngle >= -MAX_PITCH_DISPLAY && pitchLineAngle <= MAX_PITCH_DISPLAY)
            {
                drawSinglePitchLine(renderer, centerX, centerY, pitchLineAngle, pitchAngle, rollAngle);
            }
        }
    }

    void PitchLadder::drawSinglePitchLine(gfx::Renderer2D &renderer, float centerX, float centerY,
                                          float pitchLineAngle, float currentPitch, float rollAngle)
    {
        // Diferencia entre la división deseada y el pitch actual
        float pitchDiff = pitchLineAngle - currentPitch;
        float lineYNDC = pitchDiff * NDC_PER_DEGREE; // 1% NDC por grado

        // Solo mostrar líneas dentro del rango visible
        if (std::abs(lineYNDC) > NDC_VISIBILITY_LIMIT)
            return;

        // Convertir posición Y de NDC a píxeles (relativa al centro)
        float lineY = centerY - (lineYNDC * size_.y * 0.5f); // Invertir porque Y crece hacia abajo

        // Determinar ancho de la línea (0° es más larga)
        float lineWidthNDC = (pitchLineAngle == 0.0f) ? NDC_LINE_WIDTH_ZERO : NDC_LINE_WIDTH_NORMAL;
        float lineWidthPx = ndcDimensionToPixels(lineWidthNDC, size_.x);
        float gapPx = ndcDimensionToPixels(NDC_GAP, size_.x);

        // Precalcular seno y coseno para rotación
        // Convertir roll a radianes (negativo porque la rotación en pantalla es horaria para roll positivo)
        // Nota: En simuladores, roll positivo es alabeo a la derecha. Visualmente el horizonte debe girar a la IZQUIERDA.
        float rollRad = glm::radians(-rollAngle);
        float cosRoll = std::cos(rollRad);
        float sinRoll = std::sin(rollRad);

        // Función lambda para rotar un punto alrededor de (centerX, centerY)
        auto rotatePoint = [&](glm::vec2 p) -> glm::vec2 {
            float dx = p.x - centerX;
            float dy = p.y - centerY;
            return glm::vec2(
                centerX + dx * cosRoll - dy * sinRoll,
                centerY + dx * sinRoll + dy * cosRoll
            );
        };

        // Calcular extremos de las dos partes de la línea (cortada para dejar visible la mira)
        // Parte izquierda: desde -lineWidth hasta -gap
        glm::vec2 leftStartRaw(centerX - lineWidthPx, lineY);
        glm::vec2 leftEndRaw(centerX - gapPx, lineY);

        // Parte derecha: desde gap hasta lineWidth
        glm::vec2 rightStartRaw(centerX + gapPx, lineY);
        glm::vec2 rightEndRaw(centerX + lineWidthPx, lineY);

        // Aplicar rotación
        glm::vec2 leftStart = rotatePoint(leftStartRaw);
        glm::vec2 leftEnd = rotatePoint(leftEndRaw);
        glm::vec2 rightStart = rotatePoint(rightStartRaw);
        glm::vec2 rightEnd = rotatePoint(rightEndRaw);

        // Dibujar las dos partes de la línea horizontal
        renderer.drawLine(leftStart, leftEnd, color_, 2.0f);
        renderer.drawLine(rightStart, rightEnd, color_, 2.0f);

        // Si no es la línea central (0°), agregar marcadores en los extremos
        if (pitchLineAngle != 0.0f)
        {
            float markerSizePx = ndcDimensionToPixels(NDC_MARKER_SIZE, size_.y);
            bool isPositive = (pitchLineAngle > 0.0f);

            // Marcador izquierdo (vertical)
            glm::vec2 leftMarkerStartRaw(centerX - lineWidthPx, lineY);
            glm::vec2 leftMarkerEndRaw(centerX - lineWidthPx,
                                    lineY + (isPositive ? markerSizePx : -markerSizePx));
            
            // Marcador derecho (vertical)
            glm::vec2 rightMarkerStartRaw(centerX + lineWidthPx, lineY);
            glm::vec2 rightMarkerEndRaw(centerX + lineWidthPx,
                                     lineY + (isPositive ? markerSizePx : -markerSizePx));

            // Aplicar rotación a marcadores
            renderer.drawLine(rotatePoint(leftMarkerStartRaw), rotatePoint(leftMarkerEndRaw), color_, 2.0f);
            renderer.drawLine(rotatePoint(rightMarkerStartRaw), rotatePoint(rightMarkerEndRaw), color_, 2.0f);

            // Convención HUD: marcadores apuntan hacia arriba para pitch positivo (cielo)
            // y hacia abajo para pitch negativo (suelo) para transmitir sentido visual.
        }
    }

    float PitchLadder::ndcDimensionToPixels(float ndcDim, float axisLength) const
    {
        // Escala un porcentaje de NDC a la cantidad equivalente en píxeles
        return ndcDim * axisLength * 0.5f;
    }

} // namespace hud
