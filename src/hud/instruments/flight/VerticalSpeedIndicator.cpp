#include "VerticalSpeedIndicator.h"
#include "../../../gfx/rendering/TextRenderer.h"
#include <cmath>
#include <algorithm>

namespace hud
{
    // ============================================================================
    // CONFIGURACIÓN VISUAL
    // ============================================================================

    // Dimensiones de la escala (compacta y profesional)
    static const float SCALE_WIDTH = 54.0f;        // Ancho de la escala vertical
    static const float SCALE_HEIGHT = 225.0f;      // Altura total de la escala (50% de tapes)
    static const float TICK_LENGTH = 8.0f;         // Longitud de las marcas laterales (reducida)
    static const float MAJOR_TICK_LENGTH = 12.0f;  // Marcas principales (0, ±20, ±40, ±60)

    // Indicador triangular (compacto)
    static const float INDICATOR_WIDTH = 10.0f;
    static const float INDICATOR_HEIGHT = 8.0f;

    // Caja de lectura digital (consistente con otros instrumentos)
    static const float READOUT_BOX_WIDTH = 48.0f;   // Más estrecha (números más cortos)
    static const float READOUT_BOX_HEIGHT = 24.0f;

    VerticalSpeedIndicator::VerticalSpeedIndicator() : Instrument()
    {
        // Configuración específica del VSI
        size_ = glm::vec2(SCALE_WIDTH, SCALE_HEIGHT);
        color_ = glm::vec4(0.0f, 1.0f, 0.4f, 0.95f); // Verde HUD
    }

    // ============================================================================
    // FUNCIÓN PRINCIPAL DE RENDERIZADO
    // ============================================================================

    void VerticalSpeedIndicator::render(gfx::Renderer2D &renderer, const flight::FlightData &flightData)
    {
        if (!enabled_)
            return;

        float verticalSpeed = flightData.verticalSpeed; // ft/min

        // Clamping para evitar que el indicador salga de la escala
        // (valores extremos se muestran en el límite)
        float clampedVS = std::clamp(verticalSpeed, MIN_VSI, MAX_VSI);

        drawScale(renderer);
        drawIndicator(renderer, clampedVS);
        drawDigitalReadout(renderer, verticalSpeed); // Mostrar valor real (no clamped)
    }

    // ============================================================================
    // RENDERIZADO DE LA ESCALA FIJA
    // ============================================================================

    void VerticalSpeedIndicator::drawScale(gfx::Renderer2D &renderer)
    {
        float centerX = position_.x + size_.x * 0.5f;
        float centerY = position_.y + size_.y * 0.5f;

        // Línea vertical de la escala (backbone)
        float lineX = centerX + 5.0f; // Offset a la derecha del centro
        renderer.drawLine(
            glm::vec2(lineX, position_.y),
            glm::vec2(lineX, position_.y + size_.y),
            color_, 1.0f);

        // Dibujar marcas de la escala
        int numMarks = static_cast<int>((MAX_VSI - MIN_VSI) / MARK_INTERVAL) + 1;

        for (int i = 0; i < numMarks; ++i)
        {
            float vsi = MIN_VSI + i * MARK_INTERVAL; // En ft/min

            // Calcular posición Y de la marca (invertida: +arriba, -abajo)
            float normalizedVSI = vsi / MAX_VSI; // -1.0 a +1.0
            float markY = centerY - normalizedVSI * (SCALE_HEIGHT * 0.5f);

            // Determinar si es marca principal (cada 2000 ft/min = 20 en display)
            bool isMajorMark = (std::fmod(std::abs(vsi), 2000.0f) < 0.1f);

            float tickLen = isMajorMark ? MAJOR_TICK_LENGTH : TICK_LENGTH;

            // Dibujar tick horizontal
            renderer.drawLine(
                glm::vec2(lineX - tickLen, markY),
                glm::vec2(lineX, markY),
                color_, isMajorMark ? 1.25f : 0.8f);

            // Dibujar números solo en marcas principales
            if (isMajorMark)
            {
                // Formatear número en formato estándar militar: ft/min / 100
                // Ejemplo: 4000 ft/min → "+40"
                int displayValue = static_cast<int>(vsi / DISPLAY_SCALE);
                
                std::string label;
                if (displayValue == 0)
                {
                    label = "0";
                }
                else
                {
                    // Agregar signo explícito
                    label = (displayValue > 0 ? "+" : "") + std::to_string(displayValue);
                }

                // Posición del texto (a la izquierda de los ticks)
                float textX = lineX - tickLen - 18.0f;
                glm::vec2 textPos = glm::vec2(textX, markY);
                gfx::TextRenderer::drawString(renderer, label, textPos,
                                              glm::vec2(5.0f, 8.0f), color_, 7.0f);
            }
        }

        // Línea de referencia en 0 (más gruesa y más larga)
        renderer.drawLine(
            glm::vec2(lineX - (MAJOR_TICK_LENGTH + 12.0f), centerY),
            glm::vec2(lineX + 6.0f, centerY),
            color_, 3.0f);
    }

