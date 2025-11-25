#pragma once

#include <string>
#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include <cstdint>

#include "Renderer2D.h"

struct stbtt_fontinfo;

namespace gfx
{

    /**
     * @brief Carga una fuente TrueType mediante stb_truetype y la renderiza sobre Renderer2D.
     */
    class TrueTypeFont
    {
    public:
        TrueTypeFont();
        ~TrueTypeFont();

        /**
         * @brief Carga la fuente desde disco y genera un atlas de textura.
         * @param path Ruta al archivo .ttf.
         * @param bakePixelHeight Altura en píxeles utilizada para generar el atlas.
         * @param atlasSize Dimensión cuadrada del atlas (p.ej. 1024).
         */
        bool loadFromFile(const std::string &path, float bakePixelHeight = 48.0f, int atlasSize = 1024);

        bool isLoaded() const { return textureId_ != 0; }

        /**
         * @brief Devuelve el tamaño aproximado del bloque de texto.
         * @param text Cadena a medir (soporta \n).
         * @param pixelHeight Altura del texto al renderizar.
         * @param lineSpacing Multiplicador para el interlineado.
         */
        glm::vec2 measureText(const std::string &text, float pixelHeight, float lineSpacing = 1.0f) const;

        /**
         * @brief Renderiza texto comenzando desde la esquina superior izquierda.
         * @param renderer Renderer2D activo.
         * @param text Cadena a dibujar.
         * @param topLeft Posición de inicio (esquina superior izquierda del bloque).
         * @param pixelHeight Altura en píxeles deseada.
         * @param color Color multiplicador.
         * @param lineSpacing Factor de separación entre líneas.
         * @param snapToPixel Ajusta las coordenadas a pixel centers para evitar blur.
         * @param manageTexture Si es true, la función hace bind/unbind del atlas automáticamente.
         */
        void drawText(Renderer2D &renderer, const std::string &text, const glm::vec2 &topLeft,
                      float pixelHeight, const glm::vec4 &color,
                      float lineSpacing = 1.0f, bool snapToPixel = true, bool manageTexture = true) const;

        /**
         * @brief Permite controlar manualmente el bind/unbind del atlas para batchear múltiples textos.
         */
        void begin(Renderer2D &renderer) const;
        void end(Renderer2D &renderer) const;

    private:
        struct GlyphInfo
        {
            float x0 = 0.0f;
            float y0 = 0.0f;
            float x1 = 0.0f;
            float y1 = 0.0f;
            float xoff = 0.0f;
            float yoff = 0.0f;
            float xadvance = 0.0f;
            float xoff2 = 0.0f;
            float yoff2 = 0.0f;
        };

        const GlyphInfo *lookup(std::uint32_t codepoint) const;
        float glyphScale(float pixelHeight) const;
        float ascentPixels(float pixelHeight) const;
        float lineAdvance(float pixelHeight, float lineSpacing) const;
        std::uint32_t fallbackCodepoint() const;
        bool decodeUtf8(const std::string &text, size_t &index, std::uint32_t &codepoint) const;

        std::vector<unsigned char> fontBuffer_;
        std::unique_ptr<stbtt_fontinfo> fontInfo_;
        GLuint textureId_ = 0;
        int atlasWidth_ = 0;
        int atlasHeight_ = 0;
        float atlasPixelHeight_ = 48.0f;
        int firstChar_ = 32;
        int glyphCount_ = 224; // Hasta U+00FF
        std::vector<GlyphInfo> glyphs_;
        int ascent_ = 0;
        int descent_ = 0;
        int lineGap_ = 0;
    };

} // namespace gfx
