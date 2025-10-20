#pragma once
#include <vector>
#include <glm/glm.hpp>

extern "C"
{
#include <glad/glad.h>
}

namespace gfx
{

    class TerrainMesh
    {
    public:
        TerrainMesh();
        ~TerrainMesh();

        void init(int gridSize = 128); // genera grid NxN
        void draw();
        void cleanup();

    private:
        GLuint vao_ = 0;
        GLuint vbo_ = 0;
        GLuint ebo_ = 0;
        int vertexCount_ = 0;
        int indexCount_ = 0;

        void generateGrid(int N);
    };

} // namespace gfx
