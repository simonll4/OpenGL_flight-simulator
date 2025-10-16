#include "TerrainMesh.h"
#include <iostream>

namespace gfx
{

    TerrainMesh::TerrainMesh() {}

    TerrainMesh::~TerrainMesh()
    {
        cleanup();
    }

    void TerrainMesh::init(int gridSize)
    {
        generateGrid(gridSize);
    }

    void TerrainMesh::generateGrid(int N)
    {
        std::vector<float> vertices;
        std::vector<unsigned int> indices;

        // Generar grid centrado en el origen
        float halfSize = N * 0.5f;

        // Vertices: posición (x,0,z) + normal (0,1,0)
        for (int z = 0; z <= N; ++z)
        {
            for (int x = 0; x <= N; ++x)
            {
                float fx = x - halfSize;
                float fz = z - halfSize;

                // Posición
                vertices.push_back(fx);
                vertices.push_back(0.0f);
                vertices.push_back(fz);

                // Normal
                vertices.push_back(0.0f);
                vertices.push_back(1.0f);
                vertices.push_back(0.0f);
            }
        }

        // Índices (triángulos)
        for (int z = 0; z < N; ++z)
        {
            for (int x = 0; x < N; ++x)
            {
                unsigned int topLeft = z * (N + 1) + x;
                unsigned int topRight = topLeft + 1;
                unsigned int bottomLeft = (z + 1) * (N + 1) + x;
                unsigned int bottomRight = bottomLeft + 1;

                // Primer triángulo
                indices.push_back(topLeft);
                indices.push_back(bottomLeft);
                indices.push_back(topRight);

                // Segundo triángulo
                indices.push_back(topRight);
                indices.push_back(bottomLeft);
                indices.push_back(bottomRight);
            }
        }

        vertexCount_ = (N + 1) * (N + 1);
        indexCount_ = indices.size();

        // Crear VAO/VBO/EBO
        glGenVertexArrays(1, &vao_);
        glGenBuffers(1, &vbo_);
        glGenBuffers(1, &ebo_);

        glBindVertexArray(vao_);

        // VBO
        glBindBuffer(GL_ARRAY_BUFFER, vbo_);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float),
                     vertices.data(), GL_STATIC_DRAW);

        // EBO
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int),
                     indices.data(), GL_STATIC_DRAW);

        // Atributos: posición (location=0)
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);

        // Atributos: normal (location=1)
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                              (void *)(3 * sizeof(float)));

        glBindVertexArray(0);

        std::cout << "TerrainMesh created: " << vertexCount_ << " vertices, "
                  << indexCount_ << " indices" << std::endl;
    }

    void TerrainMesh::draw()
    {
        glBindVertexArray(vao_);
        glDrawElements(GL_TRIANGLES, indexCount_, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

    void TerrainMesh::cleanup()
    {
        if (vao_)
            glDeleteVertexArrays(1, &vao_);
        if (vbo_)
            glDeleteBuffers(1, &vbo_);
        if (ebo_)
            glDeleteBuffers(1, &ebo_);
        vao_ = vbo_ = ebo_ = 0;
    }

} // namespace gfx