    // ============================================================================
    // INDICADOR TRIANGULAR MÓVIL
    // ============================================================================

    void VerticalSpeedIndicator::drawIndicator(gfx::Renderer2D &renderer, float verticalSpeed)
    {
        float centerX = position_.x + size_.x * 0.5f;
        float centerY = position_.y + size_.y * 0.5f;

        // Calcular posición Y del indicador basado en VSI
        float normalizedVSI = verticalSpeed / MAX_VSI; // -1.0 a +1.0
        float indicatorY = centerY - normalizedVSI * (SCALE_HEIGHT * 0.5f);

        // Triángulo apuntando a la escala (punta a la derecha)
        float lineX = centerX + 5.0f;
        float triLeft = lineX + 2.0f;
        float triRight = triLeft + INDICATOR_WIDTH;
        float triTop = indicatorY - INDICATOR_HEIGHT * 0.5f;
        float triBot = indicatorY + INDICATOR_HEIGHT * 0.5f;

        // Dibujar triángulo relleno
        renderer.drawTriangle(
            glm::vec2(triLeft, triTop),
            glm::vec2(triLeft, triBot),
            glm::vec2(triRight, indicatorY),
            color_);

        // Contorno del triángulo (más fino)
        renderer.drawLine(glm::vec2(triLeft, triTop), glm::vec2(triRight, indicatorY), color_, 1.2f);
        renderer.drawLine(glm::vec2(triRight, indicatorY), glm::vec2(triLeft, triBot), color_, 1.2f);
        renderer.drawLine(glm::vec2(triLeft, triBot), glm::vec2(triLeft, triTop), color_, 1.2f);
    }

    // ============================================================================
    // LECTURA DIGITAL (FORMATO ESTÁNDAR MILITAR)
    // ============================================================================

    void VerticalSpeedIndicator::drawDigitalReadout(gfx::Renderer2D &renderer, float verticalSpeed)
    {
        float centerX = position_.x + size_.x * 0.5f;
        float centerY = position_.y + size_.y * 0.5f;

        // Alinear la caja digital con la línea de 0 y a la IZQUIERDA de la escala
        float lineX = centerX + 5.0f; // Debe coincidir con drawScale
        float boxX = lineX - 6.0f - READOUT_BOX_WIDTH; // Separación mínima de la escala
        float boxY = centerY - READOUT_BOX_HEIGHT * 0.5f;

        // Fondo semi-transparente para legibilidad
        glm::vec4 boxBgColor = glm::vec4(0.0f, 0.0f, 0.0f, 0.7f);
        renderer.drawRect(
            glm::vec2(boxX, boxY),
            glm::vec2(READOUT_BOX_WIDTH, READOUT_BOX_HEIGHT),
            boxBgColor, true);

        // Contorno de la caja
        renderer.drawRect(
            glm::vec2(boxX, boxY),
            glm::vec2(READOUT_BOX_WIDTH, READOUT_BOX_HEIGHT),
            color_, false);

        // FORMATO ESTÁNDAR MILITAR: ft/min / 100
        // Ejemplo: 4000 ft/min → "+40"
        // Ejemplo: -1200 ft/min → "-12"
        // Ejemplo: +500 ft/min → "+5"
        int displayValue = static_cast<int>(std::round(verticalSpeed / DISPLAY_SCALE));
        
        std::string vsiText;
        if (displayValue == 0)
        {
            vsiText = "0";
        }
        else
        {
            // Signo explícito + valor absoluto (sin decimales, formato entero compacto)
            vsiText = (displayValue > 0 ? "+" : "") + std::to_string(displayValue);
        }

        // Dibujar número centrado en la caja (consistente con otros instrumentos)
        glm::vec2 textPos = glm::vec2(boxX + READOUT_BOX_WIDTH * 0.5f, centerY);
        gfx::TextRenderer::drawString(renderer, vsiText, textPos, 
                                      glm::vec2(6.0f, 10.0f), color_, 8.0f);
    }

} // namespace hud
