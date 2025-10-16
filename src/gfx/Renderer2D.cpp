#include "gfx/Renderer2D.h"
#include "gfx/GLCheck.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>

namespace gfx
{

    Renderer2D::Renderer2D() : vao_(0), vbo_(0), ebo_(0), screenWidth_(800), screenHeight_(600)
    {
        vertices_.reserve(MAX_VERTICES);
        indices_.reserve(MAX_INDICES);
    }

    Renderer2D::~Renderer2D()
    {
        if (ebo_)
            glDeleteBuffers(1, &ebo_);
        if (vbo_)
            glDeleteBuffers(1, &vbo_);
        if (vao_)
            glDeleteVertexArrays(1, &vao_);
    }

    void Renderer2D::init(int screenWidth, int screenHeight)
    {
        screenWidth_ = screenWidth;
        screenHeight_ = screenHeight;

        // Crear proyección ortográfica
        projection_ = glm::ortho(0.0f, (float)screenWidth_, (float)screenHeight_, 0.0f, -1.0f, 1.0f);

        setupBuffers();

        // Cargar shader 2D
        shader_.load("shaders/hud.vert", "shaders/hud.frag");
    }

    void Renderer2D::setScreenSize(int width, int height)
    {
        screenWidth_ = width;
        screenHeight_ = height;
        projection_ = glm::ortho(0.0f, (float)width, (float)height, 0.0f, -1.0f, 1.0f);
    }

