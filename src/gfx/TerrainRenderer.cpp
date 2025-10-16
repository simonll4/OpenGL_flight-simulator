#include "TerrainRenderer.h"
#include <stb/stb_image.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <cmath>

namespace gfx
{

    TerrainRenderer::TerrainRenderer() {}

    TerrainRenderer::~TerrainRenderer()
    {
        cleanup();
    }

    void TerrainRenderer::init()
    {
        // Compilar shaders
        shader_.load("shaders/terrain.vert", "shaders/terrain.frag");

        // Generar mesh (grid 128x128)
        mesh_.init(128);

        std::cout << "TerrainRenderer initialized" << std::endl;
    }

    GLuint TerrainRenderer::loadTexture(const std::string &path, bool sRGB)
    {
        GLuint texID;
        glGenTextures(1, &texID);

        int width, height, channels;
        unsigned char *data = stbi_load(path.c_str(), &width, &height, &channels, 0);

        if (!data)
        {
            std::cerr << "Failed to load texture: " << path << std::endl;
            std::cerr << "STB Error: " << stbi_failure_reason() << std::endl;
            return 0;
        }

        GLenum internalFormat = GL_RGB;
        GLenum format = GL_RGB;

        if (channels == 4)
        {
            format = GL_RGBA;
            internalFormat = sRGB ? GL_SRGB_ALPHA : GL_RGBA;
        }
        else if (channels == 3)
        {
            format = GL_RGB;
            internalFormat = sRGB ? GL_SRGB : GL_RGB;
        }
        else if (channels == 1)
        {
            format = GL_RED;
            internalFormat = GL_RED;
        }

        glBindTexture(GL_TEXTURE_2D, texID);
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        // Wrap mode: repeat
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        // Filter: trilinear
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // Anisotropic filtering (opcional)
        // float maxAnisotropy;
        // glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &maxAnisotropy);
        // glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, std::min(16.0f, maxAnisotropy));

        stbi_image_free(data);

        std::cout << "Loaded texture: " << path << " (" << width << "x" << height << ", "
                  << channels << " channels)" << std::endl;

        return texID;
    }

    void TerrainRenderer::loadTextures(const std::string &basePath)
    {
        // Cargar texturas principales
        albedoTex_ = loadTexture(basePath + "/forrest_ground_01_diff_4k.jpg", true);
        roughTex_ = loadTexture(basePath + "/forrest_ground_01_rough_4k.jpg", false);

        // Normal map: intentar cargar (puede estar en EXR pero vamos a ignorar por ahora)
        // Para EXR necesitaríamos otra librería, usaremos una textura plana como fallback
        normalTex_ = loadTexture(basePath + "/forrest_ground_01_rough_4k.jpg", false); // Temporal

        // Detail textures: usar la misma textura a menor escala
        detailAlbedoTex_ = albedoTex_;
        detailNormalTex_ = normalTex_;

        std::cout << "Terrain textures loaded from: " << basePath << std::endl;
    }

    void TerrainRenderer::draw(const glm::mat4 &view, const glm::mat4 &projection,
                               const glm::vec3 &cameraPos, const TerrainParams &params)
    {
        shader_.use();

        // Floating origin: snap camera position to grid
        const float snapStep = 32.0f;
        glm::vec2 snap = glm::floor(glm::vec2(cameraPos.x, cameraPos.z) / snapStep) * snapStep;
        glm::vec3 gridOffset = glm::vec3(snap.x, params.groundY, snap.y);

        // ViewProj matrix
        glm::mat4 viewProj = projection * view;

        // Set uniforms usando los helpers de Shader
        shader_.setMat4("uViewProj", viewProj);
        shader_.setVec3("uGridOffset", gridOffset);
        shader_.setVec3("uCamPos", cameraPos);
        shader_.setVec3("uColorTint", params.colorTint);
        shader_.setFloat("uTileMacro", params.tileScaleMacro);
        shader_.setFloat("uTileDetail", params.tileScaleDetail);
        shader_.setFloat("uDetailStr", params.detailStrength);
        shader_.setFloat("uFogDensity", params.fogDensity);

        // Bind textures
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, albedoTex_);
        shader_.setInt("uAlbedo", 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, normalTex_);
        shader_.setInt("uNormal", 1);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, roughTex_);
        shader_.setInt("uRough", 2);

        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, detailAlbedoTex_);
        shader_.setInt("uDetailAlbedo", 3);

        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, detailNormalTex_);
        shader_.setInt("uDetailNormal", 4);

        // Draw mesh
        mesh_.draw();

        // Unbind
        glActiveTexture(GL_TEXTURE0);
    }

    void TerrainRenderer::cleanup()
    {
        if (albedoTex_)
            glDeleteTextures(1, &albedoTex_);
        if (normalTex_)
            glDeleteTextures(1, &normalTex_);
        if (roughTex_)
            glDeleteTextures(1, &roughTex_);
        // detailAlbedo y detailNormal son aliases, no borrar dos veces

        albedoTex_ = normalTex_ = roughTex_ = 0;
        detailAlbedoTex_ = detailNormalTex_ = 0;

        mesh_.cleanup();
    }

} // namespace gfx
