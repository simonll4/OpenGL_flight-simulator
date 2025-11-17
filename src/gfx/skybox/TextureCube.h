#pragma once
#include <string>
#include <array>

extern "C"
{
#include <glad/glad.h>
}

#include "../../util/ImageAtlas.h"

namespace gfx
{

    /**
     * @brief Abstracción ligera sobre una textura cubemap OpenGL.
     *
     * Se encarga de cargar la textura desde un atlas (cross layout) o desde seis
     * imágenes sueltas, configurar los parámetros de filtrado y exponer helpers
     * para bindearla durante el renderizado.
     */
    class TextureCube
    {
    public:
        TextureCube() = default;
        ~TextureCube()
        {
            if (id_)
                glDeleteTextures(1, &id_);
        }

        /// No es copiable para evitar duplicar handles OpenGL.
        TextureCube(const TextureCube &) = delete;
        TextureCube &operator=(const TextureCube &) = delete;

        /// Movimiento seguro para transferir la propiedad del texture id.
        TextureCube(TextureCube &&other) noexcept : id_(other.id_) { other.id_ = 0; }
        TextureCube &operator=(TextureCube &&other) noexcept
        {
            if (this != &other)
            {
                if (id_)
                    glDeleteTextures(1, &id_);
                id_ = other.id_;
                other.id_ = 0;
            }
            return *this;
        }

        /**
         * @brief Carga un cubemap desde un atlas 2D (cross layout, fila/columna, etc.).
         * @param path Ruta al atlas LDR.
         * @param flipY Invierte verticalmente el atlas durante la carga si es true.
         */
        bool loadFromAtlas(const std::string &path, bool flipY = false);

        /**
         * @brief Carga cada cara desde archivos individuales enumerados en paths.
         * @param paths Array con las 6 rutas en el orden esperado por util::CubeFaces.
         * @param flipY Invierte cada imagen durante la carga.
         */
        bool loadFromFiles(const std::array<std::string, 6> &paths, bool flipY = false);

        /// Hace bind directo del cubemap al target GL_TEXTURE_CUBE_MAP.
        void bind() const { glBindTexture(GL_TEXTURE_CUBE_MAP, id_); }

        /// Vincula la textura a una unidad concreta antes de dibujar.
        void bindUnit(GLuint unit) const
        {
            glActiveTexture(GL_TEXTURE0 + unit);
            glBindTexture(GL_TEXTURE_CUBE_MAP, id_);
        }

        /// Devuelve el identificador raw por si se requiere desde fuera.
        GLuint id() const { return id_; }

    private:
        GLuint id_ = 0; ///< Handle del cubemap en la GPU.

        void setupParameters();
        bool loadCubeFaces(const util::CubeFaces &faces);
    };

} // namespace gfx
