#pragma once
#include "../../core/Instrument.h"
#include "../../../gfx/rendering/Renderer2D.h"
#include "../../../flight/data/FlightData.h"
#include <glm/glm.hpp>

namespace hud
{
    /**
     * @class PitchLadder
     * @brief Indicador de escalera de cabeceo (pitch ladder)
     *
     * Muestra el ángulo de pitch del avión mediante:
     * - Mira central fija (círculo + líneas laterales)
     * - 5 líneas horizontales dinámicas (cada 10°)
     * - Líneas cortadas en el centro con gap
     * - Marcadores verticales en los extremos (arriba = +pitch, abajo = -pitch)
     *
     * Basado en UI::PitchLadder del proyecto computacion_grafica/simulador,
     * adaptado para usar Renderer2D y coordenadas en píxeles.
     */
    class PitchLadder : public Instrument
    {
    public:
        PitchLadder();
        ~PitchLadder() = default;

        /**
         * @brief Renderiza la escalera de pitch
         * @param renderer Renderer2D compartido
         * @param flightData Datos de vuelo (usa flightData.pitch)
         */
        void render(gfx::Renderer2D &renderer, const flight::FlightData &flightData) override;

    private:
        // ====================================================================
        // CONSTANTES DE CONFIGURACIÓN
        // ====================================================================

        // Configuración de líneas de pitch
        static constexpr float PITCH_STEP = 10.0f;        // Cada línea representa 10°
        static constexpr float MAX_PITCH_DISPLAY = 40.0f; // Mostrar hasta ±40°

        // Mira central (círculo + líneas laterales) en NDC
        static constexpr float NDC_CIRCLE_RADIUS = 0.01f;       // Radio del círculo apuntado
        static constexpr float NDC_LATERAL_LINE_LENGTH = 0.04f; // Extensión de las líneas guía
        static constexpr int CIRCLE_SEGMENTS = 16;              // Segmentos para aproximar el círculo

        // Líneas de pitch en NDC
        static constexpr float NDC_LINE_WIDTH_NORMAL = 0.1f; // Longitud para ±10°, ±20°, etc.
        static constexpr float NDC_LINE_WIDTH_ZERO = 0.15f;  // Línea 0° más larga
        static constexpr float NDC_GAP = 0.03f;              // Separación en el centro
        static constexpr float NDC_MARKER_SIZE = 0.02f;      // Marcadores extremos

        // Factor de conversión: 1% NDC por grado de pitch
        static constexpr float NDC_PER_DEGREE = 0.01f;

        // Límite de visibilidad en NDC (±0.8)
        static constexpr float NDC_VISIBILITY_LIMIT = 0.8f;

        // ====================================================================
        // MÉTODOS DE DIBUJO PRIVADOS
        // ====================================================================

        /**
         * @brief Dibuja la mira central fija (círculo + líneas laterales)
         *
         * Esta mira sirve como referencia absoluta; no depende del pitch.
         */
        void drawCrosshair(gfx::Renderer2D &renderer, float centerX, float centerY);

        /**
         * @brief Dibuja las 5 líneas de pitch dinámicas
         *
         * Calcula las líneas visibles alrededor del pitch actual y limita
         * la salida a ±2 divisiones para evitar saturar la pantalla.
         */
        void drawPitchLines(gfx::Renderer2D &renderer, float centerX, float centerY, float pitchAngle);

        /**
         * @brief Dibuja una línea de pitch individual
         * @param pitchLineAngle Ángulo que representa esta línea (ej: 10°, 20°, etc.)
         * @param currentPitch Pitch actual del avión
         *
         * Se encarga de posicionar, recortar con gap central y colocar
         * marcadores verticales que distinguen entre pitch positivo/negativo.
         */
        void drawSinglePitchLine(gfx::Renderer2D &renderer, float centerX, float centerY,
                                 float pitchLineAngle, float currentPitch);

        /**
         * @brief Convierte dimensión NDC a píxeles (sin inversión de Y)
         *
         * Utiliza el tamaño del viewport en cada eje para escalar magnitudes
         * relativas (-1..1) a longitudes concretas en pantalla.
         */
        float ndcDimensionToPixels(float ndcDim, float axisLength) const;
    };

} // namespace hud
