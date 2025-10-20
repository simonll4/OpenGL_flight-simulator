#pragma once
#include <string>
#include <glm/glm.hpp>
#include "Renderer2D.h"

namespace gfx
{
    /**
     * @class TextRenderer
     * @brief Utilidad para renderizar texto usando un estilo de 7 segmentos.
     *
     * Esta clase proporciona métodos estáticos para dibujar caracteres y cadenas
     * de texto en la pantalla, utilizando un Renderer2D para las operaciones
     * de dibujo de bajo nivel.
     */
    class TextRenderer
    {
    public:
        /**
         * @brief Dibuja una cadena de texto en la pantalla.
         *
         * @param renderer El renderizador 2D a utilizar.
         * @param text La cadena de texto a dibujar.
         * @param position La posición central del texto.
         * @param charSize El tamaño (ancho, alto) de cada caracter.
         * @param color El color del texto.
         * @param spacing El espaciado horizontal total para cada caracter (advance).
         */
        static void drawString(
            Renderer2D &renderer,
            const std::string &text,
            const glm::vec2 &position,
            const glm::vec2 &charSize,
            const glm::vec4 &color,
            float spacing);

    private:
        /**
         * @brief Dibuja un único caracter en estilo de 7 segmentos.
         *
         * @param renderer El renderizador 2D a utilizar.
         * @param character El caracter a dibujar.
         * @param pos La posición de la esquina superior izquierda del caracter.
         * @param size El tamaño (ancho, alto) del caracter.
         * @param color El color del caracter.
         */
        static void drawChar7Segment(
            Renderer2D &renderer,
            char character,
            const glm::vec2 &pos,
            const glm::vec2 &size,
            const glm::vec4 &color);
    };

} // namespace gfx
