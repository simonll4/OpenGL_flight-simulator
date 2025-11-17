#include "ClipmapTerrain.h"
#include <iostream>
#include <cmath>
#include <cassert>
#include <algorithm>
#include <fstream>

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

        // Generar grid de vértices (Y=0, altura se aplica en vertex shader)
        for (int y = 0; y <= height; y++)
        {
            for (int x = 0; x <= width; x++)
            {
                vertices.push_back({x * segmentSize,
                                    0.0f,
                                    y * segmentSize});
            }
        }

        // Generar índices como triangle strips con primitive restart
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
        assert(indices.size() > 0 && vertices.size() > 0);

        // Crear OpenGL objects
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
    // TerrainSeam Implementation
    // ============================================================================

    TerrainSeam::TerrainSeam(int columns, float segmentSize)
    {
        generateGeometry(columns, segmentSize);
    }

    TerrainSeam::~TerrainSeam()
    {
        if (vao_)
            glDeleteVertexArrays(1, &vao_);
        if (vbo_)
            glDeleteBuffers(1, &vbo_);
    }

    void TerrainSeam::generateGeometry(int columns, float segmentSize)
    {
        std::vector<glm::vec3> vertices;

        // Cada columna genera 3 vértices (2 triángulos)
        // Conecta resolución fina con gruesa
        for (int x = 0; x < columns; x++)
        {
            float xPos = x * segmentSize;
            float xMid = xPos + segmentSize / 2.0f;
            float xNext = xPos + segmentSize;

            vertices.push_back({xPos, 0.0f, 0.0f});
            vertices.push_back({xMid, 0.0f, segmentSize});
            vertices.push_back({xNext, 0.0f, 0.0f});
        }

        vertexCount_ = vertices.size();

        glGenVertexArrays(1, &vao_);
        glGenBuffers(1, &vbo_);

        glBindVertexArray(vao_);

        glBindBuffer(GL_ARRAY_BUFFER, vbo_);
        glBufferData(GL_ARRAY_BUFFER,
                     vertices.size() * sizeof(glm::vec3),
                     vertices.data(),
                     GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void *)0);
        glEnableVertexAttribArray(0);

        glBindVertexArray(0);
    }

    void TerrainSeam::bind()
    {
        glBindVertexArray(vao_);
    }

    void TerrainSeam::unbind()
    {
        glBindVertexArray(0);
    }

    void TerrainSeam::draw()
    {
        bind();
        glDrawArrays(GL_TRIANGLES, 0, vertexCount_);
        unbind();
    }

    // ============================================================================
    // ClipmapTerrain Implementation
    // ============================================================================

    ClipmapTerrain::ClipmapTerrain(const ClipmapConfig &config)
        : config_(config),
          shader_("shaders/clipmap.vert", "shaders/clipmap.frag")
    {
    }

    ClipmapTerrain::~ClipmapTerrain()
    {
        cleanup();
    }

    void ClipmapTerrain::init()
    {
        if (initialized_)
            return;

        initShaders();
        initGeometry();

        initialized_ = true;
        std::cout << "ClipmapTerrain initialized successfully\n";
        std::cout << "  Levels: " << config_.levels << "\n";
        std::cout << "  Segments: " << config_.segments << "\n";
        std::cout << "  Segment size: " << config_.segmentSize << "m\n";
    }

    void ClipmapTerrain::initGeometry()
    {
        int s = config_.segments;
        float size = config_.segmentSize;

        // Crear diferentes tipos de bloques
        tile_ = std::make_unique<TerrainBlock>(s, s, size);
        center_ = std::make_unique<TerrainBlock>(2 * s + 2, 2 * s + 2, size);
        colFixup_ = std::make_unique<TerrainBlock>(2, s, size);
        rowFixup_ = std::make_unique<TerrainBlock>(s, 2, size);
        horizontal_ = std::make_unique<TerrainBlock>(2 * s + 2, 1, size);
        vertical_ = std::make_unique<TerrainBlock>(1, 2 * s + 2, size);
        seam_ = std::make_unique<TerrainSeam>(2 * s + 2, size * 2.0f);

        std::cout << "Clipmap geometry created\n";
    }

    void ClipmapTerrain::initShaders()
    {
        // Los shaders se cargan automáticamente por la clase Shader
        // que busca archivos .vert y .frag
        std::cout << "Clipmap shaders loaded\n";
    }

    bool ClipmapTerrain::loadTextures(const std::string &basePath)
    {
        std::string heightmapPath = basePath + "/heightmap.png";
        std::string normalmapPath = basePath + "/normalmap.png";
        std::string texturePath = basePath + "/texture.png";

        const float requestedHeightScale = config_.heightScale;

        // Intentar cargar heightmap y recalibrar altura para que el altímetro 0 coincida con el suelo
        int hmWidth = 0;
        int hmHeight = 0;
        int hmChannels = 0;
        stbi_uc *heightData = stbi_load(heightmapPath.c_str(), &hmWidth, &hmHeight, &hmChannels, 1);
        if (heightData)
        {
            float minValue = 1.0f;
            float maxValue = 0.0f;
            const int pixelCount = hmWidth * hmHeight;
            for (int i = 0; i < pixelCount; ++i)
            {
                float value = heightData[i] / 255.0f;
                minValue = std::min(minValue, value);
                maxValue = std::max(maxValue, value);
            }

            float range = std::max(maxValue - minValue, 1e-5f);
            config_.heightScale = requestedHeightScale / range;
            config_.heightOffset = -config_.heightScale * minValue;

            glGenTextures(1, &heightmapTex_);
            glBindTexture(GL_TEXTURE_2D, heightmapTex_);
            glTexImage2D(GL_TEXTURE_2D,
                         0,
                         GL_RED,
                         hmWidth,
                         hmHeight,
                         0,
                         GL_RED,
                         GL_UNSIGNED_BYTE,
                         heightData);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            stbi_image_free(heightData);

            std::cout << "Clipmap heightmap loaded: " << hmWidth << "x" << hmHeight
                      << " (min=" << minValue << ", max=" << maxValue
                      << ") -> offset=" << config_.heightOffset
                      << "m, scale=" << requestedHeightScale << "m" << std::endl;
        }
        else
        {
            std::cout << "No heightmap found, creating flat terrain\n";
            config_.heightScale = requestedHeightScale;
            config_.heightOffset = 0.0f;

            unsigned char flatData[64 * 64];
            std::fill_n(flatData, 64 * 64, 0);

            glGenTextures(1, &heightmapTex_);
            glBindTexture(GL_TEXTURE_2D, heightmapTex_);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 64, 64, 0, GL_RED, GL_UNSIGNED_BYTE, flatData);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        }

        // Intentar cargar normalmap
        normalmapTex_ = loadTexture(normalmapPath, GL_RGB);

        // Si no hay normalmap, crear uno por defecto (normal apuntando hacia arriba)
        if (!normalmapTex_)
        {
            std::cout << "No normalmap found, creating default normals\n";
            unsigned char normalData[64 * 64 * 3];
            for (int i = 0; i < 64 * 64; i++)
            {
                normalData[i * 3 + 0] = 128; // X = 0.0 en rango [-1,1]
                normalData[i * 3 + 1] = 128; // Y = 0.0
                normalData[i * 3 + 2] = 255; // Z = 1.0 (hacia arriba)
            }

            glGenTextures(1, &normalmapTex_);
            glBindTexture(GL_TEXTURE_2D, normalmapTex_);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 64, 64, 0, GL_RGB, GL_UNSIGNED_BYTE, normalData);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        }

        // Intentar cargar textura de color, con fallback a textura existente
        if (!std::ifstream(texturePath).good())
        {
            texturePath = basePath + "/forrest_ground_01_diff_4k.jpg";
        }
        terrainTex_ = loadTexture(texturePath, GL_RGB);

        // Si tampoco hay textura de color, crear una verde por defecto
        if (!terrainTex_)
        {
            std::cout << "No terrain texture found, creating default green texture\n";
            unsigned char greenData[64 * 64 * 3];
            for (int i = 0; i < 64 * 64; i++)
            {
                greenData[i * 3 + 0] = 60;  // R
                greenData[i * 3 + 1] = 120; // G (más verde)
                greenData[i * 3 + 2] = 40;  // B
            }

            glGenTextures(1, &terrainTex_);
            glBindTexture(GL_TEXTURE_2D, terrainTex_);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 64, 64, 0, GL_RGB, GL_UNSIGNED_BYTE, greenData);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        }

        bool success = (heightmapTex_ != 0 && normalmapTex_ != 0 && terrainTex_ != 0);

        if (success)
        {
            std::cout << "Clipmap textures ready\n";
        }
        else
        {
            std::cerr << "Error: Failed to create clipmap textures\n";
        }

        return success;
    }

    void ClipmapTerrain::draw(const glm::mat4 &view,
                              const glm::mat4 &projection,
                              const glm::vec3 &cameraPos,
                              const glm::vec3 &backgroundColor)
    {
        if (!initialized_)
        {
            std::cerr << "ClipmapTerrain::draw() called before init()\n";
            return;
        }

        glm::vec2 cameraPosXY(cameraPos.x, cameraPos.z);
        float cameraHeight = cameraPos.y;

        // Activar texturas
        if (heightmapTex_)
        {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, heightmapTex_);
        }
        if (normalmapTex_)
        {
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, normalmapTex_);
        }
        if (terrainTex_)
        {
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, terrainTex_);
        }

        // Activar shader
        shader_.use();

        // Uniforms globales
        shader_.setInt("u_Heightmap", 0);
        shader_.setInt("u_Normalmap", 1);
        shader_.setInt("u_Texture", 2);
        shader_.setVec3("u_Background", backgroundColor);
        shader_.setMat4("u_View", view);
        shader_.setMat4("u_Projection", projection);
        shader_.setVec3("u_CameraPos", cameraPos);
        shader_.setFloat("u_TerrainSize", config_.terrainSize);
        shader_.setFloat("u_HeightScale", config_.heightScale);
        shader_.setFloat("u_HeightOffset", config_.heightOffset);

        // Usar valores de fog del config (actualizados dinámicamente)
        shader_.setFloat("u_FogMinDist", config_.fogMinDist);
        shader_.setFloat("u_FogMaxDist", config_.fogMaxDist);
        shader_.setBool("u_ShowLODColors", showLODColors);

        // Configurar OpenGL
        glEnable(GL_CULL_FACE);
        glEnable(GL_PRIMITIVE_RESTART);
        glPrimitiveRestartIndex(PRIMITIVE_RESTART);

        if (wireframe)
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }

        // Renderizar niveles - ajustar minLevel basado en altura de cámara
        // Mientras más alto, empezar desde niveles más gruesos
        int minLevel = std::max(1, (int)(cameraHeight / 1000.0f)); // 1 nivel por cada 1000m
        minLevel = std::min(minLevel, config_.levels - 6);         // Siempre renderizar al menos 6 niveles

        for (int l = minLevel; l <= config_.levels; l++)
        {
            drawLevel(l, cameraPosXY, minLevel);
        }

        // Restaurar estado
        if (wireframe)
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
        glDisable(GL_CULL_FACE);
        glDisable(GL_PRIMITIVE_RESTART);
    }

    void ClipmapTerrain::drawLevel(int l, const glm::vec2 &cameraPosXY, int &minLevel)
    {
        int s = config_.segments;
        float size = config_.segmentSize;

        float scale = std::pow(2.0f, l);
        float scaled_segment_size = size * scale;
        float tile_size = s * scaled_segment_size;

        // Uniforms por nivel
        shader_.setFloat("u_Scale", scale);
        shader_.setFloat("u_SegmentSize", scaled_segment_size);
        shader_.setFloat("u_Level", static_cast<float>(l) / config_.levels);

        // Renderizar un tile grande centrado en la cámara
        // Versión simplificada sin clipmap complejo
        glm::vec2 snapped = glm::floor(cameraPosXY / scaled_segment_size) * scaled_segment_size;
        glm::vec2 tilePos = snapped - glm::vec2(tile_size / 2.0f);

        shader_.setMat4("u_Model", transformMatrix(tilePos, scale));
        tile_->draw();
    }

    glm::vec2 ClipmapTerrain::calcBase(int level, const glm::vec2 &cameraPos) const
    {
        float scale = std::pow(2.0f, level);
        float next_scale = std::pow(2.0f, level + 2);
        float tile_size = config_.segments * config_.segmentSize * scale;
        glm::vec2 snapped = glm::floor(cameraPos / next_scale) * next_scale;
        glm::vec2 base = snapped - tile_size * 2.0f;
        return base;
    }

    glm::mat4 ClipmapTerrain::transformMatrix(const glm::vec2 &position,
                                              float scale,
                                              float angle) const
    {
        glm::mat4 S = glm::scale(glm::mat4(1.0f), glm::vec3(scale));
        glm::mat4 T = glm::translate(glm::mat4(1.0f), glm::vec3(position.x, 0.0f, position.y));
        glm::mat4 R = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
        return T * R * S;
    }

    float ClipmapTerrain::getHeightAt(const glm::vec2 &worldPos) const
    {
        // TODO: Implementar lectura de heightmap en CPU
        return 0.0f;
    }

    void ClipmapTerrain::cleanup()
    {
        if (heightmapTex_)
            glDeleteTextures(1, &heightmapTex_);
        if (normalmapTex_)
            glDeleteTextures(1, &normalmapTex_);
        if (terrainTex_)
            glDeleteTextures(1, &terrainTex_);

        heightmapTex_ = normalmapTex_ = terrainTex_ = 0;
        initialized_ = false;
    }

    GLuint ClipmapTerrain::loadTexture(const std::string &path, GLenum format)
    {
        int width, height, channels;
        unsigned char *data = stbi_load(path.c_str(), &width, &height, &channels, 0);

        if (!data)
        {
            std::cerr << "Failed to load texture: " << path << "\n";
            return 0;
        }

        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);

        // Determinar formato
        GLenum internal_format = format;
        if (format == GL_RED && channels == 1)
        {
            internal_format = GL_RED;
        }
        else if (channels == 3)
        {
            format = GL_RGB;
            internal_format = GL_RGB;
        }
        else if (channels == 4)
        {
            format = GL_RGBA;
            internal_format = GL_RGBA;
        }

        glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);

        std::cout << "Loaded clipmap texture: " << path << " (" << width << "x" << height << ")\n";

        return texture;
    }

} // namespace gfx
