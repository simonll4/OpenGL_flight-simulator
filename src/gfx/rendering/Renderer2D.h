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

    /// Representa un vértice 2D con posición, color y coordenada UV opcional.
    struct Vertex2D
    {
        glm::vec2 position;
        glm::vec4 color;
        glm::vec2 texCoord;
    };

    /**
     * @brief Batch renderer simple para HUDs e instrumentos 2D.
     *
     * Acumula vértices en CPU y los descarga en un VBO dinámico mediante flush.
     * Soporta líneas engrosadas, rectángulos, círculos y primitivas específicas
     * para instrumentos (ticks, escalas, polilíneas, triángulos indicadores).
     */
    class Renderer2D
    {
    public:
        Renderer2D();
        ~Renderer2D();

        // No permitir copia
        Renderer2D(const Renderer2D &) = delete;
        Renderer2D &operator=(const Renderer2D &) = delete;

        /**
         * @brief Configura buffers y proyección ortográfica.
         * @param screenWidth Ancho actual de la superficie de dibujo.
         * @param screenHeight Alto actual.
         */
        void init(int screenWidth, int screenHeight);
        /// Actualiza la proyección cuando el HUD cambia de resolución.
        void setScreenSize(int width, int height);

        // Comenzar/terminar batch de renderizado
        void begin();
        void end();
        void flush();

        /// Cambia el atlas/textura activa para las primitivas batcheadas.
        void setTexture(GLuint textureId);

        // Primitivas básicas
        void drawLine(const glm::vec2 &start, const glm::vec2 &end, const glm::vec4 &color, float thickness = 1.0f);
        void drawRect(const glm::vec2 &position, const glm::vec2 &size, const glm::vec4 &color, bool filled = true);
        void drawCircle(const glm::vec2 &center, float radius, const glm::vec4 &color, int segments = 32, bool filled = true);
        void drawArc(const glm::vec2 &center, float radius, float startAngle, float endAngle, const glm::vec4 &color, int segments = 32);

        // Formas específicas para instrumentos
        void drawTick(const glm::vec2 &center, float angle, float innerRadius, float outerRadius, const glm::vec4 &color, float thickness = 1.0f);
        void drawScale(const glm::vec2 &center, float radius, float startAngle, float endAngle, int numTicks, const glm::vec4 &color);

        // Primitivas adicionales para instrumentos HUD de actitud
        void drawPolyline(const std::vector<glm::vec2> &points, const glm::vec4 &color, float thickness = 1.0f, bool closed = false);
        void drawTriangle(const glm::vec2 &p1, const glm::vec2 &p2, const glm::vec2 &p3, const glm::vec4 &color, bool filled = false);
        void drawTexturedQuad(const glm::vec2 &topLeft, const glm::vec2 &bottomRight, const glm::vec4 &color,
                              const glm::vec2 &uvMin, const glm::vec2 &uvMax);

    private:
        GLuint vao_, vbo_, ebo_; ///< Objetos de buffer compartidos por todo el batch.
        Shader shader_;          ///< Programa que dibuja primitivas coloreadas.

        std::vector<Vertex2D> vertices_; ///< Buffer CPU de vértices batcheados.
        std::vector<GLuint> indices_;    ///< Indices CPU alineados al mismo batch.

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
