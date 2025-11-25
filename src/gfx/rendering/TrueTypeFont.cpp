#define STB_TRUETYPE_IMPLEMENTATION
#include "stb/stb_truetype.h"

#include "TrueTypeFont.h"

#include <fstream>
#include <iostream>
#include <algorithm>
#include <iterator>
#include <cmath>

namespace gfx
{

    namespace
    {
        constexpr int FIRST_CHAR = 32;
        constexpr int NUM_CHARS = 224; // U+0020..U+00FF para cubrir acentos comunes
    }

    TrueTypeFont::TrueTypeFont()
    {
        firstChar_ = FIRST_CHAR;
        glyphCount_ = NUM_CHARS;
    }

    TrueTypeFont::~TrueTypeFont()
    {
        if (textureId_ != 0)
        {
            glDeleteTextures(1, &textureId_);
            textureId_ = 0;
        }
    }

    bool TrueTypeFont::loadFromFile(const std::string &path, float bakePixelHeight, int atlasSize)
    {
        std::ifstream file(path, std::ios::binary);
        if (!file)
        {
            std::cerr << "[TrueTypeFont] No se pudo abrir la fuente: " << path << std::endl;
            return false;
        }

        fontBuffer_ = std::vector<unsigned char>(std::istreambuf_iterator<char>(file), {});
        if (fontBuffer_.empty())
        {
            std::cerr << "[TrueTypeFont] Fuente vacía o inválida: " << path << std::endl;
            return false;
        }

        fontInfo_ = std::make_unique<stbtt_fontinfo>();
        if (!stbtt_InitFont(fontInfo_.get(), fontBuffer_.data(), stbtt_GetFontOffsetForIndex(fontBuffer_.data(), 0)))
        {
            std::cerr << "[TrueTypeFont] stbtt_InitFont falló para: " << path << std::endl;
            return false;
        }

        atlasPixelHeight_ = bakePixelHeight;
        atlasWidth_ = atlasSize;
        atlasHeight_ = atlasSize;
        std::vector<unsigned char> atlas(atlasWidth_ * atlasHeight_, 0);
        std::vector<stbtt_packedchar> packedChars(glyphCount_);
        stbtt_pack_context ctx;
        if (!stbtt_PackBegin(&ctx, atlas.data(), atlasWidth_, atlasHeight_, 0, 1, nullptr))
        {
            std::cerr << "[TrueTypeFont] stbtt_PackBegin falló" << std::endl;
            return false;
        }
        // Aumentar oversampling para mejor calidad de texto
        stbtt_PackSetOversampling(&ctx, 3, 3);
        if (!stbtt_PackFontRange(&ctx, fontBuffer_.data(), 0, atlasPixelHeight_, firstChar_, glyphCount_, packedChars.data()))
        {
            stbtt_PackEnd(&ctx);
            std::cerr << "[TrueTypeFont] stbtt_PackFontRange falló" << std::endl;
            return false;
        }
        stbtt_PackEnd(&ctx);

        glyphs_.resize(glyphCount_);
        for (int i = 0; i < glyphCount_; ++i)
        {
            const stbtt_packedchar &src = packedChars[i];
            GlyphInfo info;
            info.x0 = static_cast<float>(src.x0);
            info.y0 = static_cast<float>(src.y0);
            info.x1 = static_cast<float>(src.x1);
            info.y1 = static_cast<float>(src.y1);
            info.xoff = src.xoff;
            info.yoff = src.yoff;
            info.xadvance = src.xadvance;
            info.xoff2 = src.xoff2;
            info.yoff2 = src.yoff2;
            glyphs_[i] = info;
        }

        if (textureId_ == 0)
        {
            glGenTextures(1, &textureId_);
        }
        glBindTexture(GL_TEXTURE_2D, textureId_);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, atlasWidth_, atlasHeight_, 0, GL_RED, GL_UNSIGNED_BYTE, atlas.data());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glBindTexture(GL_TEXTURE_2D, 0);

        stbtt_GetFontVMetrics(fontInfo_.get(), &ascent_, &descent_, &lineGap_);

