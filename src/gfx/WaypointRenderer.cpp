#include "WaypointRenderer.h"
#include <cmath>
#include <vector>
#include <iostream>

namespace gfx
{
    ////////////////////////////////////////////////////////////////////////////
    //  Lifecycle
    ////////////////////////////////////////////////////////////////////////////

    WaypointRenderer::~WaypointRenderer()
    {
        if (vao_)
            glDeleteVertexArrays(1, &vao_);
        if (vbo_)
            glDeleteBuffers(1, &vbo_);
        if (ebo_)
            glDeleteBuffers(1, &ebo_);
    }

    void WaypointRenderer::init()
    {
        // 1) Compile specialized shader (colors cylinders with emissive pulses).
        shader_.load("shaders/waypoint.vert", "shaders/waypoint.frag");

        // 2) Build standard geometry shared by all waypoints.
        createCylinderGeometry();
    }

    ////////////////////////////////////////////////////////////////////////////
    //  Cylinder Geometry
    ////////////////////////////////////////////////////////////////////////////

    void WaypointRenderer::createCylinderGeometry()
    {
        const int segments = 16;
        const float radius = 3.0f;
        const float height = 30.0f;

        std::vector<float> vertices;
        std::vector<unsigned int> indices;

        // Generate cylinder vertices (bottom/top pairs with normal).
        for (int i = 0; i <= segments; ++i)
        {
            float angle = (float)i / (float)segments * 2.0f * M_PI;
            float x = radius * cosf(angle);
            float z = radius * sinf(angle);

            // Bottom vertex
            vertices.push_back(x);
            vertices.push_back(0.0f);
            vertices.push_back(z);
            // Normal
            vertices.push_back(x / radius);
            vertices.push_back(0.0f);
            vertices.push_back(z / radius);

            // Top vertex
            vertices.push_back(x);
            vertices.push_back(height);
            vertices.push_back(z);
            // Normal
            vertices.push_back(x / radius);
            vertices.push_back(0.0f);
            vertices.push_back(z / radius);
        }

        // Build indices for each side quad (2 triangles per segment).
        for (int i = 0; i < segments; ++i)
        {
            int base = i * 2;
            // Triangle 1
            indices.push_back(base);
            indices.push_back(base + 1);
            indices.push_back(base + 2);
            // Triangle 2
            indices.push_back(base + 1);
            indices.push_back(base + 3);
            indices.push_back(base + 2);
        }

        indexCount_ = indices.size();

        // Reserve GPU buffers and upload data.
        glGenVertexArrays(1, &vao_);
        glGenBuffers(1, &vbo_);
        glGenBuffers(1, &ebo_);

        glBindVertexArray(vao_);

        glBindBuffer(GL_ARRAY_BUFFER, vbo_);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

        // Attribute 0: position (vec3)
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(0);

        // Attribute 1: normal (vec3)
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glBindVertexArray(0);
    }

    ////////////////////////////////////////////////////////////////////////////
    //  Render
    ////////////////////////////////////////////////////////////////////////////

    void WaypointRenderer::drawWaypoint(const glm::mat4 &view, const glm::mat4 &proj,
                                        const glm::vec3 &position, const glm::vec4 &color,
                                        bool isActive)
    {
        shader_.use();

        // Translate the unit cylinder to the waypoint position.
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, position);

        shader_.setMat4("model", model);
        shader_.setMat4("view", view);
        shader_.setMat4("projection", proj);
        shader_.setVec3("waypointColor", glm::vec3(color));
        shader_.setFloat("waypointAlpha", color.a);
        shader_.setBool("isActive", isActive);

        glBindVertexArray(vao_);
        glDrawElements(GL_TRIANGLES, indexCount_, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

} // namespace gfx
