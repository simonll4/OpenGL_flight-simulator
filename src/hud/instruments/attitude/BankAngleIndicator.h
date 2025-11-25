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

        // Línea central del semicírculo en NDC (zona superior del HUD)
        static constexpr float NDC_CENTER_Y = -0.85f;

        // Longitud de la barra base y pendiente fija para dar sensación de arco
        static constexpr float NDC_LINE_WIDTH = 0.45f; // Un poco más ancho
        static constexpr float NDC_LINE_SLOPE = 0.15f; // Pendiente más pronunciada (15%) para menos "achatamiento"

        // Separación entre marcas (10°) y alturas diferenciadas para mayor/menor
        static constexpr float NDC_LINE_SPACING = 0.05f; // Un poco más de espacio
        static constexpr float DEGREES_PER_LINE = 10.0f;
        static constexpr float NDC_MARK_HEIGHT_MAJOR = 0.06f; // Marcas más altas
        static constexpr float NDC_MARK_HEIGHT_MINOR = 0.04f; // Marcas secundarias más visibles

        // Triángulo indicador (aguja)
        static constexpr float NDC_NEEDLE_OFFSET = 0.03f;
        static constexpr float NDC_TRIANGLE_SIZE = 0.020f;

        // Tamaño en píxeles para renderizar números +/-20°
        static constexpr float DIGIT_WIDTH = 8.0f;
        static constexpr float DIGIT_HEIGHT = 12.0f;

        // ====================================================================
        // MÉTODOS DE DIBUJO PRIVADOS
        // ====================================================================

        /**
         * @brief Dibuja la línea base inclinada fija
         *
         * La barra actúa como horizonte estático ligeramente inclinado.
         * Se deja deshabilitada por defecto para un HUD más limpio.
         */
        void drawBaseLine(gfx::Renderer2D &renderer, float centerX, float centerY);

        /**
         * @brief Dibuja las 5 marcas graduadas móviles y sus números
         *
         * Calcula la posición relativa de cada marca respecto al roll actual
         * y limita la visibilidad a dos marcas por lado para reducir ruido.
         */
        void drawGraduations(gfx::Renderer2D &renderer, float centerX, float centerY,
                             float leftX, float leftY, float rightX, float rightY,
                             float rollAngle);

        /**
         * @brief Dibuja el triángulo indicador (aguja) fijo
         *
         * Resalta el punto de referencia estático con un color más brillante.
         */
        void drawNeedle(gfx::Renderer2D &renderer, float centerX, float centerY);

        /**
         * @brief Convierte coordenadas NDC a píxeles dentro del viewport asignado
         * @param ndcX Coordenada X en NDC (-1.0 a 1.0)
         * @param ndcY Coordenada Y en NDC (-1.0 a 1.0)
         * @return Coordenada en píxeles relativa a position_/size_
         *
         * Aprovecha la caja asignada al instrumento para escalar valores
         * -1..1 a offsets en pantalla respetando el origen superior izquierdo.
         */
        glm::vec2 ndcToPixels(float ndcX, float ndcY) const;

        /**
         * @brief Normaliza ángulo de roll a rango -180° a 180°
         *
         * Evita saltos bruscos cuando el simulador reporta valores > 360°.
         */
        float normalizeRoll(float roll) const;
    };

} // namespace hud
