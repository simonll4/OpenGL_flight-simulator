#pragma once
#include <glm/glm.hpp>
#include <string>
#include "Shader.h"
#include "TerrainMesh.h"

extern "C"
{
#include <glad/glad.h>
}

namespace gfx
{

    struct TerrainParams
    {
        float groundY = 0.0f;
        float tileScaleMacro = 0.25f; // texels por metro (macro)
        float tileScaleDetail = 2.0f; // texels por metro (detail)
        float detailStrength = 0.35f;
        float fogDensity = 0.015f;
        glm::vec3 colorTint = glm::vec3(1.0f, 1.0f, 1.0f);
    };

    class TerrainRenderer
    {
    public:
        TerrainRenderer();
        ~TerrainRenderer();

        void init();
        void loadTextures(const std::string &basePath);
        void draw(const glm::mat4 &view, const glm::mat4 &projection,
                  const glm::vec3 &cameraPos, const TerrainParams &params);
        void cleanup();

    private:
        Shader shader_;
        TerrainMesh mesh_;

        GLuint albedoTex_ = 0;
        GLuint normalTex_ = 0;
        GLuint roughTex_ = 0;
        GLuint detailAlbedoTex_ = 0;
        GLuint detailNormalTex_ = 0;

        GLuint loadTexture(const std::string &path, bool sRGB = false);
    };

} // namespace gfx