        return true;
    }

    void TrueTypeFont::begin(Renderer2D &renderer) const
    {
        if (!isLoaded())
            return;
        renderer.setTexture(textureId_);
    }

    void TrueTypeFont::end(Renderer2D &renderer) const
    {
        renderer.setTexture(0);
    }

    const TrueTypeFont::GlyphInfo *TrueTypeFont::lookup(std::uint32_t codepoint) const
    {
        if (codepoint < static_cast<std::uint32_t>(firstChar_) ||
            codepoint >= static_cast<std::uint32_t>(firstChar_ + glyphCount_))
        {
            return nullptr;
        }
        return &glyphs_[codepoint - firstChar_];
    }

    float TrueTypeFont::glyphScale(float pixelHeight) const
    {
        if (atlasPixelHeight_ <= 0.0f)
            return 1.0f;
        return pixelHeight / atlasPixelHeight_;
    }

    float TrueTypeFont::ascentPixels(float pixelHeight) const
    {
        if (!fontInfo_)
            return pixelHeight;
        float scale = stbtt_ScaleForPixelHeight(fontInfo_.get(), pixelHeight);
        return ascent_ * scale;
    }

    float TrueTypeFont::lineAdvance(float pixelHeight, float lineSpacing) const
    {
        if (!fontInfo_)
            return pixelHeight * lineSpacing;
        float scale = stbtt_ScaleForPixelHeight(fontInfo_.get(), pixelHeight);
        float baseLineHeight = (ascent_ - descent_) * scale;
        float gap = lineGap_ * scale;
        return (baseLineHeight + gap) * lineSpacing;
    }

    glm::vec2 TrueTypeFont::measureText(const std::string &text, float pixelHeight, float lineSpacing) const
    {
        if (!isLoaded() || text.empty())
        {
            return glm::vec2(0.0f);
        }

        float scale = glyphScale(pixelHeight);
        float metricsAdvance = lineAdvance(pixelHeight, lineSpacing);
        float baseScale = fontInfo_ ? stbtt_ScaleForPixelHeight(fontInfo_.get(), pixelHeight) : glyphScale(pixelHeight);
        float baseLineHeight = (ascent_ - descent_) * baseScale;

        float width = 0.0f;
        float maxWidth = 0.0f;
        int lines = 1;

        for (size_t i = 0; i < text.size();)
        {
            std::uint32_t codepoint = 0;
            if (!decodeUtf8(text, i, codepoint))
            {
                break;
            }
            if (codepoint == '\n')
            {
                maxWidth = std::max(maxWidth, width);
                width = 0.0f;
                ++lines;
                continue;
            }
            if (codepoint == '\r')
            {
                continue;
            }
            const GlyphInfo *glyph = lookup(codepoint);
            if (!glyph)
                glyph = lookup(fallbackCodepoint());
            if (!glyph)
                continue;
            width += glyph->xadvance * scale;
        }

        maxWidth = std::max(maxWidth, width);
        float totalHeight = baseLineHeight;
        if (lines > 1)
        {
            totalHeight += (lines - 1) * metricsAdvance;
        }
        return glm::vec2(maxWidth, totalHeight);
    }

    void TrueTypeFont::drawText(Renderer2D &renderer, const std::string &text, const glm::vec2 &topLeft,
                                float pixelHeight, const glm::vec4 &color, float lineSpacing, bool snapToPixel, bool manageTexture) const
    {
        if (!isLoaded() || text.empty())
        {
            return;
        }

        if (manageTexture)
        {
            begin(renderer);
        }

        float scaleGlyph = glyphScale(pixelHeight);
        float ascentPx = ascentPixels(pixelHeight);
        float lineStep = lineAdvance(pixelHeight, lineSpacing);

        float penX = topLeft.x;
        float baselineY = topLeft.y + ascentPx;

        for (size_t i = 0; i < text.size();)
        {
            std::uint32_t codepoint = 0;
            if (!decodeUtf8(text, i, codepoint))
            {
                break;
            }

            if (codepoint == '\n')
            {
                penX = topLeft.x;
                baselineY += lineStep;
                continue;
            }
            if (codepoint == '\r')
            {
                continue;
            }

            const GlyphInfo *glyph = lookup(codepoint);
            if (!glyph)
            {
                glyph = lookup(fallbackCodepoint());
            }
            if (!glyph)
            {
                continue;
            }

            float x0 = penX + glyph->xoff * scaleGlyph;
            float y0 = baselineY + glyph->yoff * scaleGlyph;
            float x1 = penX + glyph->xoff2 * scaleGlyph;
            float y1 = baselineY + glyph->yoff2 * scaleGlyph;

            if (snapToPixel)
            {
                x0 = std::floor(x0) + 0.5f;
                y0 = std::floor(y0) + 0.5f;
                x1 = std::floor(x1) + 0.5f;
                y1 = std::floor(y1) + 0.5f;
            }

            glm::vec2 uv0(glyph->x0 / static_cast<float>(atlasWidth_), glyph->y0 / static_cast<float>(atlasHeight_));
            glm::vec2 uv1(glyph->x1 / static_cast<float>(atlasWidth_), glyph->y1 / static_cast<float>(atlasHeight_));

            renderer.drawTexturedQuad(glm::vec2(x0, y0), glm::vec2(x1, y1), color, uv0, uv1);

            penX += glyph->xadvance * scaleGlyph;
        }

        if (manageTexture)
        {
            end(renderer);
        }
    }

    std::uint32_t TrueTypeFont::fallbackCodepoint() const
    {
        return static_cast<std::uint32_t>('?');
    }

    bool TrueTypeFont::decodeUtf8(const std::string &text, size_t &index, std::uint32_t &codepoint) const
    {
        if (index >= text.size())
        {
            return false;
        }

        unsigned char byte = static_cast<unsigned char>(text[index]);
        if (byte < 0x80)
        {
            codepoint = byte;
            ++index;
            return true;
        }
        else if ((byte & 0xE0) == 0xC0 && index + 1 < text.size())
        {
            codepoint = ((byte & 0x1F) << 6) |
                        (static_cast<unsigned char>(text[index + 1]) & 0x3F);
            index += 2;
            return true;
        }
        else if ((byte & 0xF0) == 0xE0 && index + 2 < text.size())
        {
            codepoint = ((byte & 0x0F) << 12) |
                        ((static_cast<unsigned char>(text[index + 1]) & 0x3F) << 6) |
                        (static_cast<unsigned char>(text[index + 2]) & 0x3F);
            index += 3;
            return true;
        }
        else if ((byte & 0xF8) == 0xF0 && index + 3 < text.size())
        {
            codepoint = ((byte & 0x07) << 18) |
                        ((static_cast<unsigned char>(text[index + 1]) & 0x3F) << 12) |
                        ((static_cast<unsigned char>(text[index + 2]) & 0x3F) << 6) |
                        (static_cast<unsigned char>(text[index + 3]) & 0x3F);
            index += 4;
            return true;
        }

        // Secuencia inválida: consumir un byte y devolver símbolo de respaldo
        ++index;
        codepoint = fallbackCodepoint();
        return true;
    }

} // namespace gfx
