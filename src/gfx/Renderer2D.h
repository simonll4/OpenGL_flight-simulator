#pragma once
#include <vector>
#include <glm/glm.hpp>

extern "C"
{
#include <glad/glad.h>
}

#include "Shader.h"

namespace gfx
{

    struct Vertex2D
    {
        glm::vec2 position;
        glm::vec4 color;
        glm::vec2 texCoord;
    };

    class Renderer2D
    {
    public:
        Renderer2D();
        ~Renderer2D();

        // No permitir copia
        Renderer2D(const Renderer2D &) = delete;
        Renderer2D &operator=(const Renderer2D &) = delete;

        void init(int screenWidth, int screenHeight);
        void setScreenSize(int width, int height);

        // Comenzar/terminar batch de renderizado
        void begin();
        void end();
        void flush();

        // Primitivas básicas
        void drawLine(const glm::vec2 &start, const glm::vec2 &end, const glm::vec4 &color, float thickness = 1.0f);
        void drawRect(const glm::vec2 &position, const glm::vec2 &size, const glm::vec4 &color, bool filled = true);
        void drawCircle(const glm::vec2 &center, float radius, const glm::vec4 &color, int segments = 32, bool filled = true);
        void drawArc(const glm::vec2 &center, float radius, float startAngle, float endAngle, const glm::vec4 &color, int segments = 32);

        // Formas específicas para instrumentos
        void drawTick(const glm::vec2 &center, float angle, float innerRadius, float outerRadius, const glm::vec4 &color, float thickness = 1.0f);
        void drawScale(const glm::vec2 &center, float radius, float startAngle, float endAngle, int numTicks, const glm::vec4 &color);

    private:
        GLuint vao_, vbo_, ebo_;
        Shader shader_;

        std::vector<Vertex2D> vertices_;
        std::vector<GLuint> indices_;

        glm::mat4 projection_;
        int screenWidth_, screenHeight_;

        static const size_t MAX_VERTICES = 10000;
        static const size_t MAX_INDICES = 15000;

        void addVertex(const Vertex2D &vertex);
        void addQuad(const glm::vec2 &pos, const glm::vec2 &size, const glm::vec4 &color);
        void setupBuffers();
    };

} // namespace gfx
