/**
 * @file Renderer2D.h
 * @brief 2D batch renderer for HUD and UI elements.
 */

#pragma once
#include <vector>
#include <glm/glm.hpp>

extern "C"
{
#include <glad/glad.h>
}

#include "../core/Shader.h"

namespace gfx
{

    /// Represents a 2D vertex with position, color, and optional UV coordinate.
    struct Vertex2D
    {
        glm::vec2 position;
        glm::vec4 color;
        glm::vec2 texCoord;
    };

    /**
     * @brief Simple batch renderer for HUDs and 2D instruments.
     *
     * Accumulates vertices on the CPU and flushes them to a dynamic VBO.
     * Supports thickened lines, rectangles, circles, and specific primitives
     * for instruments (ticks, scales, polylines, indicator triangles).
     */
    class Renderer2D
    {
    public:
        Renderer2D();
        ~Renderer2D();

        // Disallow copy
        Renderer2D(const Renderer2D &) = delete;
        Renderer2D &operator=(const Renderer2D &) = delete;

        /**
         * @brief Configures buffers and orthographic projection.
         * @param screenWidth Current width of the drawing surface.
         * @param screenHeight Current height.
         */
        void init(int screenWidth, int screenHeight);
        /// Updates the projection when the HUD resolution changes.
        void setScreenSize(int width, int height);

        // Begin/end rendering batch
        void begin();
        void end();
        void flush();

        /// Changes the active atlas/texture for batched primitives.
        void setTexture(GLuint textureId);

        // Basic primitives
        /// Thickened 2D segment rendered como quad.
        void drawLine(const glm::vec2 &start, const glm::vec2 &end, const glm::vec4 &color, float thickness = 1.0f);
        /// Eje-alineado; `filled=false` dibuja solo el borde.
        void drawRect(const glm::vec2 &position, const glm::vec2 &size, const glm::vec4 &color, bool filled = true);
        /// Círculo aproximado por triángulos; relleno u outline.
        void drawCircle(const glm::vec2 &center, float radius, const glm::vec4 &color, int segments = 32, bool filled = true);

        // Instrument-specific shapes
        /// Marca radial para escalas circulares (HSI, bank angle).
        void drawTick(const glm::vec2 &center, float angle, float innerRadius, float outerRadius, const glm::vec4 &color, float thickness = 1.0f);
        /// Serie de ticks distribuidos entre dos ángulos.
        void drawScale(const glm::vec2 &center, float radius, float startAngle, float endAngle, int numTicks, const glm::vec4 &color);

        // Additional primitives for attitude HUD instruments
        /// Polilínea abierta/cerrada con grosor.
        void drawPolyline(const std::vector<glm::vec2> &points, const glm::vec4 &color, float thickness = 1.0f, bool closed = false);
        /// Triángulo relleno o solo contorno.
        void drawTriangle(const glm::vec2 &p1, const glm::vec2 &p2, const glm::vec2 &p3, const glm::vec4 &color, bool filled = false);
        /// Quad texturizado con UV explícitos.
        void drawTexturedQuad(const glm::vec2 &topLeft, const glm::vec2 &bottomRight, const glm::vec4 &color,
                              const glm::vec2 &uvMin, const glm::vec2 &uvMax);

    private:
        GLuint vao_, vbo_, ebo_; ///< Buffer objects shared by the entire batch.
        Shader shader_;          ///< Program that draws colored primitives.

        std::vector<Vertex2D> vertices_; ///< CPU buffer of batched vertices.
        std::vector<GLuint> indices_;    ///< CPU indices aligned to the same batch.

        glm::mat4 projection_;
        int screenWidth_, screenHeight_;

        static const size_t MAX_VERTICES = 10000;
        static const size_t MAX_INDICES = 15000;

        GLuint currentTexture_ = 0;

        void addVertex(const Vertex2D &vertex);
        void addQuad(const glm::vec2 &pos, const glm::vec2 &size, const glm::vec4 &color);
        void setupBuffers();
        void ensureCapacity(size_t vertexCount, size_t indexCount);
    };

} // namespace gfx
