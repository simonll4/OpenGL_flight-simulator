#include "Renderer2D.h"
#include "../core/GLCheck.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>
#include <stdexcept>

namespace gfx
{

    ////////////////////////////////////////////////////////////////////////////
    //  Lifecycle
    ////////////////////////////////////////////////////////////////////////////

    Renderer2D::Renderer2D() : vao_(0), vbo_(0), ebo_(0), screenWidth_(800), screenHeight_(600), currentTexture_(0)
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

        // Create pixel-perfect orthographic projection for HUD (origin top-left).
        projection_ = glm::ortho(0.0f, (float)screenWidth_, (float)screenHeight_, 0.0f, -1.0f, 1.0f);

        setupBuffers();

        // Compile shader that applies solid color (no textures for now).
        shader_.load("shaders/hud.vert", "shaders/hud.frag");
    }

    void Renderer2D::setScreenSize(int width, int height)
    {
        screenWidth_ = width;
        screenHeight_ = height;
        projection_ = glm::ortho(0.0f, (float)width, (float)height, 0.0f, -1.0f, 1.0f);
    }

    ////////////////////////////////////////////////////////////////////////////
    //  Buffering
    ////////////////////////////////////////////////////////////////////////////

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

    ////////////////////////////////////////////////////////////////////////////
    //  Batch Cycle
    ////////////////////////////////////////////////////////////////////////////

    void Renderer2D::begin()
    {
        vertices_.clear();
        indices_.clear();
        currentTexture_ = 0;
    }

    void Renderer2D::end()
    {
        flush();
    }

    void Renderer2D::flush()
    {
        if (vertices_.empty())
            return;

        // Upload data to GPU
        glBindBuffer(GL_ARRAY_BUFFER, vbo_);
        glBufferSubData(GL_ARRAY_BUFFER, 0, vertices_.size() * sizeof(Vertex2D), vertices_.data());

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, indices_.size() * sizeof(GLuint), indices_.data());

        // Render
        shader_.use();
        shader_.setMat4("uProjection", projection_);
        shader_.setBool("uUseTexture", currentTexture_ != 0);
        shader_.setInt("uTexture", 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, currentTexture_);

        glBindVertexArray(vao_);
        glDrawElements(GL_TRIANGLES, indices_.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        checkGLError("Flushing 2D renderer");

        // Clear after rendering to avoid mixing consecutive batches.
        vertices_.clear();
        indices_.clear();
    }

    void Renderer2D::setTexture(GLuint textureId)
    {
        if (currentTexture_ == textureId)
        {
            return;
        }
        // Finish the current batch before changing texture
        flush();
        currentTexture_ = textureId;
    }

    void Renderer2D::ensureCapacity(size_t vertexCount, size_t indexCount)
    {
        if (vertexCount > MAX_VERTICES || indexCount > MAX_INDICES)
        {
            throw std::runtime_error("Renderer2D primitive exceeds buffer capacity");
        }
        if (vertices_.size() + vertexCount > MAX_VERTICES || indices_.size() + indexCount > MAX_INDICES)
        {
            flush();
        }
    }

    ////////////////////////////////////////////////////////////////////////////
    //  Primitive Construction Helpers
    ////////////////////////////////////////////////////////////////////////////

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
        ensureCapacity(4, 6);
        GLuint baseIndex = vertices_.size();

        // Four vertices of the quad
        addVertex({{pos.x, pos.y}, color, {0.0f, 0.0f}});
        addVertex({{pos.x + size.x, pos.y}, color, {1.0f, 0.0f}});
        addVertex({{pos.x + size.x, pos.y + size.y}, color, {1.0f, 1.0f}});
        addVertex({{pos.x, pos.y + size.y}, color, {0.0f, 1.0f}});

        // Two triangles
        indices_.push_back(baseIndex);
        indices_.push_back(baseIndex + 1);
        indices_.push_back(baseIndex + 2);

        indices_.push_back(baseIndex);
        indices_.push_back(baseIndex + 2);
        indices_.push_back(baseIndex + 3);
    }

    void Renderer2D::drawLine(const glm::vec2 &start, const glm::vec2 &end, const glm::vec4 &color, float thickness)
    {
        ensureCapacity(4, 6);
        glm::vec2 direction = glm::normalize(end - start);
        glm::vec2 perpendicular = glm::vec2(-direction.y, direction.x) * (thickness * 0.5f);

        GLuint baseIndex = vertices_.size();

        // Four vertices for the thick line
        addVertex({{start.x - perpendicular.x, start.y - perpendicular.y}, color, {0.0f, 0.0f}});
        addVertex({{start.x + perpendicular.x, start.y + perpendicular.y}, color, {1.0f, 0.0f}});
        addVertex({{end.x + perpendicular.x, end.y + perpendicular.y}, color, {1.0f, 1.0f}});
        addVertex({{end.x - perpendicular.x, end.y - perpendicular.y}, color, {0.0f, 1.0f}});

        // Two triangles
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
            // Draw border
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
            ensureCapacity(static_cast<size_t>(segments) + 2, static_cast<size_t>(segments) * 3);
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
            // Draw border
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

    // Instrument-specific shapes
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
            float tickLength = (i % 5 == 0) ? 10.0f : 5.0f; // Longer ticks every 5
            drawTick(center, angle, radius - tickLength, radius, color, 1.0f);
        }
    }

    void Renderer2D::drawPolyline(const std::vector<glm::vec2> &points, const glm::vec4 &color, float thickness, bool closed)
    {
        if (points.size() < 2)
            return;

        // Draw connected lines
        for (size_t i = 0; i < points.size() - 1; ++i)
        {
            drawLine(points[i], points[i + 1], color, thickness);
        }

        // If closed, connect last to first
        if (closed && points.size() > 2)
        {
            drawLine(points[points.size() - 1], points[0], color, thickness);
        }
    }

    void Renderer2D::drawTriangle(const glm::vec2 &p1, const glm::vec2 &p2, const glm::vec2 &p3, const glm::vec4 &color, bool filled)
    {
        if (filled)
        {
            ensureCapacity(3, 3);
            GLuint baseIndex = vertices_.size();

            addVertex({p1, color, {0.0f, 0.0f}});
            addVertex({p2, color, {1.0f, 0.0f}});
            addVertex({p3, color, {0.5f, 1.0f}});

            indices_.push_back(baseIndex);
            indices_.push_back(baseIndex + 1);
            indices_.push_back(baseIndex + 2);
        }
        else
        {
            // Draw border as closed polyline
            drawPolyline({p1, p2, p3}, color, 1.0f, true);
        }
    }

    void Renderer2D::drawTexturedQuad(const glm::vec2 &topLeft, const glm::vec2 &bottomRight, const glm::vec4 &color,
                                      const glm::vec2 &uvMin, const glm::vec2 &uvMax)
    {
        ensureCapacity(4, 6);
        GLuint baseIndex = vertices_.size();

        addVertex({{topLeft.x, topLeft.y}, color, {uvMin.x, uvMin.y}});
        addVertex({{bottomRight.x, topLeft.y}, color, {uvMax.x, uvMin.y}});
        addVertex({{bottomRight.x, bottomRight.y}, color, {uvMax.x, uvMax.y}});
        addVertex({{topLeft.x, bottomRight.y}, color, {uvMin.x, uvMax.y}});

        indices_.push_back(baseIndex);
        indices_.push_back(baseIndex + 1);
        indices_.push_back(baseIndex + 2);

        indices_.push_back(baseIndex);
        indices_.push_back(baseIndex + 2);
        indices_.push_back(baseIndex + 3);
    }

} // namespace gfx
