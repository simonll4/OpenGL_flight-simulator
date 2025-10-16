#include "SpeedIndicator.h"
#include <cmath>

namespace hud
{
    // ============================================================================
    // CONFIGURACIÓN DE LA ESCALA DEL TAPE
    // ============================================================================

    static const float SPEED_STEP = 10.0f;      // Marcas cada 10 nudos
    static const float PIXELS_PER_STEP = 30.0f; // Separación vertical entre marcas
    static const int VISIBLE_MARKS = 12;        // Cuántas marcas mostrar arriba/abajo

    // Configuración visual
    static const float TICK_LENGTH = 16.0f;
    static const float TICK_TO_NUMBER_GAP = 6.0f;
    static const float READOUT_BOX_WIDTH = 100.0f;
    static const float READOUT_BOX_HEIGHT = 44.0f;
    static const float CHEVRON_WIDTH = 10.0f;
    static const float CHEVRON_HEIGHT = 12.0f;

    SpeedIndicator::SpeedIndicator() : Instrument()
    {
        // Configuración específica del indicador de velocidad
        size_ = glm::vec2(100.0f, 400.0f);
        color_ = glm::vec4(0.0f, 1.0f, 0.4f, 0.95f); // Verde HUD
    }

    // ============================================================================
    // FUNCIÓN PRINCIPAL DE RENDERIZADO
    // ============================================================================

    void SpeedIndicator::render(gfx::Renderer2D &renderer, const flight::FlightData &flightData)
    {
        // Solo renderizar si está habilitado
        if (!enabled_)
            return;

        float airspeed = flightData.airspeed;

        drawSpeedTape(renderer, airspeed);
        drawCurrentSpeedBox(renderer, airspeed);
    }

    // ============================================================================
    // RENDERIZADO DEL TAPE DE VELOCIDAD
    // ============================================================================

    void SpeedIndicator::drawSpeedTape(gfx::Renderer2D &renderer, float airspeed)
    {
        // Calcular centro vertical del instrumento (heredado de Instrument)
        float centerY = position_.y + size_.y * 0.5f;
        float ticksX = position_.x + 15.0f; // Columna de ticks a la izquierda

        // Calcular desplazamiento del tape
        float baseSpeed = floor(airspeed / SPEED_STEP) * SPEED_STEP;
        float fraction = (airspeed - baseSpeed) / SPEED_STEP;
        float scrollOffset = fraction * PIXELS_PER_STEP;

        // Dibujar marcas de velocidad visibles
        for (int i = -VISIBLE_MARKS; i <= VISIBLE_MARKS; ++i)
        {
            int markSpeed = (int)baseSpeed + i * (int)SPEED_STEP;
            
            // Saltar velocidades negativas
            if (markSpeed < 0)
                continue;

            float markY = centerY + scrollOffset - i * PIXELS_PER_STEP;

            // Culling de marcas fuera del área visible
            const float CULLING_MARGIN = 30.0f;
            if (markY < position_.y - CULLING_MARGIN || markY > position_.y + size_.y + CULLING_MARGIN)
                continue;

            // Saltar marcas dentro de la caja de lectura
            bool insideReadoutBox = (markY > centerY - READOUT_BOX_HEIGHT * 0.5f &&
                                     markY < centerY + READOUT_BOX_HEIGHT * 0.5f);
            if (insideReadoutBox)
                continue;

            // Dibujar tick
            renderer.drawRect(
                glm::vec2(ticksX, markY - 0.5f),
                glm::vec2(TICK_LENGTH, 1.0f),
                color_,
                true);

            // Dibujar número (solo cada 20 nudos para no saturar)
            if (markSpeed % 20 == 0)
            {
                float numberX = ticksX + TICK_LENGTH + TICK_TO_NUMBER_GAP + 20.0f;
                glm::vec2 numberPos = glm::vec2(numberX, markY);
                drawSpeedNumber(renderer, markSpeed, numberPos);
            }
        }
    }

    // ============================================================================
    // CAJA DE LECTURA DIGITAL (CENTRO)
    // ============================================================================

    void SpeedIndicator::drawCurrentSpeedBox(gfx::Renderer2D &renderer, float airspeed)
    {
        float centerY = position_.y + size_.y * 0.5f;

        // Posicionar la caja en el centro
        float boxX = position_.x + (size_.x - READOUT_BOX_WIDTH) * 0.5f;
        float boxY = centerY - READOUT_BOX_HEIGHT * 0.5f;

        // Dibujar marco de la caja
        renderer.drawRect(
            glm::vec2(boxX, boxY),
            glm::vec2(READOUT_BOX_WIDTH, READOUT_BOX_HEIGHT),
            color_,
            false // Solo borde
        );

        // Dibujar chevron a la derecha de la caja
        float chevronX = boxX + READOUT_BOX_WIDTH;
        float chevronTopY = centerY - CHEVRON_HEIGHT * 0.5f;
        float chevronBotY = centerY + CHEVRON_HEIGHT * 0.5f;

        renderer.drawLine(
            glm::vec2(chevronX, chevronTopY),
            glm::vec2(chevronX + CHEVRON_WIDTH, centerY),
            color_, 2.0f);
        renderer.drawLine(
            glm::vec2(chevronX + CHEVRON_WIDTH, centerY),
            glm::vec2(chevronX, chevronBotY),
            color_, 2.0f);
        renderer.drawLine(
            glm::vec2(chevronX, chevronTopY),
            glm::vec2(chevronX, chevronBotY),
            color_, 2.0f);

        // Mostrar velocidad actual redondeada
        int displaySpeed = (int)round(airspeed);
        if (displaySpeed < 0)
            displaySpeed = 0;

        // Dibujar el número centrado
        glm::vec2 numberPos = glm::vec2(boxX + READOUT_BOX_WIDTH * 0.5f, centerY);
        drawSpeedNumber(renderer, displaySpeed, numberPos);
    }

    // ============================================================================
    // RENDERIZADO DE NÚMEROS SIMPLES
    // ============================================================================

    void SpeedIndicator::drawSpeedNumber(gfx::Renderer2D &renderer, int speed, const glm::vec2 &position)
    {
        // Por simplicidad, usa rectángulos para formar números básicos
        // En una implementación completa, se usaría 7-segmentos como en Altimeter
        
        std::string speedStr = std::to_string(speed);
        
        const float digitWidth = 8.0f;
        const float digitHeight = 12.0f;
        const float digitSpacing = 10.0f;
        
        float totalWidth = speedStr.length() * digitSpacing;
        float startX = position.x - totalWidth * 0.5f;
        
        // Dibujar cada dígito (versión simplificada - solo rectángulos como placeholder)
        for (size_t i = 0; i < speedStr.length(); ++i)
        {
            float digitX = startX + i * digitSpacing;
            float digitY = position.y - digitHeight * 0.5f;
            
            // Placeholder: dibujar un pequeño rectángulo
            renderer.drawRect(
                glm::vec2(digitX, digitY),
                glm::vec2(digitWidth, digitHeight),
                color_,
                false);
        }
    }

} // namespace hud
