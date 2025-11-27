/**
 * @file TerrainPlane.h
 * @brief Flat textured terrain plane that follows the camera.
 */

#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>
#include <string>
#include <vector>

extern "C"
{
#include <glad/glad.h>
}

#include "../core/Shader.h"

namespace gfx
{

    /**
     * @brief Constante para primitive restart
     */
    constexpr unsigned int PRIMITIVE_RESTART = 0xFFFFU;

    /**
     * @brief Configuración del plano de terreno
     */
    struct TerrainConfig
    {
        int segments = 32;            ///< Segmentos por lado del grid.
        float segmentSize = 2000.0f;  ///< Tamaño de cada segmento en metros.
        float textureTiling = 12.0f;  ///< Cuántas veces se repite la textura por tile.
        float fogMinDist = 1000.0f;   ///< Distancia mínima de niebla.
        float fogMaxDist = 100000.0f; ///< Distancia máxima de niebla.
    };

    /**
     * @brief Bloque de geometría plano
     */
    class TerrainBlock
    {
    public:
        TerrainBlock(int width, int height, float segmentSize);
        ~TerrainBlock();

        void draw();
        void bind();
        void unbind();

    private:
        GLuint vao_ = 0;
        GLuint vbo_ = 0;
        GLuint ebo_ = 0;
        unsigned int indexCount_ = 0;

        void generateGeometry(int width, int height, float segmentSize);
    };

    /**
     * @brief Terreno plano texturizado
     */
    class TerrainPlane
    {
    public:
        explicit TerrainPlane(TerrainConfig &config);
        ~TerrainPlane();

        void init();
        bool loadTexture(const std::string &basePath);

        void draw(const glm::mat4 &view,
                  const glm::mat4 &projection,
                  const glm::vec3 &cameraPos,
                  const glm::vec3 &backgroundColor = glm::vec3(0.5f, 0.7f, 1.0f));

        void cleanup();

        bool wireframe = false;

    private:
        TerrainConfig *config_ = nullptr; // Non-owning pointer to shared config.
        Shader shader_;
        GLuint terrainTex_ = 0;
        std::unique_ptr<TerrainBlock> tile_;
        bool initialized_ = false;

        GLuint loadTextureFromFile(const std::string &path);
    };

} // namespace gfx
