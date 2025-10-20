#include "WaypointRenderer.h"
#include <cmath>
#include <vector>
#include <iostream>

namespace gfx
{
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
        // Cargar shaders desde archivos
        shader_.load("shaders/waypoint.vert", "shaders/waypoint.frag");
        createCylinderGeometry();
    }

    void WaypointRenderer::createCylinderGeometry()
    {
        const int segments = 16;
        const float radius = 3.0f;
        const float height = 30.0f;

        std::vector<float> vertices;
        std::vector<unsigned int> indices;

        // Generar vértices del cilindro
        for (int i = 0; i <= segments; ++i)
        {
            float angle = (float)i / (float)segments * 2.0f * M_PI;
            float x = radius * cosf(angle);
            float z = radius * sinf(angle);

            // Vértice inferior
            vertices.push_back(x);
            vertices.push_back(0.0f);
            vertices.push_back(z);
            // Normal
            vertices.push_back(x / radius);
            vertices.push_back(0.0f);
            vertices.push_back(z / radius);

            // Vértice superior
            vertices.push_back(x);
            vertices.push_back(height);
            vertices.push_back(z);
            // Normal
            vertices.push_back(x / radius);
            vertices.push_back(0.0f);
            vertices.push_back(z / radius);
        }

        // Generar índices
        for (int i = 0; i < segments; ++i)
        {
            int base = i * 2;
            // Triángulo 1
            indices.push_back(base);
            indices.push_back(base + 1);
            indices.push_back(base + 2);
            // Triángulo 2
            indices.push_back(base + 1);
            indices.push_back(base + 3);
            indices.push_back(base + 2);
        }

        indexCount_ = indices.size();

        // Crear buffers
        glGenVertexArrays(1, &vao_);
        glGenBuffers(1, &vbo_);
        glGenBuffers(1, &ebo_);

        glBindVertexArray(vao_);

        glBindBuffer(GL_ARRAY_BUFFER, vbo_);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

        // Posición
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(0);

        // Normal
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glBindVertexArray(0);
    }

    void WaypointRenderer::drawWaypoint(const glm::mat4 &view, const glm::mat4 &proj,
                                        const glm::vec3 &position, const glm::vec4 &color,
                                        bool isActive)
    {
        shader_.use();

        // Crear matriz de modelo
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, position);

        shader_.setMat4("model", model);
        shader_.setMat4("view", view);
        shader_.setMat4("projection", proj);
        shader_.setVec3("waypointColor", glm::vec3(color));
        shader_.setFloat("waypointAlpha", color.a);
        shader_.setBool("isActive", isActive);

        // Obtener viewPos desde la matriz view (inversa)
        glm::mat4 viewInv = glm::inverse(view);
        glm::vec3 viewPos = glm::vec3(viewInv[3]);
        shader_.setVec3("viewPos", viewPos);

        glBindVertexArray(vao_);
        glDrawElements(GL_TRIANGLES, indexCount_, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

} // namespace gfx
