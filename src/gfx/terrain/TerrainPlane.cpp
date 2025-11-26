#include "TerrainPlane.h"

#include <fstream>
#include <iostream>

extern "C"
{
#include <stb/stb_image.h>
}

namespace gfx
{

    // ============================================================================
    // TerrainBlock Implementation
    // ============================================================================

    TerrainBlock::TerrainBlock(int width, int height, float segmentSize)
    {
        generateGeometry(width, height, segmentSize);
    }

    TerrainBlock::~TerrainBlock()
    {
        if (vao_)
            glDeleteVertexArrays(1, &vao_);
        if (vbo_)
            glDeleteBuffers(1, &vbo_);
        if (ebo_)
            glDeleteBuffers(1, &ebo_);
    }

    void TerrainBlock::generateGeometry(int width, int height, float segmentSize)
    {
        std::vector<glm::vec3> vertices;
        std::vector<unsigned int> indices;

        for (int y = 0; y <= height; y++)
        {
            for (int x = 0; x <= width; x++)
            {
                vertices.push_back({x * segmentSize,
                                    0.0f,
                                    y * segmentSize});
            }
        }

        for (int r = 0; r < height; r++)
        {
            for (int c = 0; c <= width; c++)
            {
                unsigned int i0 = r * (width + 1) + c;
                indices.push_back(i0);

                unsigned int i1 = (r + 1) * (width + 1) + c;
                indices.push_back(i1);
            }
            indices.push_back(PRIMITIVE_RESTART);
        }

        indexCount_ = indices.size();

        glGenVertexArrays(1, &vao_);
        glGenBuffers(1, &vbo_);
        glGenBuffers(1, &ebo_);

        glBindVertexArray(vao_);

        glBindBuffer(GL_ARRAY_BUFFER, vbo_);
        glBufferData(GL_ARRAY_BUFFER,
                     vertices.size() * sizeof(glm::vec3),
                     vertices.data(),
                     GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                     indices.size() * sizeof(unsigned int),
                     indices.data(),
                     GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void *)0);
        glEnableVertexAttribArray(0);

        glBindVertexArray(0);
    }

    void TerrainBlock::bind()
    {
        glBindVertexArray(vao_);
    }

    void TerrainBlock::unbind()
    {
        glBindVertexArray(0);
    }

    void TerrainBlock::draw()
    {
        bind();
        glDrawElements(GL_TRIANGLE_STRIP, indexCount_, GL_UNSIGNED_INT, 0);
        unbind();
    }

    // ============================================================================
    // TerrainPlane Implementation
    // ============================================================================

    TerrainPlane::TerrainPlane(TerrainConfig &config)
        : config_(&config),
          shader_("shaders/terrain_plane.vert", "shaders/terrain_plane.frag")
    {
    }

    TerrainPlane::~TerrainPlane()
    {
        cleanup();
    }

    void TerrainPlane::init()
    {
        if (initialized_)
            return;

        if (!config_)
        {
            std::cerr << "TerrainPlane::init() missing config\n";
            return;
        }

        tile_ = std::make_unique<TerrainBlock>(config_->segments, config_->segments, config_->segmentSize);
        initialized_ = true;

        std::cout << "TerrainPlane initialized (segments=" << config_->segments
                  << ", segmentSize=" << config_->segmentSize
                  << ", tiling=" << config_->textureTiling << ")\n";
    }

    bool TerrainPlane::loadTexture(const std::string &basePath)
    {
        std::string texturePath = basePath + "/texture.png";

        if (!std::ifstream(texturePath).good())
        {
            texturePath = basePath + "/forrest_ground_01_diff_4k.jpg";
        }

        terrainTex_ = loadTextureFromFile(texturePath);

        if (!terrainTex_)
        {
            std::cout << "No terrain texture found, creating default green texture\n";
            unsigned char greenData[64 * 64 * 3];
            for (int i = 0; i < 64 * 64; i++)
            {
                greenData[i * 3 + 0] = 60;
                greenData[i * 3 + 1] = 120;
                greenData[i * 3 + 2] = 40;
            }

            glGenTextures(1, &terrainTex_);
            glBindTexture(GL_TEXTURE_2D, terrainTex_);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 64, 64, 0, GL_RGB, GL_UNSIGNED_BYTE, greenData);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        }

        std::cout << "Terrain texture ready\n";
        return terrainTex_ != 0;
    }

    void TerrainPlane::draw(const glm::mat4 &view,
                            const glm::mat4 &projection,
                            const glm::vec3 &cameraPos,
                            const glm::vec3 &backgroundColor)
    {
        if (!initialized_)
        {
            std::cerr << "TerrainPlane::draw() called before init()\n";
            return;
        }

        if (!config_)
        {
            std::cerr << "TerrainPlane::draw() missing config\n";
            return;
        }

        if (!terrainTex_)
        {
            std::cerr << "TerrainPlane::draw() called without texture\n";
            return;
        }

        float tileSize = config_->segments * config_->segmentSize;
        glm::vec2 cameraPosXY(cameraPos.x, cameraPos.z);
        glm::vec2 snapped = glm::floor(cameraPosXY / tileSize) * tileSize;

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, terrainTex_);

        shader_.use();
        shader_.setInt("u_Texture", 0);
        shader_.setMat4("u_View", view);
        shader_.setMat4("u_Projection", projection);
        shader_.setVec3("u_Background", backgroundColor);
        shader_.setVec3("u_CameraPos", cameraPos);
        shader_.setFloat("u_FogMinDist", config_->fogMinDist);
        shader_.setFloat("u_FogMaxDist", config_->fogMaxDist);
        shader_.setFloat("u_TileSize", tileSize);
        shader_.setFloat("u_TextureTiling", config_->textureTiling);

        glEnable(GL_CULL_FACE);
        glEnable(GL_PRIMITIVE_RESTART);
        glPrimitiveRestartIndex(PRIMITIVE_RESTART);

        if (wireframe)
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }

        for (int dz = -1; dz <= 1; ++dz)
        {
            for (int dx = -1; dx <= 1; ++dx)
            {
                glm::vec2 tileOffset = snapped + glm::vec2(dx * tileSize, dz * tileSize);
                glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(tileOffset.x, 0.0f, tileOffset.y));
                shader_.setMat4("u_Model", model);
                tile_->draw();
            }
        }

        if (wireframe)
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }

        glDisable(GL_CULL_FACE);
        glDisable(GL_PRIMITIVE_RESTART);
    }

    void TerrainPlane::cleanup()
    {
        if (terrainTex_)
        {
            glDeleteTextures(1, &terrainTex_);
        }

        terrainTex_ = 0;
        initialized_ = false;
    }

    GLuint TerrainPlane::loadTextureFromFile(const std::string &path)
    {
        int width, height, channels;
        unsigned char *data = stbi_load(path.c_str(), &width, &height, &channels, 0);

        if (!data)
        {
            std::cerr << "Failed to load texture: " << path << "\n";
            return 0;
        }

        GLenum format = GL_RGB;
        if (channels == 1)
        {
            format = GL_RED;
        }
        else if (channels == 4)
        {
            format = GL_RGBA;
        }

        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);

        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);

        std::cout << "Loaded terrain texture: " << path << " (" << width << "x" << height << ")\n";

        return texture;
    }

} // namespace gfx
