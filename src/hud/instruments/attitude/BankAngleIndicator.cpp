#include "BankAngleIndicator.h"
#include <cmath>
#include <string>

namespace hud
{

    BankAngleIndicator::BankAngleIndicator() : Instrument()
    {
        // Color base ligeramente más turquesa para destacar sobre el HUD
        color_ = glm::vec4(0.0f, 1.0f, 0.4f, 0.95f);
    }

    void BankAngleIndicator::render(gfx::Renderer2D &renderer, const flight::FlightData &flightData)
    {
        if (!enabled_)
            return;

        // Normalizar ángulo de roll para evitar saltos cuando supera 360°/-360°
        float rollAngle = normalizeRoll(flightData.roll);

        // Convertir la posición del centro del instrumento desde NDC a píxeles
        glm::vec2 centerPx = ndcToPixels(0.0f, NDC_CENTER_Y);
        float centerX = centerPx.x;
        float centerY = centerPx.y;

        // Calcular extremos de la línea base inclinada en función del viewport
        glm::vec2 leftPx = ndcToPixels(-NDC_LINE_WIDTH * 0.5f,
                                       NDC_CENTER_Y - NDC_LINE_SLOPE * NDC_LINE_WIDTH * 0.5f);
        glm::vec2 rightPx = ndcToPixels(NDC_LINE_WIDTH * 0.5f,
                                        NDC_CENTER_Y + NDC_LINE_SLOPE * NDC_LINE_WIDTH * 0.5f);

        // Dibujar componentes del indicador
        // Nota: No dibujamos la línea base para mantener el HUD limpio
        // drawBaseLine(renderer, centerX, centerY);

        drawGraduations(renderer, centerX, centerY,
                        leftPx.x, leftPx.y, rightPx.x, rightPx.y,
                        rollAngle);

        drawNeedle(renderer, centerX, centerY);
    }

    void BankAngleIndicator::drawBaseLine(gfx::Renderer2D &renderer, float centerX, float centerY)
    {
        // Convertir extremos de la línea a píxeles
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
        // Índice de la marca alineada con el triángulo central
        int centerLineIndex = static_cast<int>(std::round(rollAngle / DEGREES_PER_LINE));

        // Dibujar solo 5 líneas: 2 a la izquierda, 1 central, 2 a la derecha
        int visibleLines = 0;
        for (int i = centerLineIndex - 2; i <= centerLineIndex + 2; ++i)
        {
            float lineAngle = i * DEGREES_PER_LINE;

            // Calcular diferencia angular y posición normalizada (t) en la línea
            float angleDiff = lineAngle - rollAngle;
            float t = 0.5f + (angleDiff / DEGREES_PER_LINE) * (NDC_LINE_SPACING / NDC_LINE_WIDTH);

            // Solo dibujar líneas dentro del rango visible
            if (t >= 0.0f && t <= 1.0f)
            {
                // Interpolar posición en la línea inclinada
                float lineX = leftX + t * (rightX - leftX);
                float lineY = leftY + t * (rightY - leftY);

                // Determinar altura de marca (mayor cada 30°, menor cada 10°)
                bool isMajor = (i % 3 == 0);
                float markHeightNDC = isMajor ? NDC_MARK_HEIGHT_MAJOR : NDC_MARK_HEIGHT_MINOR;

                // Convertir altura de NDC a píxeles (solo dimensión, no posición)
                float markHeightPx = markHeightNDC * size_.y * 0.5f;

                // Dibujar línea vertical de graduación
                glm::vec2 markTop(lineX, lineY - markHeightPx * 0.5f);
                glm::vec2 markBot(lineX, lineY + markHeightPx * 0.5f);
                renderer.drawLine(markTop, markBot, color_, 1.5f);

                // Mostrar texto cada 20° (múltiplos de 2 líneas de 10°), excluyendo 0°
                if (i % 2 == 0 && lineAngle != 0.0f)
                {
                    float textOffsetNDC = 0.035f;
                    float textOffsetPx = textOffsetNDC * size_.y * 0.5f;
                    glm::vec2 textPos(lineX, lineY + markHeightPx * 0.5f + textOffsetPx);

                    // Renderizar valor entero del ángulo usando el Renderer2D global
                    gfx::TextRenderer::drawString(
                        renderer,
                        std::to_string(static_cast<int>(lineAngle)),
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
        // Colocar la aguja ligeramente por debajo del centro
        float needleOffsetPx = NDC_NEEDLE_OFFSET * size_.y * 0.5f;
        float needleY = centerY - needleOffsetPx;

        // Tamaño del triángulo en píxeles
        float triangleSizePx = NDC_TRIANGLE_SIZE * size_.y * 0.5f;

        // Vértices del triángulo apuntando hacia arriba
        glm::vec2 tip(centerX, needleY + triangleSizePx);                                      // Punta
        glm::vec2 baseLeft(centerX - triangleSizePx * 0.6f, needleY - triangleSizePx * 0.3f);  // Base izquierda
        glm::vec2 baseRight(centerX + triangleSizePx * 0.6f, needleY - triangleSizePx * 0.3f); // Base derecha

        // Color más brillante para la aguja
        glm::vec4 needleColor = glm::vec4(0.0f, 1.0f, 0.2f, 1.0f);

        // Dibujar triángulo (outline)
        renderer.drawTriangle(tip, baseLeft, baseRight, needleColor, false);
    }

    glm::vec2 BankAngleIndicator::ndcToPixels(float ndcX, float ndcY) const
    {
        // NDC: -1.0 a 1.0, origen en centro
        // Pixels: 0 a width/height, origen en esquina superior izquierda

        float pixelX = position_.x + (ndcX + 1.0f) * size_.x * 0.5f;
        float pixelY = position_.y + (1.0f - ndcY) * size_.y * 0.5f; // Invertir Y

        return glm::vec2(pixelX, pixelY);
    }

    float BankAngleIndicator::normalizeRoll(float roll) const
    {
        // Normalizar a rango -180° a 180°
        float normalized = roll;
        while (normalized > 180.0f)
            normalized -= 360.0f;
        while (normalized < -180.0f)
            normalized += 360.0f;
        return normalized;
    }

} // namespace hud
