#pragma once
#include "../../core/Instrument.h"
#include "../../../gfx/rendering/Renderer2D.h"
#include "../../../gfx/rendering/TextRenderer.h"
#include "../../../flight/data/FlightData.h"
#include <glm/glm.hpp>

namespace hud
{
    /**
     * @class BankAngleIndicator
     * @brief Indicador de ángulo de alabeo (bank angle/roll)
     * 
     * Muestra el ángulo de inclinación lateral del avión mediante:
     * - Línea base inclinada fija (ligera pendiente ascendente)
     * - Triángulo indicador fijo (aguja) en el centro
     * - 5 marcas graduadas móviles (cada 10°)
     * - Números cada 20° para referencia
     * 
     * Basado en UI::BankAngleIndicator del proyecto computacion_grafica/simulador,
     * adaptado para usar Renderer2D y coordenadas en píxeles.
     */
    class BankAngleIndicator : public Instrument
    {
    public:
        BankAngleIndicator();
        ~BankAngleIndicator() = default;

        /**
         * @brief Renderiza el indicador de bank angle
         * @param renderer Renderer2D compartido
         * @param flightData Datos de vuelo (usa flightData.roll)
         */
        void render(gfx::Renderer2D &renderer, const flight::FlightData &flightData) override;

    private:
        // ====================================================================
        // CONSTANTES DE CONFIGURACIÓN
        // ====================================================================

        // Posición vertical del indicador en NDC (será convertida a píxeles)
        static constexpr float NDC_CENTER_Y = -0.85f;

        // Dimensiones de la línea base en NDC
        static constexpr float NDC_LINE_WIDTH = 0.4f;
        static constexpr float NDC_LINE_SLOPE = 0.05f; // Pendiente 5%

        // Graduaciones
        static constexpr float NDC_LINE_SPACING = 0.045f;
        static constexpr float DEGREES_PER_LINE = 10.0f;
        static constexpr float NDC_MARK_HEIGHT_MAJOR = 0.04f;
        static constexpr float NDC_MARK_HEIGHT_MINOR = 0.025f;

        // Triángulo indicador (aguja)
        static constexpr float NDC_NEEDLE_OFFSET = 0.03f;
        static constexpr float NDC_TRIANGLE_SIZE = 0.020f;

        // Tamaño de dígitos para números
        static constexpr float DIGIT_WIDTH = 8.0f;
        static constexpr float DIGIT_HEIGHT = 12.0f;

        // ====================================================================
        // MÉTODOS DE DIBUJO PRIVADOS
        // ====================================================================

        /**
         * @brief Dibuja la línea base inclinada fija
         */
        void drawBaseLine(gfx::Renderer2D &renderer, float centerX, float centerY);

        /**
         * @brief Dibuja las 5 marcas graduadas móviles y sus números
         */
        void drawGraduations(gfx::Renderer2D &renderer, float centerX, float centerY,
                             float leftX, float leftY, float rightX, float rightY,
                             float rollAngle);

        /**
         * @brief Dibuja el triángulo indicador (aguja) fijo
         */
        void drawNeedle(gfx::Renderer2D &renderer, float centerX, float centerY);

        /**
         * @brief Convierte coordenadas NDC a píxeles dentro del viewport asignado
         * @param ndcX Coordenada X en NDC (-1.0 a 1.0)
         * @param ndcY Coordenada Y en NDC (-1.0 a 1.0)
         * @return Coordenada en píxeles relativa a position_/size_
         */
        glm::vec2 ndcToPixels(float ndcX, float ndcY) const;

        /**
         * @brief Normaliza ángulo de roll a rango -180° a 180°
         */
        float normalizeRoll(float roll) const;
    };

} // namespace hud
