#include "TextRenderer.h"
#include <cmath>

namespace gfx
{

    ////////////////////////////////////////////////////////////////////////////
    //  Public API
    ////////////////////////////////////////////////////////////////////////////

    // Quick example:
    // gfx::TextRenderer::drawString(renderer, "123", {200, 100}, {8, 12}, {1,1,1,1}, 10);
    // Draws "123" centered at (200,100) with 8x12 px characters, horizontal advance 10.
    void TextRenderer::drawString(Renderer2D &renderer, const std::string &text, const glm::vec2 &position, const glm::vec2 &charSize, const glm::vec4 &color, float spacing)
    {
        // Center the full string horizontally (midpoint alignment).
        float totalWidth = text.length() * spacing - (spacing - charSize.x);
        float startX = position.x - totalWidth * 0.5f;

        // Draw each digit with fixed spacing (tabular)
        for (size_t i = 0; i < text.length(); ++i)
        {
            char character = text[i];

            // Digit position with pixel snapping to avoid blur on HUD.
            float charX = floor(startX + i * spacing) + 0.5f;
            float charY = floor(position.y - charSize.y * 0.5f) + 0.5f;
            glm::vec2 charPos = glm::vec2(charX, charY);

            // Render 7 segments according to the predefined table.
            drawChar7Segment(renderer, character, charPos, charSize, color);
        }
    }

    ////////////////////////////////////////////////////////////////////////////
    //  Internal Helper: 7 segments
    ////////////////////////////////////////////////////////////////////////////

    void TextRenderer::drawChar7Segment(Renderer2D &renderer, char character, const glm::vec2 &pos, const glm::vec2 &size, const glm::vec4 &color)
    {
        const float w = size.x;
        const float h = size.y;
        const float thickness = 1.5f; // Segment thickness
        const float halfH = h * 0.5f;

        bool s[7] = {false}; // Active segments: a, b, c, d, e, f, g

        // Convert to uppercase to simplify
        char c = toupper(character);

        switch (c)
        {
        // Numbers
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

        // Letters (common 7-segment representations)
        case 'A':
            s[0] = s[1] = s[2] = s[4] = s[5] = s[6] = true;
            break; // A
        case 'B':
            s[2] = s[3] = s[4] = s[5] = s[6] = true;
            break; // b (lowercase)
        case 'C':
            s[0] = s[3] = s[4] = s[5] = true;
            break; // C
        case 'D':
            s[1] = s[2] = s[3] = s[4] = s[6] = true;
            break; // d (lowercase)
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
            break; // O (same as 0)
        case 'P':
            s[0] = s[1] = s[4] = s[5] = s[6] = true;
            break; // P
        case 'Q':
            s[0] = s[1] = s[2] = s[5] = s[6] = true;
            break; // q (lowercase)
        case 'R':
            s[4] = s[6] = true;
            break; // r (lowercase)
        case 'S':
            s[0] = s[2] = s[3] = s[5] = s[6] = true;
            break; // S (same as 5)
        case 'T':
            s[3] = s[4] = s[5] = s[6] = true;
            break; // t (lowercase)
        case 'U':
            s[1] = s[2] = s[3] = s[4] = s[5] = true;
            break; // U
        case 'Y':
            s[1] = s[2] = s[3] = s[5] = s[6] = true;
            break; // Y

        // Symbols
        case '-':
            s[6] = true;
            break; // Dash
        case '.':
            renderer.drawRect(pos + glm::vec2(w * 0.5f - thickness * 0.5f, h - thickness), glm::vec2(thickness, thickness), color, true);
            return; // Dot
        case ' ':
            break; // Space
        default:
            break; // Unsupported character
        }

        // Draw active segments
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
