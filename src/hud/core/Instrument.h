#pragma once
#include <glm/glm.hpp>
#include "../../gfx/rendering/Renderer2D.h"
#include "../../flight/data/FlightData.h"

namespace hud
{
    /**
     * @class Instrument
     * @brief Clase base abstracta para todos los instrumentos del HUD
     *
     * Esta clase define la interfaz común para todos los instrumentos de vuelo.
     * Proporciona:
     * - Propiedades comunes (posición, tamaño, color) expresadas en píxeles relativos al viewport del HUD
     * - Métodos de configuración (setters) para integrarse con los layouts calculados por FlightHUD
     * - Interfaz de renderizado (método virtual puro) que recibe el Renderer2D y los datos de vuelo ya actualizados
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
         *
         * La posición equivale al origen local que usa cada instrumento para convertir
         * coordenadas Normalized Device Coordinates (NDC) a píxeles.
         */
        void setPosition(const glm::vec2 &position) { position_ = position; }

        /**
         * @brief Establece el tamaño del instrumento
         * @param size Dimensiones (ancho, alto) del instrumento
         *
         * Los instrumentos usan este tamaño para escalar geometría proporcionalmente
         * y mantener una apariencia consistente en cualquier resolución.
         */
        void setSize(const glm::vec2 &size) { size_ = size; }

        /**
         * @brief Establece el color principal del instrumento
         * @param color Color RGBA (valores entre 0.0 y 1.0)
         *
         * Normalmente se usa un verde translúcido para imitar HUD reales, pero la
         * propiedad permite variaciones para estados especiales o temas alternos.
         */
        void setColor(const glm::vec4 &color) { color_ = color; }

        /**
         * @brief Habilita o deshabilita la visualización del instrumento
         * @param enabled true para mostrar, false para ocultar
         *
         * FlightHUD usa este flag para no invocar render() cuando un módulo está
         * configurado como opcional o temporalmente indisponible.
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