    void Renderer2D::setupBuffers()
    {
        glGenVertexArrays(1, &vao_);
        glGenBuffers(1, &vbo_);
        glGenBuffers(1, &ebo_);

        glBindVertexArray(vao_);

        glBindBuffer(GL_ARRAY_BUFFER, vbo_);
        glBufferData(GL_ARRAY_BUFFER, MAX_VERTICES * sizeof(Vertex2D), nullptr, GL_DYNAMIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, MAX_INDICES * sizeof(GLuint), nullptr, GL_DYNAMIC_DRAW);

        // Position
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex2D), (void *)offsetof(Vertex2D, position));
        glEnableVertexAttribArray(0);

        // Color
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex2D), (void *)offsetof(Vertex2D, color));
        glEnableVertexAttribArray(1);

        // TexCoord
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex2D), (void *)offsetof(Vertex2D, texCoord));
        glEnableVertexAttribArray(2);

        glBindVertexArray(0);

        checkGLError("Setting up 2D renderer buffers");
    }

    void Renderer2D::begin()
    {
        vertices_.clear();
        indices_.clear();
    }

    void Renderer2D::end()
    {
        flush();
    }

    void Renderer2D::flush()
    {
        if (vertices_.empty())
            return;

        // Subir datos a GPU
        glBindBuffer(GL_ARRAY_BUFFER, vbo_);
        glBufferSubData(GL_ARRAY_BUFFER, 0, vertices_.size() * sizeof(Vertex2D), vertices_.data());

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, indices_.size() * sizeof(GLuint), indices_.data());

        // Renderizar
        shader_.use();
        shader_.setMat4("uProjection", projection_);

        glBindVertexArray(vao_);
        glDrawElements(GL_TRIANGLES, indices_.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        checkGLError("Flushing 2D renderer");
    }

    void Renderer2D::addVertex(const Vertex2D &vertex)
    {
        if (vertices_.size() >= MAX_VERTICES)
        {
            flush();
            vertices_.clear();
            indices_.clear();
        }
        vertices_.push_back(vertex);
    }

    void Renderer2D::addQuad(const glm::vec2 &pos, const glm::vec2 &size, const glm::vec4 &color)
    {
        GLuint baseIndex = vertices_.size();

        // Cuatro vértices del quad
        addVertex({{pos.x, pos.y}, color, {0.0f, 0.0f}});
        addVertex({{pos.x + size.x, pos.y}, color, {1.0f, 0.0f}});
        addVertex({{pos.x + size.x, pos.y + size.y}, color, {1.0f, 1.0f}});
        addVertex({{pos.x, pos.y + size.y}, color, {0.0f, 1.0f}});

        // Dos triángulos
        indices_.push_back(baseIndex);
        indices_.push_back(baseIndex + 1);
        indices_.push_back(baseIndex + 2);

        indices_.push_back(baseIndex);
        indices_.push_back(baseIndex + 2);
        indices_.push_back(baseIndex + 3);
    }

    void Renderer2D::drawLine(const glm::vec2 &start, const glm::vec2 &end, const glm::vec4 &color, float thickness)
    {
        glm::vec2 direction = glm::normalize(end - start);
        glm::vec2 perpendicular = glm::vec2(-direction.y, direction.x) * (thickness * 0.5f);

        GLuint baseIndex = vertices_.size();

        // Cuatro vértices para la línea gruesa
        addVertex({{start.x - perpendicular.x, start.y - perpendicular.y}, color, {0.0f, 0.0f}});
        addVertex({{start.x + perpendicular.x, start.y + perpendicular.y}, color, {1.0f, 0.0f}});
        addVertex({{end.x + perpendicular.x, end.y + perpendicular.y}, color, {1.0f, 1.0f}});
        addVertex({{end.x - perpendicular.x, end.y - perpendicular.y}, color, {0.0f, 1.0f}});

        // Dos triángulos
        indices_.push_back(baseIndex);
        indices_.push_back(baseIndex + 1);
        indices_.push_back(baseIndex + 2);

        indices_.push_back(baseIndex);
        indices_.push_back(baseIndex + 2);
        indices_.push_back(baseIndex + 3);
    }

    void Renderer2D::drawRect(const glm::vec2 &position, const glm::vec2 &size, const glm::vec4 &color, bool filled)
    {
        if (filled)
        {
            addQuad(position, size, color);
        }
        else
        {
            // Dibujar borde
            float thickness = 1.0f;
            drawLine(position, {position.x + size.x, position.y}, color, thickness);
            drawLine({position.x + size.x, position.y}, position + size, color, thickness);
            drawLine(position + size, {position.x, position.y + size.y}, color, thickness);
            drawLine({position.x, position.y + size.y}, position, color, thickness);
        }
    }

    void Renderer2D::drawCircle(const glm::vec2 &center, float radius, const glm::vec4 &color, int segments, bool filled)
    {
        if (filled)
        {
            GLuint centerIndex = vertices_.size();
            addVertex({center, color, {0.5f, 0.5f}});

            for (int i = 0; i <= segments; ++i)
            {
                float angle = 2.0f * M_PI * i / segments;
                glm::vec2 pos = center + glm::vec2(cos(angle), sin(angle)) * radius;
                addVertex({pos, color, {0.5f + 0.5f * cos(angle), 0.5f + 0.5f * sin(angle)}});

                if (i > 0)
                {
                    indices_.push_back(centerIndex);
                    indices_.push_back(centerIndex + i);
                    indices_.push_back(centerIndex + i + 1);
                }
            }
        }
        else
        {
            // Dibujar borde
            for (int i = 0; i < segments; ++i)
            {
                float angle1 = 2.0f * M_PI * i / segments;
                float angle2 = 2.0f * M_PI * (i + 1) / segments;

                glm::vec2 pos1 = center + glm::vec2(cos(angle1), sin(angle1)) * radius;
                glm::vec2 pos2 = center + glm::vec2(cos(angle2), sin(angle2)) * radius;

                drawLine(pos1, pos2, color, 1.0f);
            }
        }
    }

    void Renderer2D::drawArc(const glm::vec2 &center, float radius, float startAngle, float endAngle, const glm::vec4 &color, int segments)
    {
        float angleRange = endAngle - startAngle;
        int arcSegments = (int)(segments * angleRange / (2.0f * M_PI));

        for (int i = 0; i < arcSegments; ++i)
        {
            float angle1 = startAngle + angleRange * i / arcSegments;
            float angle2 = startAngle + angleRange * (i + 1) / arcSegments;

            glm::vec2 pos1 = center + glm::vec2(cos(angle1), sin(angle1)) * radius;
            glm::vec2 pos2 = center + glm::vec2(cos(angle2), sin(angle2)) * radius;

            drawLine(pos1, pos2, color, 1.0f);
        }
    }

    void Renderer2D::drawTick(const glm::vec2 &center, float angle, float innerRadius, float outerRadius, const glm::vec4 &color, float thickness)
    {
        glm::vec2 inner = center + glm::vec2(cos(angle), sin(angle)) * innerRadius;
        glm::vec2 outer = center + glm::vec2(cos(angle), sin(angle)) * outerRadius;
        drawLine(inner, outer, color, thickness);
    }

    void Renderer2D::drawScale(const glm::vec2 &center, float radius, float startAngle, float endAngle, int numTicks, const glm::vec4 &color)
    {
        float angleRange = endAngle - startAngle;
        for (int i = 0; i <= numTicks; ++i)
        {
            float angle = startAngle + angleRange * i / numTicks;
            float tickLength = (i % 5 == 0) ? 10.0f : 5.0f; // Ticks más largos cada 5
            drawTick(center, angle, radius - tickLength, radius, color, 1.0f);
        }
    }

} // namespace gfx
