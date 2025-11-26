/**
 * @file TextRenderer.h
 * @brief 7-segment display style text renderer.
 */

#pragma once
#include <string>
#include <glm/glm.hpp>
#include "Renderer2D.h"

namespace gfx
{
    /**
     * @class TextRenderer
     * @brief Static utility for 7-segment display style text.
     *
     * Relies on `Renderer2D` to draw rectangular blocks simulating lit segments.
     * All logic is stateless: position and size are defined by parameters in each call.
     */
    class TextRenderer
    {
    public:
        /**
         * @brief Draws a text string on the screen.
         *
         * @param renderer The 2D renderer to use.
         * @param text The text string to draw.
         * @param position The center position of the text.
         * @param charSize The size (width, height) of each character.
         * @param color The text color.
         * @param spacing The total horizontal spacing for each character (advance).
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
         * @brief Draws a single character in 7-segment style.
         *
         * @param renderer The 2D renderer to use.
         * @param character The character to draw.
         * @param pos The top-left position of the character.
         * @param size The size (width, height) of the character.
         * @param color The character color.
         */
        static void drawChar7Segment(
            Renderer2D &renderer,
            char character,
            const glm::vec2 &pos,
            const glm::vec2 &size,
            const glm::vec4 &color);
    };

} // namespace gfx
