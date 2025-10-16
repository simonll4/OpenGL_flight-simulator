#pragma once
#include <glm/glm.hpp>
#include "../gfx/Renderer2D.h"
#include "../flight/FlightData.h"

namespace hud
{
    /**
     * @class Instrument
     * @brief Clase base abstracta para todos los instrumentos del HUD
     *
     * Esta clase define la interfaz común para todos los instrumentos de vuelo.
     * Proporciona:
     * - Propiedades comunes (posición, tamaño, color)
     * - Métodos de configuración (setters)
     * - Interfaz de renderizado (método virtual puro)
     *
     * Cada instrumento específico (Altimeter, AttitudeIndicator, etc.)
     * debe heredar de esta clase e implementar su propio método render().
     */
    class Instrument
    {
    public:
        /**
         * @brief Constructor por defecto
         * Inicializa el instrumento con valores predeterminados
         */
        Instrument();

        /**
         * @brief Destructor virtual para permitir polimorfismo
         */
        virtual ~Instrument() = default;

        // ====================================================================
        // MÉTODOS DE CONFIGURACIÓN COMUNES
        // ====================================================================

        /**
         * @brief Establece la posición del instrumento en coordenadas de pantalla
         * @param position Posición (x, y) de la esquina superior izquierda
         */
        void setPosition(const glm::vec2 &position) { position_ = position; }

        /**
         * @brief Establece el tamaño del instrumento
         * @param size Dimensiones (ancho, alto) del instrumento
         */
        void setSize(const glm::vec2 &size) { size_ = size; }

        /**
         * @brief Establece el color principal del instrumento
         * @param color Color RGBA (valores entre 0.0 y 1.0)
         */
        void setColor(const glm::vec4 &color) { color_ = color; }

        /**
         * @brief Habilita o deshabilita la visualización del instrumento
         * @param enabled true para mostrar, false para ocultar
         */
        void setEnabled(bool enabled) { enabled_ = enabled; }

        // ====================================================================
        // MÉTODOS DE ACCESO (GETTERS)
        // ====================================================================

        const glm::vec2 &getPosition() const { return position_; }
        const glm::vec2 &getSize() const { return size_; }
        const glm::vec4 &getColor() const { return color_; }
        bool isEnabled() const { return enabled_; }

        // ====================================================================
        // INTERFAZ DE RENDERIZADO
        // ====================================================================

        /**
         * @brief Renderiza el instrumento en pantalla
         * @param renderer Renderer 2D compartido
         * @param flightData Datos actuales del vuelo
         *
         * Este método debe ser implementado por cada instrumento específico.
         * Es responsable de dibujar todos los elementos visuales del instrumento.
         */
        virtual void render(gfx::Renderer2D &renderer, const flight::FlightData &flightData) = 0;

    protected:
        // ====================================================================
        // PROPIEDADES COMUNES A TODOS LOS INSTRUMENTOS
        // ====================================================================

        glm::vec2 position_; ///< Posición en pantalla (x, y)
        glm::vec2 size_;     ///< Tamaño del instrumento (ancho, alto)
        glm::vec4 color_;    ///< Color principal RGBA
        bool enabled_;       ///< Si el instrumento está activo/visible
    };

} // namespace hud
