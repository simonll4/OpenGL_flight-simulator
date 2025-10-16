#include "TextRenderer.h"
#include <cmath>

namespace gfx
{

    // ejemplo de uso:
    // gfx::TextRenderer::drawString(renderer, "123", glm::vec2(200, 100), glm::vec2(8, 12), glm::vec4(1,1,1,1), 10);
    // dibuja "123" centrado en (200,100) con caracteres de 8x12 píxeles y espaciado de 10 píxeles
    void TextRenderer::drawString(Renderer2D &renderer, const std::string &text, const glm::vec2 &position, const glm::vec2 &charSize, const glm::vec4 &color, float spacing)
    {
        // Centrar el string completo horizontalmente
        float totalWidth = text.length() * spacing - (spacing - charSize.x);
        float startX = position.x - totalWidth * 0.5f;

        // Dibujar cada dígito con spacing fijo (tabular)
        for (size_t i = 0; i < text.length(); ++i)
        {
            char character = text[i];

            // Posición del dígito con pixel snapping
            float charX = floor(startX + i * spacing) + 0.5f;
            float charY = floor(position.y - charSize.y * 0.5f) + 0.5f;
            glm::vec2 charPos = glm::vec2(charX, charY);

            // Renderizar 7-segmentos
            drawChar7Segment(renderer, character, charPos, charSize, color);
        }
    }

    void TextRenderer::drawChar7Segment(Renderer2D &renderer, char character, const glm::vec2 &pos, const glm::vec2 &size, const glm::vec4 &color)
    {
        const float w = size.x;
        const float h = size.y;
        const float thickness = 1.5f; // Grosor del segmento
        const float halfH = h * 0.5f;

        bool s[7] = {false}; // Segments: a, b, c, d, e, f, g

        // Convertir a mayúsculas para simplificar
        char c = toupper(character);

        switch (c)
        {
        // Números
        case '0':
            s[0] = s[1] = s[2] = s[3] = s[4] = s[5] = true;
            break;
        case '1':
            s[1] = s[2] = true;
            break;
        case '2':
            s[0] = s[1] = s[3] = s[4] = s[6] = true;
            break;
        case '3':
            s[0] = s[1] = s[2] = s[3] = s[6] = true;
            break;
        case '4':
            s[1] = s[2] = s[5] = s[6] = true;
            break;
        case '5':
            s[0] = s[2] = s[3] = s[5] = s[6] = true;
            break;
        case '6':
            s[0] = s[2] = s[3] = s[4] = s[5] = s[6] = true;
            break;
        case '7':
            s[0] = s[1] = s[2] = true;
            break;
        case '8':
            s[0] = s[1] = s[2] = s[3] = s[4] = s[5] = s[6] = true;
            break;
        case '9':
            s[0] = s[1] = s[2] = s[3] = s[5] = s[6] = true;
            break;

        // Letras (representaciones comunes en 7 segmentos)
        case 'A':
            s[0] = s[1] = s[2] = s[4] = s[5] = s[6] = true;
            break; // A
        case 'B':
            s[2] = s[3] = s[4] = s[5] = s[6] = true;
            break; // b (minúscula)
        case 'C':
            s[0] = s[3] = s[4] = s[5] = true;
            break; // C
        case 'D':
            s[1] = s[2] = s[3] = s[4] = s[6] = true;
            break; // d (minúscula)
        case 'E':
            s[0] = s[3] = s[4] = s[5] = s[6] = true;
            break; // E
        case 'F':
            s[0] = s[4] = s[5] = s[6] = true;
            break; // F
        case 'G':
            s[0] = s[2] = s[3] = s[4] = s[5] = true;
            break; // G
        case 'H':
            s[1] = s[2] = s[4] = s[5] = s[6] = true;
            break; // H
        case 'I':
            s[4] = s[5] = true;
            break; // I
        case 'J':
            s[1] = s[2] = s[3] = s[4] = true;
            break; // J
        case 'L':
            s[3] = s[4] = s[5] = true;
            break; // L
        case 'N':
            s[0] = s[1] = s[2] = s[4] = s[5] = true;
            break; // N
        case 'O':
            s[0] = s[1] = s[2] = s[3] = s[4] = s[5] = true;
            break; // O (como 0)
        case 'P':
            s[0] = s[1] = s[4] = s[5] = s[6] = true;
            break; // P
        case 'Q':
            s[0] = s[1] = s[2] = s[5] = s[6] = true;
            break; // q (minúscula)
        case 'R':
            s[4] = s[6] = true;
            break; // r (minúscula)
        case 'S':
            s[0] = s[2] = s[3] = s[5] = s[6] = true;
            break; // S (como 5)
        case 'T':
            s[3] = s[4] = s[5] = s[6] = true;
            break; // t (minúscula)
        case 'U':
            s[1] = s[2] = s[3] = s[4] = s[5] = true;
            break; // U
        case 'Y':
            s[1] = s[2] = s[3] = s[5] = s[6] = true;
            break; // Y

        // Símbolos
        case '-':
            s[6] = true;
            break; // Guión
        case '.':
            renderer.drawRect(pos + glm::vec2(w * 0.5f - thickness * 0.5f, h - thickness), glm::vec2(thickness, thickness), color, true);
            return; // Punto
        case ' ':
            break; // Espacio
        default:
            break; // Caracter no soportado
        }

        // Dibujar segmentos activos
        if (s[0])
            renderer.drawRect(pos + glm::vec2(thickness, 0), glm::vec2(w - 2 * thickness, thickness), color, true); // a
        if (s[1])
            renderer.drawRect(pos + glm::vec2(w - thickness, thickness), glm::vec2(thickness, halfH - thickness), color, true); // b
        if (s[2])
            renderer.drawRect(pos + glm::vec2(w - thickness, halfH), glm::vec2(thickness, halfH - thickness), color, true); // c
        if (s[3])
            renderer.drawRect(pos + glm::vec2(thickness, h - thickness), glm::vec2(w - 2 * thickness, thickness), color, true); // d
        if (s[4])
            renderer.drawRect(pos + glm::vec2(0, halfH), glm::vec2(thickness, halfH - thickness), color, true); // e
        if (s[5])
            renderer.drawRect(pos + glm::vec2(0, thickness), glm::vec2(thickness, halfH - thickness), color, true); // f
        if (s[6])
            renderer.drawRect(pos + glm::vec2(thickness, halfH - thickness * 0.5f), glm::vec2(w - 2 * thickness, thickness), color, true); // g
    }

} // namespace gfx
