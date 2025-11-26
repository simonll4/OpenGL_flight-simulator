/**
 * @file TrueTypeFont.h
 * @brief TrueType font loader and renderer using stb_truetype.
 */

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
     * @brief Loads a TrueType font using stb_truetype and renders it via Renderer2D.
     */
    class TrueTypeFont
    {
    public:
        TrueTypeFont();
        ~TrueTypeFont();

        /**
         * @brief Loads the font from disk and generates a texture atlas.
         * @param path Path to the .ttf file.
         * @param bakePixelHeight Height in pixels used to generate the atlas.
         * @param atlasSize Square dimension of the atlas (e.g., 1024).
         */
        bool loadFromFile(const std::string &path, float bakePixelHeight = 48.0f, int atlasSize = 1024);

        bool isLoaded() const { return textureId_ != 0; }

        /**
         * @brief Returns the approximate size of the text block.
         * @param text String to measure (supports \n).
         * @param pixelHeight Text height when rendering.
         * @param lineSpacing Multiplier for line spacing.
         */
        glm::vec2 measureText(const std::string &text, float pixelHeight, float lineSpacing = 1.0f) const;

        /**
         * @brief Renders text starting from the top-left corner.
         * @param renderer Active Renderer2D.
         * @param text String to draw.
         * @param topLeft Start position (top-left corner of the block).
         * @param pixelHeight Desired height in pixels.
         * @param color Color multiplier.
         * @param lineSpacing Line spacing factor.
         * @param snapToPixel Adjusts coordinates to pixel centers to avoid blur.
         * @param manageTexture If true, the function automatically binds/unbinds the atlas.
         */
        void drawText(Renderer2D &renderer, const std::string &text, const glm::vec2 &topLeft,
                      float pixelHeight, const glm::vec4 &color,
                      float lineSpacing = 1.0f, bool snapToPixel = true, bool manageTexture = true) const;

        /**
         * @brief Allows manual control of atlas bind/unbind for batching multiple texts.
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
