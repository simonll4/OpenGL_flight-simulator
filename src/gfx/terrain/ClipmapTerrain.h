#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <memory>
#include <string>

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
     * @brief Configuración del sistema Clipmap
     */
    struct ClipmapConfig
    {
        int levels = 12;               ///< Número de niveles de LOD (8-16)
        int segments = 32;             ///< Segmentos por tile (32 o 64)
        float segmentSize = 2.0f;      ///< Tamaño de cada segmento en metros
        float heightScale = 3000.0f;   ///< Escala de altura del heightmap
        float heightOffset = 0.0f;     ///< Offset de altura base
        float terrainSize = 200000.0f; ///< Tamaño total del terreno (metros)

        // Fog
        float fogMinDist = 1000.0f;
        float fogMaxDist = 100000.0f;
    };

    /**
     * @brief Bloque de geometría del terreno
     *
     * Representa una sección rectangular del terreno usando triangle strips
     * con primitive restart para renderizado eficiente.
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
     * @brief Seam (costura) para conectar niveles LOD
     *
     * Previene cracks entre niveles de diferente resolución
     */
    class TerrainSeam
    {
    public:
        TerrainSeam(int columns, float segmentSize);
        ~TerrainSeam();

        void draw();
        void bind();
        void unbind();

    private:
        GLuint vao_ = 0;
        GLuint vbo_ = 0;
        unsigned int vertexCount_ = 0;

        void generateGeometry(int columns, float segmentSize);
    };

    /**
     * @brief Sistema de terreno con Geometry Clipmap
     *
     * Implementa un sistema de terreno eficiente para áreas grandes usando
     * múltiples niveles de detalle (LOD) anidados.
     *
     * Características:
     * - LOD basado en distancia a cámara
     * - Heightmap para elevación
     * - Normal mapping para iluminación
     * - Fog atmosférico
     * - Renderizado eficiente con triangle strips
     *
     * Basado en "Geometry Clipmaps: Terrain Rendering Using Nested Regular Grids"
     * (Losasso & Hoppe, 2004)
     */
    class ClipmapTerrain
    {
    public:
        /**
         * @brief Constructor con configuración
         */
        explicit ClipmapTerrain(const ClipmapConfig &config = ClipmapConfig());

        /**
         * @brief Destructor
         */
        ~ClipmapTerrain();

        /**
         * @brief Inicializa el sistema (geometría y shaders)
         */
        void init();

        /**
         * @brief Carga las texturas del terreno
         * @param basePath Directorio base de las texturas
         * @return true si se cargaron correctamente
         */
        bool loadTextures(const std::string &basePath);

        /**
         * @brief Renderiza el terreno
         * @param view Matriz de vista
         * @param projection Matriz de proyección
         * @param cameraPos Posición de la cámara
         * @param backgroundColor Color de fondo para fog
         */
        void draw(const glm::mat4 &view,
                  const glm::mat4 &projection,
                  const glm::vec3 &cameraPos,
                  const glm::vec3 &backgroundColor = glm::vec3(0.5f, 0.7f, 1.0f));

        /**
         * @brief Obtiene la altura del terreno en una posición
         * @param worldPos Posición 2D (X, Z)
         * @return Altura Y
         */
        float getHeightAt(const glm::vec2 &worldPos) const;

        /**
         * @brief Limpia recursos
         */
        void cleanup();

        // Opciones públicas
        bool wireframe = false;     ///< Modo wireframe para debug
        bool showLODColors = false; ///< Colorear por nivel LOD

    private:
        ClipmapConfig config_;
        Shader shader_;

        // Texturas
        GLuint heightmapTex_ = 0;
        GLuint normalmapTex_ = 0;
        GLuint terrainTex_ = 0;

        // Geometría - diferentes tipos de bloques
        std::unique_ptr<TerrainBlock> tile_;       ///< Tile estándar
        std::unique_ptr<TerrainBlock> center_;     ///< Bloque central
        std::unique_ptr<TerrainBlock> colFixup_;   ///< Fixup vertical
        std::unique_ptr<TerrainBlock> rowFixup_;   ///< Fixup horizontal
        std::unique_ptr<TerrainBlock> horizontal_; ///< Bloque horizontal
        std::unique_ptr<TerrainBlock> vertical_;   ///< Bloque vertical
        std::unique_ptr<TerrainSeam> seam_;        ///< Seam LOD

        bool initialized_ = false;

        /**
         * @brief Inicializa la geometría de todos los bloques
         */
        void initGeometry();

        /**
         * @brief Inicializa los shaders
         */
        void initShaders();

        /**
         * @brief Renderiza un nivel del clipmap
         * @param level Índice del nivel
         * @param cameraPosXY Posición 2D de la cámara
         * @param minLevel Nivel mínimo a renderizar (se modifica por culling)
         */
        void drawLevel(int level, const glm::vec2 &cameraPosXY, int &minLevel);

        /**
         * @brief Calcula la posición base de un nivel
         * @param level Nivel del clipmap
         * @param cameraPos Posición de la cámara
         * @return Posición base del nivel
         */
        glm::vec2 calcBase(int level, const glm::vec2 &cameraPos) const;

        /**
         * @brief Crea matriz de transformación para un tile
         * @param position Posición 2D
         * @param scale Escala
         * @param angle Rotación en grados
         * @return Matriz modelo
         */
        glm::mat4 transformMatrix(const glm::vec2 &position,
                                  float scale,
                                  float angle = 0.0f) const;

        /**
         * @brief Carga una textura desde archivo
         * @param path Ruta al archivo
         * @param format Formato OpenGL
         * @return ID de textura o 0 si falla
         */
        GLuint loadTexture(const std::string &path, GLenum format = GL_RGB);
    };

} // namespace gfx
